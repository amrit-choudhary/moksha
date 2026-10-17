#ifdef ME_DX12

#include "mesh_dx.h"

#include "utils_dx.h"

ME::MeshDX::MeshDX(const char* path, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList) {
    LoadFromFile(path);
}

ME::MeshDX::~MeshDX() {
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

void ME::MeshDX::CreateBuffers(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList) {
    vertexBuffer =
        ME::UtilsDirectX::CreateDefaultBufferResource(device, cmdList, vertices, verticesSize, &vertexBufferUpload);
    indexBuffer =
        ME::UtilsDirectX::CreateDefaultBufferResource(device, cmdList, indices, indicesSize, &indexBufferUpload);
}

void ME::MeshDX::ReleaseUploadBuffers() {
    if (vertexBufferUpload) {
        vertexBufferUpload->Release();
        vertexBufferUpload = nullptr;
    }
    if (indexBufferUpload) {
        indexBufferUpload->Release();
        indexBufferUpload = nullptr;
    }
}

D3D12_VERTEX_BUFFER_VIEW ME::MeshDX::GetVertexBufferView() const {
    D3D12_VERTEX_BUFFER_VIEW vbv;
    vbv.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vbv.StrideInBytes = sizeof(ME::Vertex);
    vbv.SizeInBytes = verticesSize;
    return vbv;
}

D3D12_INDEX_BUFFER_VIEW ME::MeshDX::GetIndexBufferView() const {
    D3D12_INDEX_BUFFER_VIEW ibv;
    ibv.BufferLocation = indexBuffer->GetGPUVirtualAddress();
    ibv.SizeInBytes = indicesSize;
    ibv.Format = DXGI_FORMAT_R32_UINT;
    return ibv;
}

#endif  // ME_DX12
