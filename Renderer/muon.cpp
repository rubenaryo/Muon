/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2021/03
Description : D3D12 Pipeline Initialization and Functions
----------------------------------------------*/

#include "Renderer_pch.h"

#include "muon.h"
#include "muon_utils.h"

namespace
{
    void GetHardwareAdapter(
        IDXGIFactory1* pFactory,
        IDXGIAdapter1** ppAdapter,
        bool requestHighPerformanceAdapter)
    {
        *ppAdapter = nullptr;

        IDXGIAdapter1* pAdapter;

        IDXGIFactory6* pFactory6;
        if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&pFactory6))))
        {
            for (
                UINT adapterIndex = 0;
                DXGI_ERROR_NOT_FOUND != pFactory6->EnumAdapterByGpuPreference(
                    adapterIndex,
                    requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                    IID_PPV_ARGS(&pAdapter));
                ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                pAdapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver adapter.
                    // If you want a software adapter, pass in "/warp" on the command line.
                    continue;
                }

                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                {
                    break;
                }
            }
            pFactory6->Release();
        }
        else
        {
            for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &pAdapter); ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                pAdapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver adapter.
                    // If you want a software adapter, pass in "/warp" on the command line.
                    continue;
                }

                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                {
                    break;
                }
            }
        }

        *ppAdapter = pAdapter;
        pAdapter->Release();
    }
}

namespace Renderer
{

MNPipeline::MNPipeline(UINT width, UINT height, const wchar_t* name) :
    mpSwapChain(NULL),
    mpDevice(NULL),
    mpCommandAllocator(NULL),
    mpCommandQueue(NULL),
    mpRootSig(NULL),
    mpDescriptorHeap(NULL),
    mpPipelineState(NULL),
    mpCommandList(NULL)
{
    mViewport.TopLeftX  = 0.0f;
    mViewport.TopLeftY  = 0.0f;
    mViewport.Width     = static_cast<float>(width);
    mViewport.Height    = static_cast<float>(height);
    mViewport.MinDepth  = D3D12_MIN_DEPTH;
    mViewport.MaxDepth  = D3D12_MAX_DEPTH;

    mRect.left      = 0;
    mRect.top       = 0;
    mRect.right     = static_cast<LONG>(width);
    mRect.bottom    = static_cast<LONG>(height);

    for (uint8_t i = 0; i != BUFFER_COUNT; ++i)
        mpRenderTargets[i] = NULL;
}

bool MNPipeline::Init()
{
    UINT dxgiFactoryFlags = 0;

#if defined(MN_DEBUG)
    // Enable the D3D12 debug layer.
    {
        ID3D12Debug* pDebugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController))))
        {
            pDebugController->EnableDebugLayer();
            pDebugController->Release();


            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif

    IDXGIFactory4* pFactory;
    HR(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&pFactory)));

    // Standard hardware adapter
    IDXGIAdapter1* pHardwareAdapter = NULL;
    HR(D3D12CreateDevice(pHardwareAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mpDevice)));

    // Copy Paste Validation of SM6.5 and Mesh Shaders
    D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_5 };
    if (FAILED(mpDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)))
        || (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_5))
    {
        OutputDebugStringA("ERROR: Shader Model 6.5 is not supported\n");
        throw std::exception("Shader Model 6.5 is not supported");
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS7 features = {};
    if (FAILED(mpDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &features, sizeof(features)))
        || (features.MeshShaderTier == D3D12_MESH_SHADER_TIER_NOT_SUPPORTED))
    {
        OutputDebugStringA("ERROR: Mesh Shaders aren't supported!\n");
        throw std::exception("Mesh Shaders aren't supported!");
    }

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    HR(mpDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mpCommandQueue)));

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = BUFFER_COUNT;
    swapChainDesc.Width = mRect.right;
    swapChainDesc.Height = mRect.bottom;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    //IDXGISwapChain1* pSwapChain = NULL;

    return true;
}

}