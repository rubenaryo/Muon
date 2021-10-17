/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2021/03
Description : Main D3D12 Interface
----------------------------------------------*/
#ifndef MUON_H
#define MUON_H

#include "Renderer_pch.h"

namespace Renderer
{

class MNPipeline
{
    friend class Game;

    static const UINT BUFFER_COUNT = 2;

    IDXGISwapChain3*            mpSwapChain;
    ID3D12Device*               mpDevice;
    ID3D12Resource*             mpRenderTargets[BUFFER_COUNT];
    ID3D12CommandAllocator*     mpCommandAllocator;
    ID3D12CommandQueue*         mpCommandQueue;
    ID3D12RootSignature*        mpRootSig;
    ID3D12DescriptorHeap*       mpDescriptorHeap;
    ID3D12PipelineState*        mpPipelineState;
    ID3D12GraphicsCommandList*  mpCommandList;

    D3D12_VIEWPORT  mViewport;
    D3D12_RECT      mRect;

public:

    MNPipeline(UINT width, UINT height, const wchar_t* name);

    bool Init();
    void Update();
    void Render();
    void Shutdown();

private:
};

}
#endif