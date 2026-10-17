#pragma once
#ifdef ME_WIN

/**
 * DirectX's specific mesh class.
 * Has D3D12 buffers for vertex, index and instance data.
 * This class is used to load and manage meshes using DirectX.
 */

#include <d3d12.h>

#include "moksha/client/rendering/shared/mesh.h"

namespace ME {

class MeshDX : public Mesh {
   public:
    MeshDX(const char* path, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
    ~MeshDX() override;

    /** Create the vertex and index buffer on GPU.*/
    void CreateBuffers(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
    /** Release the upload buffers after data has been copied to GPU. */
    void ReleaseUploadBuffers();

    D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const;
    D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const;

    ID3D12Resource* vertexBuffer = nullptr;
    ID3D12Resource* indexBuffer = nullptr;

    ID3D12Resource* vertexBufferUpload = nullptr;
    ID3D12Resource* indexBufferUpload = nullptr;
};

}  // namespace ME
#endif  // ME_WIN
