#ifdef ME_METAL

#include "scene_metal.h"

#include <Metal/Metal.hpp>

#include "moksha/client/rendering/shared/shader_constants.h"
#include "moksha/shared/misc/engine_limits.h"

ME::SceneMetal::SceneMetal(MTL::Device* inDevice, MTL::ResidencySet* inResidencySet, ME::Scene* gameScene) {
    device = inDevice;
    residencySet = inResidencySet;
    scene = gameScene;

    ambientLight = scene->ambientLight;
    directionalLight = scene->directionalLight;
    spriteCamera = scene->spriteCamera;

    spriteTextures = new ME::TextureMetal*[Constants::MaxSpriteTextureCount];
    perPassCBs = new ME::UploadBufferMetal*[Constants::MaxConstantBuffersCount];
    perSpriteCBs = new ME::UploadBufferMetal*[Constants::MaxConstantBuffersCount];
    textureAtlasCBs = new ME::UploadBufferMetal*[Constants::MaxConstantBuffersCount];

    textureAtlasProperties = scene->textureAtlasProperties;
    spriteTransforms = scene->spriteTransforms;
    spriteRenderers = scene->spriteRenderers;

    instancedSpriteRenderers0 = scene->instancedSpriteRenderers0;
    spriteInstanceData0 = scene->spriteInstanceData0;
    instancedSpriteRenderers1 = scene->instancedSpriteRenderers1;
    spriteInstanceData1 = scene->spriteInstanceData1;

    MakeConstantBuffers();
    MakeSpriteTextures();
    MakeSpriteInstanceBuffer();

    // Only read while constructing: the game deletes its scene before the renderer ends.
    scene = nullptr;
}

ME::SceneMetal::~SceneMetal() {
    for (uint32_t i = 0; i < spriteTextureCount; ++i) {
        delete spriteTextures[i];
    }
    delete[] spriteTextures;

    for (uint32_t i = 0; i < perPassCBCount; ++i) {
        delete perPassCBs[i];
    }
    delete[] perPassCBs;

    for (uint32_t i = 0; i < perSpriteCBCount; ++i) {
        delete perSpriteCBs[i];
    }
    delete[] perSpriteCBs;

    for (uint32_t i = 0; i < textureAtlasCBCount; ++i) {
        delete textureAtlasCBs[i];
    }
    delete[] textureAtlasCBs;

    delete spriteInstanceBuffer0;
    delete spriteInstanceBuffer1;
}

void ME::SceneMetal::Update() {}

void ME::SceneMetal::MakeSpriteTextures() {
    spriteTextureCount = static_cast<uint32_t>(scene->spriteTexturePaths.count);
    for (uint32_t i = 0; i < spriteTextureCount; ++i) {
        spriteTextures[i] = new ME::TextureMetal(scene->spriteTexturePaths[i], device);
        spriteTextures[i]->CreateBuffers(device);
        if (spriteTextures[i]->GetTexture() != nullptr) {
            residencySet->addAllocation(spriteTextures[i]->GetTexture());
        }
    }
}

void ME::SceneMetal::MakeConstantBuffers() {
    // Per-pass constants for sprite rendering.
    perPassCBCount = 1;
    perPassCBs[0] = AddResident(new ME::UploadBufferMetal(device, true, 1, sizeof(ME::CBPerPass)));

    // One per sprite renderer, as in SceneDX.
    perSpriteCBCount = static_cast<uint32_t>(spriteRenderers.count);
    for (uint32_t i = 0; i < perSpriteCBCount; ++i) {
        perSpriteCBs[i] = AddResident(new ME::UploadBufferMetal(device, true, 1, sizeof(ME::CBPerSprite)));
    }

    textureAtlasCBCount = static_cast<uint32_t>(scene->textureAtlasProperties.count);
    for (uint32_t i = 0; i < textureAtlasCBCount; ++i) {
        textureAtlasCBs[i] =
            AddResident(new ME::UploadBufferMetal(device, true, 1, sizeof(ME::TextureAtlasProperties)));
    }
}

void ME::SceneMetal::MakeSpriteInstanceBuffer() {
    // Each set independently (SceneDX skips set 1 when set 0 is empty).
    if (instancedSpriteRenderers0.count != 0) {
        spriteInstanceBuffer0 =
            AddResident(new ME::UploadBufferMetal(device, false, static_cast<uint32_t>(instancedSpriteRenderers0.count),
                                                  sizeof(ME::SpriteRendererInstanceData)));
    }
    if (instancedSpriteRenderers1.count != 0) {
        spriteInstanceBuffer1 =
            AddResident(new ME::UploadBufferMetal(device, false, static_cast<uint32_t>(instancedSpriteRenderers1.count),
                                                  sizeof(ME::SpriteRendererInstanceData)));
    }
}

ME::UploadBufferMetal* ME::SceneMetal::AddResident(ME::UploadBufferMetal* buffer) {
    if (buffer->GetBuffer() != nullptr) {
        residencySet->addAllocation(buffer->GetBuffer());
    }
    return buffer;
}

#endif  // ME_METAL
