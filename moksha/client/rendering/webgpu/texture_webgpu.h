#pragma once
#ifdef ME_WEBGPU

#include <webgpu/webgpu.h>

#include "moksha/client/rendering/shared/texture.h"

namespace ME {

class TextureWebGPU : public Texture {
   public:
    // Loads the texture file at path (Resources root) and copies its pixels to the GPU.
    TextureWebGPU(const char* path, WGPUDevice device, WGPUQueue queue);
    ~TextureWebGPU() override;

    TextureWebGPU(const TextureWebGPU&) = delete;
    TextureWebGPU& operator=(const TextureWebGPU&) = delete;

    // Null if the file couldn't be loaded.
    WGPUTextureView GetView() const;

   private:
    WGPUTexture texture = nullptr;
    WGPUTextureView view = nullptr;
};

}  // namespace ME

#endif  // ME_WEBGPU
