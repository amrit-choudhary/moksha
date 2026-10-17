#pragma once
#ifdef ME_WIN

/**
 * DirectX's specific quad class.
 */

#include <d3d12.h>

#include "moksha/client/rendering/shared/quad.h"

namespace ME {

class QuadDX : public Quad {
   public:
    QuadDX(const char* path, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
    ~QuadDX() override;

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
