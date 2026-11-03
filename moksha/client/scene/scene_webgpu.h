#pragma once
#ifdef ME_WEBGPU

/**
 * Takes a cross platform scene and creates its WebGPU resources: the counterpart of SceneMetal. Bind groups are made
 * here, once, and the renderer only selects them. All per-sprite constants share one buffer, as do all atlas
 * properties, each read at a dynamic offset. The game's Scene is read only while constructing.
 */

#include <cstdint>

#include "moksha/client/rendering/webgpu/pso_webgpu.h"
#include "moksha/client/rendering/webgpu/texture_webgpu.h"
#include "moksha/client/rendering/webgpu/upload_buffer_webgpu.h"
#include "scene.h"

namespace ME {

class SceneWebGPU {
   public:
    SceneWebGPU() = delete;
    SceneWebGPU(const WebGPUContext& context, ME::Scene* gameScene);
    ~SceneWebGPU();

    SceneWebGPU(const SceneWebGPU&) = delete;
    SceneWebGPU& operator=(const SceneWebGPU&) = delete;

    ME::Light* ambientLight = nullptr;
    ME::Light* directionalLight = nullptr;
    ME::Camera* spriteCamera = nullptr;

    ME::TextureWebGPU** spriteTextures = nullptr;
    WGPUBindGroup* textureBindGroups = nullptr;  // Null where the texture couldn't be loaded.
    uint32_t spriteTextureCount = 0;
    ME::Span<ME::TextureAtlasProperties> textureAtlasProperties;
    ME::Span<ME::Transform> spriteTransforms;
    ME::Span<ME::SpriteRenderer> spriteRenderers;

    // Instanced sets, sized once from their counts at construction (game_guide: "Instanced sets are sized once").
    ME::Span<ME::SpriteRenderer> instancedSpriteRenderers0;
    ME::SpriteRendererInstanceData* spriteInstanceData0 = nullptr;
    ME::UploadBufferWebGPU* spriteInstanceBuffer0 = nullptr;
    WGPUBindGroup spriteInstanceBindGroup0 = nullptr;

    ME::Span<ME::SpriteRenderer> instancedSpriteRenderers1;
    ME::SpriteRendererInstanceData* spriteInstanceData1 = nullptr;
    ME::UploadBufferWebGPU* spriteInstanceBuffer1 = nullptr;
    WGPUBindGroup spriteInstanceBindGroup1 = nullptr;

    uint32_t perSpriteCount = 0;

    ME::UploadBufferWebGPU* perPassBuffer = nullptr;
    ME::UploadBufferWebGPU* textureAtlasBuffer = nullptr;
    WGPUBindGroup passBindGroup = nullptr;
    ME::UploadBufferWebGPU* perSpriteBuffer = nullptr;
    WGPUBindGroup perSpriteBindGroup = nullptr;

   private:
    void MakeSpriteTextures(const WebGPUContext& context, ME::Scene* scene);
    void MakeConstantBuffers(const WebGPUContext& context);
    void MakeSpriteInstanceBuffers(const WebGPUContext& context);
};

}  // namespace ME

#endif  // ME_WEBGPU
