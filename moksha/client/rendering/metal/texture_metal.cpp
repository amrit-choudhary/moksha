#ifdef ME_MAC

#include "texture_metal.h"

ME::TextureMetal::TextureMetal() : Texture() {}

ME::TextureMetal::TextureMetal(const char* path, MTL::Device* device, bool hasMipmaps, MTL::CommandQueue* cmdQueue) {
    Texture::Load(path);

    MTL::TextureDescriptor* textureDesc = MTL::TextureDescriptor::alloc()->init();
    textureDesc->setWidth(width);
    textureDesc->setHeight(height);
    textureDesc->setPixelFormat(MTL::PixelFormat::PixelFormatRGBA8Unorm_sRGB);
    textureDesc->setTextureType(MTL::TextureType2D);
    textureDesc->setStorageMode(MTL::StorageModeShared);
    textureDesc->setUsage(MTL::ResourceUsageSample | MTL::ResourceUsageRead | MTL::ResourceUsageWrite);

    if (hasMipmaps) {
        textureDesc->setMipmapLevelCount(ME::MIP_LEVELS_DEFAULT);
    } else {
        textureDesc->setMipmapLevelCount(1);
    }

    textureMetal = device->newTexture(textureDesc);
    textureDesc->release();
    textureMetal->replaceRegion(MTL::Region::Make2D(0, 0, width, height), 0, data, width * channels);

    if (hasMipmaps) {
        MTL::CommandBuffer* cmdBuffer = cmdQueue->commandBuffer();
        MTL::BlitCommandEncoder* blitEncoder = cmdBuffer->blitCommandEncoder();
        blitEncoder->generateMipmaps(textureMetal);
        blitEncoder->endEncoding();
        cmdBuffer->commit();
        cmdBuffer->waitUntilCompleted();
    }
}

ME::TextureMetal::~TextureMetal() {
    textureMetal->release();
}

MTL::SamplerState* ME::TextureMetal::GetSamplerStateNearest(MTL::Device* device) {
    MTL::SamplerDescriptor* samplerDesc = MTL::SamplerDescriptor::alloc()->init();
    samplerDesc->setMinFilter(MTL::SamplerMinMagFilterNearest);
    samplerDesc->setMagFilter(MTL::SamplerMinMagFilterNearest);
    samplerDesc->setMipFilter(MTL::SamplerMipFilterNearest);
    samplerDesc->setSAddressMode(MTL::SamplerAddressModeRepeat);
    samplerDesc->setTAddressMode(MTL::SamplerAddressModeRepeat);
    samplerDesc->setNormalizedCoordinates(true);

    MTL::SamplerState* newState = device->newSamplerState(samplerDesc);
    samplerDesc->release();

    return newState;
}

MTL::SamplerState* ME::TextureMetal::GetSamplerStateLinear(MTL::Device* device) {
    MTL::SamplerDescriptor* samplerDesc = MTL::SamplerDescriptor::alloc()->init();
    samplerDesc->setMinFilter(MTL::SamplerMinMagFilterLinear);
    samplerDesc->setMagFilter(MTL::SamplerMinMagFilterLinear);
    samplerDesc->setMipFilter(MTL::SamplerMipFilterLinear);
    samplerDesc->setSAddressMode(MTL::SamplerAddressModeRepeat);
    samplerDesc->setTAddressMode(MTL::SamplerAddressModeRepeat);
    samplerDesc->setNormalizedCoordinates(true);

    MTL::SamplerState* newState = device->newSamplerState(samplerDesc);
    samplerDesc->release();

    return newState;
}

#endif
