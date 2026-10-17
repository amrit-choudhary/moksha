#ifdef ME_WIN

#include "quad_dx.h"

#include "utils_dx.h"

ME::QuadDX::QuadDX(const char* path, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList) : Quad() {}

ME::QuadDX::~QuadDX() {
    if (vertexBuffer) {
        vertexBuffer->Release();
        vertexBuffer = nullptr;
    }
    if (indexBuffer) {
        indexBuffer->Release();
        indexBuffer = nullptr;
    }
    if (vertexBufferUpload) {
        vertexBufferUpload->Release();
        vertexBufferUpload = nullptr;
    }
    if (indexBufferUpload) {
        indexBufferUpload->Release();
        indexBufferUpload = nullptr;
    }
}

void ME::QuadDX::CreateBuffers(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList) {
    vertexBuffer =
        ME::UtilsDirectX::CreateDefaultBufferResource(device, cmdList, vertices, verticesSize, &vertexBufferUpload);
    indexBuffer =
        ME::UtilsDirectX::CreateDefaultBufferResource(device, cmdList, indices, indicesSize, &indexBufferUpload);
}

void ME::QuadDX::ReleaseUploadBuffers() {
    if (vertexBufferUpload) {
        vertexBufferUpload->Release();
        vertexBufferUpload = nullptr;
    }
    if (indexBufferUpload) {
        indexBufferUpload->Release();
        indexBufferUpload = nullptr;
    }
}

D3D12_VERTEX_BUFFER_VIEW ME::QuadDX::GetVertexBufferView() const {
    D3D12_VERTEX_BUFFER_VIEW vbv;
    vbv.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vbv.StrideInBytes = sizeof(ME::Vertex2D);
    vbv.SizeInBytes = verticesSize;
    return vbv;
}

D3D12_INDEX_BUFFER_VIEW ME::QuadDX::GetIndexBufferView() const {
    D3D12_INDEX_BUFFER_VIEW ibv;
    ibv.BufferLocation = indexBuffer->GetGPUVirtualAddress();
    ibv.SizeInBytes = indicesSize;
    ibv.Format = DXGI_FORMAT_R16_UINT;
    return ibv;
}

#endif
