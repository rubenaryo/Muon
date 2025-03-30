/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2025/3
Description : Interface for GPU Upload Buffers
----------------------------------------------*/
#ifndef MUON_UPLOADBUFFER_H
#define MUON_UPLOADBUFFER_H

#include <wrl/client.h>
#include <d3d12.h>
#include <stdint.h>

namespace Muon
{

struct UploadBuffer
{
    UploadBuffer();
    ~UploadBuffer();

    void Create(const wchar_t* name, size_t size);
    void TryDestroy();

    void* Map();
    void Unmap(size_t begin, size_t end);

    size_t mBufferSize = 0;
    Microsoft::WRL::ComPtr<ID3D12Resource> mpResource;
};

size_t GetConstantBufferSize(size_t desiredSize);

}
#endif