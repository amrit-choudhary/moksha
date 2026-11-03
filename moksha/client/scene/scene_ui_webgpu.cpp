#ifdef ME_WEBGPU

#include "scene_ui_webgpu.h"

#include <initializer_list>

#include "moksha/client/rendering/shared/shader_constants.h"
#include "moksha/shared/misc/engine_limits.h"

ME::SceneUIWebGPU::SceneUIWebGPU(const WebGPUContext& context, ME::SceneUI* uiScene) {
    scene = uiScene;

    textureAtlasProperties = scene->textureAtlasProperties;
    uiSpriteInstanceData = scene->uiSpriteInstanceData;
    textInstanceData = scene->textInstanceData;

    spriteTextureCount = static_cast<uint32_t>(scene->spriteTexturePaths.count);
    spriteTextures = new ME::TextureWebGPU*[spriteTextureCount];
    textureBindGroups = new WGPUBindGroup[spriteTextureCount];
    for (uint32_t i = 0; i < spriteTextureCount; ++i) {
        spriteTextures[i] = new ME::TextureWebGPU(scene->spriteTexturePaths[i], context.device, context.queue);
        textureBindGroups[i] = PSOWebGPU::CreateTextureBindGroup(context, spriteTextures[i]->GetView());
    }

    // At least one element: a bind group always needs the buffer.
    const uint32_t atlasCount = static_cast<uint32_t>(textureAtlasProperties.count);
    textureAtlasBuffer = new ME::UploadBufferWebGPU(
        context.device, context.queue, true, atlasCount > 0 ? atlasCount : 1, sizeof(ME::TextureAtlasProperties));

    uiSpritePassBuffer =
        new ME::UploadBufferWebGPU(context.device, context.queue, true, 1, sizeof(ME::CBPerPassUISprite));
    uiSpritePassBindGroup = PSOWebGPU::CreatePassBindGroup(context, uiSpritePassBuffer, textureAtlasBuffer);
    textPassBuffer = new ME::UploadBufferWebGPU(context.device, context.queue, true, 1, sizeof(ME::CBPerPassUIText));
    textPassBindGroup = PSOWebGPU::CreatePassBindGroup(context, textPassBuffer, textureAtlasBuffer);

    // At capacity, so dynamic UI elements added after construction fit.
    uiSpriteInstanceBuffer =
        new ME::UploadBufferWebGPU(context.device, context.queue, false, Constants::MaxUISpriteInstanceDataCount,
                                   sizeof(ME::UISpriteRendererInstanceData));
    uiSpriteInstanceBindGroup = PSOWebGPU::CreateInstanceBindGroup(context, uiSpriteInstanceBuffer);
    textInstanceBuffer =
        new ME::UploadBufferWebGPU(context.device, context.queue, false, Constants::MaxTextInstanceDataCount,
                                   sizeof(ME::TextRendererInstanceData));
    textInstanceBindGroup = PSOWebGPU::CreateInstanceBindGroup(context, textInstanceBuffer);
}

ME::SceneUIWebGPU::~SceneUIWebGPU() {
    for (uint32_t i = 0; i < spriteTextureCount; ++i) {
        if (textureBindGroups[i] != nullptr) {
            wgpuBindGroupRelease(textureBindGroups[i]);
        }
        delete spriteTextures[i];
    }
    delete[] textureBindGroups;
    delete[] spriteTextures;

    for (WGPUBindGroup group :
         {uiSpritePassBindGroup, textPassBindGroup, uiSpriteInstanceBindGroup, textInstanceBindGroup}) {
        wgpuBindGroupRelease(group);
    }

    delete textureAtlasBuffer;
    delete uiSpritePassBuffer;
    delete textPassBuffer;
    delete uiSpriteInstanceBuffer;
    delete textInstanceBuffer;
}

void ME::SceneUIWebGPU::Update() {
    // UI built after construction adds elements; the arrays are allocated once at capacity, so only counts change.
    uiSpriteInstanceData.count = scene->uiSpriteInstanceData.count;
    textInstanceData.count = scene->textInstanceData.count;
}

#endif  // ME_WEBGPU
