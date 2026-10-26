#ifdef ME_METAL

#include "scene_ui_metal.h"

#include <Metal/Metal.hpp>

#include "moksha/client/rendering/shared/shader_constants.h"
#include "moksha/shared/misc/engine_limits.h"

ME::SceneUIMetal::SceneUIMetal(MTL::Device* inDevice, MTL::ResidencySet* inResidencySet, ME::SceneUI* uiScene) {
    device = inDevice;
    residencySet = inResidencySet;
    scene = uiScene;

    spriteTextures = new ME::TextureMetal*[Constants::MaxSpriteTextureCount];
    perPassCBs = new ME::UploadBufferMetal*[Constants::MaxConstantBuffersCount];
    textureAtlasCBs = new ME::UploadBufferMetal*[Constants::MaxConstantBuffersCount];

    textureAtlasProperties = scene->textureAtlasProperties;
    uiSpriteInstanceData = scene->uiSpriteInstanceData;
    textInstanceData = scene->textInstanceData;

    MakeSpriteTextures();
    MakeConstantBuffers();
    MakeUISpriteInstanceBuffer();
    MakeTextInstanceBuffer();
}

ME::SceneUIMetal::~SceneUIMetal() {
    for (uint32_t i = 0; i < spriteTextureCount; ++i) {
        delete spriteTextures[i];
    }
    delete[] spriteTextures;

    for (uint32_t i = 0; i < perPassCBCount; ++i) {
        delete perPassCBs[i];
    }
    delete[] perPassCBs;

    for (uint32_t i = 0; i < textureAtlasCBCount; ++i) {
        delete textureAtlasCBs[i];
    }
    delete[] textureAtlasCBs;

    delete uiSpriteInstanceBuffer;
    delete textInstanceBuffer;
}

void ME::SceneUIMetal::Update() {
    // UI built after construction adds elements; the arrays are allocated once at capacity, so only counts change.
    uiSpriteInstanceData.count = scene->uiSpriteInstanceData.count;
    textInstanceData.count = scene->textInstanceData.count;
}

void ME::SceneUIMetal::MakeSpriteTextures() {
    spriteTextureCount = static_cast<uint32_t>(scene->spriteTexturePaths.count);
    for (uint32_t i = 0; i < spriteTextureCount; ++i) {
        spriteTextures[i] = new ME::TextureMetal(scene->spriteTexturePaths[i], device);
        spriteTextures[i]->CreateBuffers(device);
        if (spriteTextures[i]->GetTexture() != nullptr) {
            residencySet->addAllocation(spriteTextures[i]->GetTexture());
        }
    }
}

void ME::SceneUIMetal::MakeConstantBuffers() {
    perPassCBCount = 2;

    // UI Sprite Rendering.
    perPassCBs[0] = AddResident(new ME::UploadBufferMetal(device, true, 1, sizeof(ME::CBPerPassUISprite)));

    // UI Text Rendering.
    perPassCBs[1] = AddResident(new ME::UploadBufferMetal(device, true, 1, sizeof(ME::CBPerPassUIText)));

    textureAtlasCBCount = static_cast<uint32_t>(scene->textureAtlasProperties.count);
    for (uint32_t i = 0; i < textureAtlasCBCount; ++i) {
        textureAtlasCBs[i] =
            AddResident(new ME::UploadBufferMetal(device, true, 1, sizeof(ME::TextureAtlasProperties)));
    }
}

void ME::SceneUIMetal::MakeUISpriteInstanceBuffer() {
    // At capacity, so dynamic UI elements added after construction fit.
    uiSpriteInstanceBuffer = AddResident(new ME::UploadBufferMetal(
        device, false, Constants::MaxUISpriteInstanceDataCount, sizeof(ME::UISpriteRendererInstanceData)));
}

void ME::SceneUIMetal::MakeTextInstanceBuffer() {
    textInstanceBuffer = AddResident(new ME::UploadBufferMetal(device, false, Constants::MaxTextInstanceDataCount,
                                                               sizeof(ME::TextRendererInstanceData)));
}

ME::UploadBufferMetal* ME::SceneUIMetal::AddResident(ME::UploadBufferMetal* buffer) {
    if (buffer->GetBuffer() != nullptr) {
        residencySet->addAllocation(buffer->GetBuffer());
    }
    return buffer;
}

#endif  // ME_METAL
