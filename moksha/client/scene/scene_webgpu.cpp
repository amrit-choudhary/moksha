#ifdef ME_WEBGPU

#include "scene_webgpu.h"

#include <initializer_list>

#include "moksha/client/rendering/shared/shader_constants.h"
#include "moksha/logging/logging.h"
#include "moksha/shared/misc/engine_limits.h"

ME::SceneWebGPU::SceneWebGPU(const WebGPUContext& context, ME::Scene* gameScene) {
    ambientLight = gameScene->ambientLight;
    directionalLight = gameScene->directionalLight;
    spriteCamera = gameScene->spriteCamera;

    textureAtlasProperties = gameScene->textureAtlasProperties;
    spriteTransforms = gameScene->spriteTransforms;
    spriteRenderers = gameScene->spriteRenderers;

    instancedSpriteRenderers0 = gameScene->instancedSpriteRenderers0;
    spriteInstanceData0 = gameScene->spriteInstanceData0;
    instancedSpriteRenderers1 = gameScene->instancedSpriteRenderers1;
    spriteInstanceData1 = gameScene->spriteInstanceData1;

    MakeConstantBuffers(context);
    MakeSpriteTextures(context, gameScene);
    MakeSpriteInstanceBuffers(context);
}

ME::SceneWebGPU::~SceneWebGPU() {
    for (uint32_t i = 0; i < spriteTextureCount; ++i) {
        if (textureBindGroups[i] != nullptr) {
            wgpuBindGroupRelease(textureBindGroups[i]);
        }
        delete spriteTextures[i];
    }
    delete[] textureBindGroups;
    delete[] spriteTextures;

    for (WGPUBindGroup group :
         {passBindGroup, perSpriteBindGroup, spriteInstanceBindGroup0, spriteInstanceBindGroup1}) {
        if (group != nullptr) {
            wgpuBindGroupRelease(group);
        }
    }

    delete perPassBuffer;
    delete textureAtlasBuffer;
    delete perSpriteBuffer;
    delete spriteInstanceBuffer0;
    delete spriteInstanceBuffer1;
}

void ME::SceneWebGPU::MakeSpriteTextures(const WebGPUContext& context, ME::Scene* scene) {
    spriteTextureCount = static_cast<uint32_t>(scene->spriteTexturePaths.count);
    spriteTextures = new ME::TextureWebGPU*[spriteTextureCount];
    textureBindGroups = new WGPUBindGroup[spriteTextureCount];
    for (uint32_t i = 0; i < spriteTextureCount; ++i) {
        spriteTextures[i] = new ME::TextureWebGPU(scene->spriteTexturePaths[i], context.device, context.queue);
        textureBindGroups[i] = PSOWebGPU::CreateTextureBindGroup(context, spriteTextures[i]->GetView());
    }
}

void ME::SceneWebGPU::MakeConstantBuffers(const WebGPUContext& context) {
    perPassBuffer = new ME::UploadBufferWebGPU(context.device, context.queue, true, 1, sizeof(ME::CBPerPass));

    // At least one element: a bind group always needs the buffer, even in a scene without atlases.
    const uint32_t atlasCount = static_cast<uint32_t>(textureAtlasProperties.count);
    textureAtlasBuffer = new ME::UploadBufferWebGPU(
        context.device, context.queue, true, atlasCount > 0 ? atlasCount : 1, sizeof(ME::TextureAtlasProperties));
    passBindGroup = PSOWebGPU::CreatePassBindGroup(context, perPassBuffer, textureAtlasBuffer);

    perSpriteCount = static_cast<uint32_t>(spriteRenderers.count);
    if (perSpriteCount > Constants::MaxSpriteRendererCount) {
        ME::LogError("SceneWebGPU: more sprite renderers than MaxSpriteRendererCount; the extra ones are not drawn");
        perSpriteCount = static_cast<uint32_t>(Constants::MaxSpriteRendererCount);
    }
    if (perSpriteCount != 0) {
        perSpriteBuffer =
            new ME::UploadBufferWebGPU(context.device, context.queue, true, perSpriteCount, sizeof(ME::CBPerSprite));
        perSpriteBindGroup = PSOWebGPU::CreatePerSpriteBindGroup(context, perSpriteBuffer);
    }
}

void ME::SceneWebGPU::MakeSpriteInstanceBuffers(const WebGPUContext& context) {
    if (instancedSpriteRenderers0.count != 0) {
        spriteInstanceBuffer0 = new ME::UploadBufferWebGPU(context.device, context.queue, false,
                                                           static_cast<uint32_t>(instancedSpriteRenderers0.count),
                                                           sizeof(ME::SpriteRendererInstanceData));
        spriteInstanceBindGroup0 = PSOWebGPU::CreateInstanceBindGroup(context, spriteInstanceBuffer0);
    }
    if (instancedSpriteRenderers1.count != 0) {
        spriteInstanceBuffer1 = new ME::UploadBufferWebGPU(context.device, context.queue, false,
                                                           static_cast<uint32_t>(instancedSpriteRenderers1.count),
                                                           sizeof(ME::SpriteRendererInstanceData));
        spriteInstanceBindGroup1 = PSOWebGPU::CreateInstanceBindGroup(context, spriteInstanceBuffer1);
    }
}

#endif  // ME_WEBGPU
