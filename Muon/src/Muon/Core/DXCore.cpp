/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2025/3
Description : Protocol and device resources for DX12
----------------------------------------------*/
#include <Muon.h>
#include <Muon/Utils/Utils.h>
#include <Muon/Core/DXCore.h>
#include <Muon/Renderer/ThrowMacros.h> // TODO: move to Core?

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <stdint.h>
#include <wrl/client.h>

#define CHECK_SUCCESS(s, msg)       \
do {                                \
    if (!s)                         \
    {                               \
        Muon::Print(msg);    \
    }                               \
} while (0)                         \

namespace Muon
{
    ID3D12Device* gDevice = nullptr;

    ID3D12Fence* gFence = nullptr;

    UINT gRTVSize = 0;
    UINT gDSVSize = 0;
    UINT gCBVSize = 0;
    UINT gMSAAQuality = 0;

    ID3D12CommandQueue* gCommandQueue = nullptr;
    ID3D12CommandAllocator* gCommandAllocator = nullptr;
    ID3D12GraphicsCommandList* gCommandList = nullptr;

    DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    const int SwapChainBufferCount = 2;

    ID3D12Resource* gSwapChainBuffers[SwapChainBufferCount] = {0};
    ID3D12Resource* gDepthStencilBuffer = nullptr;

    ID3D12DescriptorHeap* gRTVHeap = nullptr;
    ID3D12DescriptorHeap* gDSVHeap = nullptr;

    /////////////////////////////////////////////////////////////////////
    // Accessors
    ID3D12Device* GetDevice() { return gDevice; }
    ID3D12Fence* GetFence() { return gFence; }
    UINT GetRTVDescriptorSize() { return gRTVSize; }
    UINT GetDSVDescriptorSize() { return gDSVSize; }
    UINT GetCBVDescriptorSize() { return gCBVSize; }
    UINT GetMSAAQualityLevel() { return gMSAAQuality; }
    ID3D12CommandQueue* GetCommandQueue() { return gCommandQueue; }
    ID3D12CommandAllocator* GetCommandAllocator() { return gCommandAllocator; }
    ID3D12GraphicsCommandList* GetCommandList() { return gCommandList; }
    DXGI_FORMAT GetBackBufferFormat() { return BackBufferFormat; }
    DXGI_FORMAT GetDepthStencilFormat() { return DepthStencilFormat; }


    /////////////////////////////////////////////////////////////////////
    /// Init Steps

    bool IsDirectXRaytracingSupported(ID3D12Device* testDevice)
    {
        D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureSupport = {};

        if (FAILED(testDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &featureSupport, sizeof(featureSupport))))
            return false;

        return featureSupport.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
    }

    bool EnableDX12DebugFeatures(DWORD& out_dxgiFactoryFlags)
    {
        using Microsoft::WRL::ComPtr;

        ComPtr<ID3D12Debug> debugInterface;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface))))
        {
            debugInterface->EnableDebugLayer();

            uint32_t useGPUBasedValidation = 1; // always validate for now
            if (useGPUBasedValidation)
            {
                Microsoft::WRL::ComPtr<ID3D12Debug1> debugInterface1;
                if (SUCCEEDED((debugInterface->QueryInterface(IID_PPV_ARGS(&debugInterface1)))))
                {
                    debugInterface1->SetEnableGPUBasedValidation(true);
                }
            }
        }
        else
        {
            Muon::Print("WARNING:  Unable to enable D3D12 debug validation layer\n");
            return false;
        }

#if MN_DEBUG
        ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
        {
            out_dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

            DXGI_INFO_QUEUE_MESSAGE_ID hide[] =
            {
                80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
            };
            DXGI_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = _countof(hide);
            filter.DenyList.pIDList = hide;
            dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
        }
