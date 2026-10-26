#ifdef ME_METAL

#include "texture_metal.h"

#include <Metal/Metal.hpp>

ME::TextureMetal::TextureMetal(const char* path, MTL::Device* device) {
    Texture::Load(path);
}

ME::TextureMetal::~TextureMetal() {
    if (texture != nullptr) {
        texture->release();
        texture = nullptr;
    }
}

void ME::TextureMetal::CreateBuffers(MTL::Device* device) {
    if (data == nullptr || width == 0 || height == 0) {
        return;
    }

    // sRGB like TextureDX. Shared storage: Apple silicon memory is unified, so the pixels are written directly.
    MTL::TextureDescriptor* descriptor =
        MTL::TextureDescriptor::texture2DDescriptor(MTL::PixelFormatRGBA8Unorm_sRGB, width, height, false);
    descriptor->setUsage(MTL::TextureUsageShaderRead);
    descriptor->setStorageMode(MTL::StorageModeShared);
    texture = device->newTexture(descriptor);
    if (texture == nullptr) {
        return;
    }

    const NS::UInteger bytesPerRow = static_cast<NS::UInteger>(width) * channels;
    texture->replaceRegion(MTL::Region::Make2D(0, 0, width, height), 0, data, bytesPerRow);
}

MTL::Texture* ME::TextureMetal::GetTexture() const {
    return texture;
}

#endif  // ME_METAL
