#pragma once
#ifdef ME_METAL

/**
 * Metal's texture class, the counterpart of TextureDX: loads a cooked .dds and creates an sRGB RGBA8 Metal texture.
 * The owner adds GetTexture() to a residency set and binds its gpuResourceID in an argument table.
 */

#include "moksha/client/rendering/shared/texture.h"

namespace MTL {
class Device;
class Texture;
}  // namespace MTL

namespace ME {

class TextureMetal : public Texture {
   public:
    // Loads the texture file at path (Resources root).
    TextureMetal(const char* path, MTL::Device* device);
    ~TextureMetal() override;

    TextureMetal(const TextureMetal&) = delete;
    TextureMetal& operator=(const TextureMetal&) = delete;

    // Creates the Metal texture and copies the pixels into it.
    void CreateBuffers(MTL::Device* device);

    MTL::Texture* GetTexture() const;

   private:
    MTL::Texture* texture = nullptr;
};

}  // namespace ME

#endif  // ME_METAL
