#pragma once
#ifdef ME_WEBGPU

/**
 * Takes a cross platform UI scene and creates its WebGPU resources: the counterpart of SceneUIMetal. The UI atlas
 * (texture slot 0) and font (slot 1), the per-pass and atlas constants, and the UI sprite and text instance buffers,
 * sized at their maximum counts so UI elements can be added later.
 */

#include <cstdint>

#include "moksha/client/rendering/webgpu/pso_webgpu.h"
#include "moksha/client/rendering/webgpu/texture_webgpu.h"
#include "moksha/client/rendering/webgpu/upload_buffer_webgpu.h"
#include "scene_ui.h"

namespace ME {

class SceneUIWebGPU {
   public:
    SceneUIWebGPU() = delete;
    SceneUIWebGPU(const WebGPUContext& context, ME::SceneUI* uiScene);
    ~SceneUIWebGPU();

    SceneUIWebGPU(const SceneUIWebGPU&) = delete;
    SceneUIWebGPU& operator=(const SceneUIWebGPU&) = delete;

    // Re-reads the UI scene's live counts (elements can be added after construction); called from
    // RendererWebGPU::Update after SceneUI::Update, never after the UI scene is gone.
    void Update();

    ME::TextureWebGPU** spriteTextures = nullptr;
    WGPUBindGroup* textureBindGroups = nullptr;  // Null where the texture couldn't be loaded.
    uint32_t spriteTextureCount = 0;
    ME::Span<ME::TextureAtlasProperties> textureAtlasProperties;

    ME::Span<ME::UISpriteRendererInstanceData> uiSpriteInstanceData;
    ME::UploadBufferWebGPU* uiSpriteInstanceBuffer = nullptr;
    WGPUBindGroup uiSpriteInstanceBindGroup = nullptr;

    ME::Span<ME::TextRendererInstanceData> textInstanceData;
    ME::UploadBufferWebGPU* textInstanceBuffer = nullptr;
    WGPUBindGroup textInstanceBindGroup = nullptr;

    ME::UploadBufferWebGPU* textureAtlasBuffer = nullptr;

    // UI sprite and UI text per-pass constants, each in a bind group with the atlas buffer.
    ME::UploadBufferWebGPU* uiSpritePassBuffer = nullptr;
    WGPUBindGroup uiSpritePassBindGroup = nullptr;
    ME::UploadBufferWebGPU* textPassBuffer = nullptr;
    WGPUBindGroup textPassBindGroup = nullptr;

   private:
    ME::SceneUI* scene = nullptr;
};

}  // namespace ME

#endif  // ME_WEBGPU
