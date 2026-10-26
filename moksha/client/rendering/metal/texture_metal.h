#ifdef ME_METAL

/**
 * Metal's specific texture class.
 */

#pragma once

#include <simd/simd.h>

#include <Metal/Metal.hpp>

#include "moksha/client/rendering/shared/texture.h"

namespace ME {

/**
 * Texture class for handling textures in Metal.
 * This class is used to load and manage textures using Metal.
 */
class TextureMetal : public Texture {
   public:
    TextureMetal();
    /**
     * 1. Loads a PNG texture from file.
     * 2. Create a Metal texture buffer.
     * 3. Upload the data to the GPU.
     * 4. Optionally creates Mipmaps.
     */
    TextureMetal(const char* path, MTL::Device* device, bool hasMipmaps = false, MTL::CommandQueue* cmdQueue = nullptr);
    ~TextureMetal() override;

    MTL::Texture* GetTextureMetal() const {
        return textureMetal;
    }

    /**
     * Creates a default Sampler State and returns for the render pass.
     * Nearest sampler is good for pixel art.
     */
    static MTL::SamplerState* GetSamplerStateNearest(MTL::Device* device);

    /**
     * Creates a default Sampler State and returns for the render pass.
     * Linear sampler is good for smooth textures.
     */
    static MTL::SamplerState* GetSamplerStateLinear(MTL::Device* device);

   protected:
    MTL::Texture* textureMetal;
};

}  // namespace ME

#endif
