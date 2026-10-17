#pragma once
#ifdef ME_DX12

/**
 * DirectX's specific texture class.
 */

#include "d3dx12.h"
#include "moksha/client/rendering/shared/texture.h"

namespace ME {

/**
 * DirectX's specific texture class.
 * This class is used to load and manage textures using DirectX.
 */
class TextureDX : public Texture {
   public:
    TextureDX();
    ~TextureDX() override;
    /** 1. Loads a PNG texture from file.
     * 2. Create a DirectX texture buffer.
     * 3. Upload the data to the GPU.
     */
    TextureDX(const char* path, ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);

    ID3D12Resource* GetTextureBuffer() const;

    /** Create the texture buffer on GPU.*/
    void CreateBuffers(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
    /** Release the upload buffers after data has been copied to GPU. */
    void ReleaseUploadBuffers();

    /** Index of the descriptor in the descriptor heap. This is used to return the GPU & CPU handle. */
    uint32_t descHeapIndex;

   private:
    ID3D12Resource* textureBuffer = nullptr;
    ID3D12Resource* uploadBuffer = nullptr;
};

}  // namespace ME

#endif  // ME_DX12