#endif
        return true;
    }

    bool CreateAdapterAndDevice(IDXGIFactory6* pFactory, Microsoft::WRL::ComPtr<IDXGIAdapter1>& out_adapter, Microsoft::WRL::ComPtr<ID3D12Device> out_device)
    {
        SIZE_T MaxSize = 0;

        for (uint32_t Idx = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(Idx, &out_adapter); ++Idx)
        {
            DXGI_ADAPTER_DESC1 desc;
            out_adapter->GetDesc1(&desc);

            // Is a software adapter?
            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                continue;

            // Can create a D3D12 device?
            if (FAILED(D3D12CreateDevice(out_adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&out_device))))
            {
                Muon::Print("Error: Failed to create device!");
                continue;
            }

            if (!IsDirectXRaytracingSupported(out_device.Get()))
            {
                Muon::Print("Warning: Found device does NOT support DXR raytracing.");
            }

            // By default, search for the adapter with the most memory because that's usually the dGPU.
            if (desc.DedicatedVideoMemory < MaxSize)
                continue;

            MaxSize = desc.DedicatedVideoMemory;
            //Muon::Printf("Selected GPU:  %s (%u MB)\n", desc.Description, desc.DedicatedVideoMemory >> 20);
        }

        return out_device.Get() != nullptr && out_adapter.Get() != nullptr;
    }

    bool CreateFence(ID3D12Device* pDevice, ID3D12Fence** out_fence)
    {
        if (!pDevice)
        {
            Muon::Print("Failed to create fence because of null device!");
            return false;
        }

        HRESULT hr = pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(out_fence));
        COM_EXCEPT(hr);

        return SUCCEEDED(hr);
    }

    bool GetDescriptorSizes(ID3D12Device* pDevice, UINT* out_rtv, UINT* out_dsv, UINT* out_cbv)
    {
        if (!pDevice)
        {
            Muon::Print("Failed to get descriptor sizes because of null device!");
            return false;
        }

        *out_rtv = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        *out_dsv = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        *out_cbv = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);        
        return true;
    }

    bool GetMSAAQualityLevel(ID3D12Device* pDevice, UINT* out_quality)
    {
        D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
        msQualityLevels.Format = GetBackBufferFormat();
        msQualityLevels.SampleCount = 4;
        msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
        msQualityLevels.NumQualityLevels = 0;
        HRESULT hr = pDevice->CheckFeatureSupport(
            D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
            &msQualityLevels,
            sizeof(msQualityLevels));

        *out_quality = msQualityLevels.NumQualityLevels;
        return SUCCEEDED(hr);
    }

    bool CreateCommandObjects(ID3D12Device* pDevice, ID3D12CommandQueue** out_queue, ID3D12CommandAllocator** out_alloc, ID3D12GraphicsCommandList** out_list)
    {
        HRESULT hr;

        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        hr = pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(out_queue));
        COM_EXCEPT(hr);

        hr = pDevice->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(out_alloc));
        COM_EXCEPT(hr);

        hr = pDevice->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            *out_alloc, // Associated command allocator
            nullptr,                   // Initial PipelineStateObject
            IID_PPV_ARGS(out_list));
        COM_EXCEPT(hr);

        // Start off in a closed state.  This is because the first time we refer 
        // to the command list we will Reset it, and it needs to be closed before
        // calling Reset.
        (*out_list)->Close();

        return SUCCEEDED(hr);
    }

    bool CreateSwapChain(ID3D12Device* pDevice, IDXGIFactory6* pFactory, ID3D12CommandQueue* pQueue, HWND hwnd, int width, int height, IDXGISwapChain** out_swapchain)
    {
        // Release the previous swapchain we will be recreating.
        //mSwapChain.Reset();

        DXGI_SWAP_CHAIN_DESC sd;
        sd.BufferDesc.Width = width;
        sd.BufferDesc.Height = height;
        sd.BufferDesc.RefreshRate.Numerator = 60; // TODO: higher refresh rates
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferDesc.Format = GetBackBufferFormat();
        sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        sd.SampleDesc.Count = 4; // TODO: Currently assume 4xMSAA is supported..
        sd.SampleDesc.Quality = GetMSAAQualityLevel();
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = SwapChainBufferCount;
        sd.OutputWindow = hwnd;
        sd.Windowed = true;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        // Note: Swap chain uses queue to perform flush.
        HRESULT hr = pFactory->CreateSwapChain(
            pQueue,
            &sd,
            out_swapchain);

        return SUCCEEDED(hr);
    }

    bool CreateDescriptorHeaps(ID3D12Device* pDevice);
    /////////////////////////////////////////////////////////////////////

    // Most of this initialization code is adapted from https://github.com/microsoft/DirectX-Graphics-Samples
    // or "Introduction to 3D Game Programming with DirectX 12" by Frank Luna 
    bool Initialize(HWND hwnd, int width, int height)
    {
        using Microsoft::WRL::ComPtr;
    
        ComPtr<ID3D12Device> pDevice;
        DWORD dxgiFactoryFlags = 0;
        HRESULT hr;
        bool success = true;
    
        uint32_t debugMode = 1; // Always debug for now
        if (debugMode)
        {
            success = EnableDX12DebugFeatures(dxgiFactoryFlags);
            CHECK_SUCCESS(success, "Warning: Failed to enable DX12 Debug Features!");
        }
    
        // Obtain the DXGI factory
        ComPtr<IDXGIFactory6> dxgiFactory;
        hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory));
        COM_EXCEPT(hr);
    
        D3D12EnableExperimentalFeatures(0, nullptr, nullptr, nullptr);
        ComPtr<IDXGIAdapter1> pAdapter;
    
        success = CreateAdapterAndDevice(dxgiFactory.Get(), pAdapter, pDevice);
        CHECK_SUCCESS(success, "Error: Failed to create DX12 Adapter and Device!");
    
        if (!pDevice)
        {
            // TODO: Fall back to WARP.
            return false;
        }
    
        if (gDevice != nullptr)
            gDevice->Release();
    
        gDevice = pDevice.Detach();
    
        
    
        return true;
    }

#undef CHECK_SUCCESS
}