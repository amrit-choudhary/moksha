#pragma once
#ifdef ME_METAL

/**
 * Takes a cross platform UI scene and creates its Metal resources: the counterpart of SceneUIDX. The UI atlas
 * (texture slot 0) and font (slot 1), the per-pass and atlas constant buffers, and the UI sprite and text instance
 * buffers, sized at their maximum counts so UI elements can be added later. Everything goes in the renderer's
 * residency set; the renderer commits it.
 */

#include <cstdint>

#include "moksha/client/rendering/metal/texture_metal.h"
#include "moksha/client/rendering/metal/upload_buffer_metal.h"
#include "scene_ui.h"

namespace MTL {
class Device;
class ResidencySet;
}  // namespace MTL

namespace ME {

class SceneUIMetal {
   public:
    SceneUIMetal() = delete;
    SceneUIMetal(MTL::Device* device, MTL::ResidencySet* residencySet, ME::SceneUI* uiScene);
    ~SceneUIMetal();

    SceneUIMetal(const SceneUIMetal&) = delete;
    SceneUIMetal& operator=(const SceneUIMetal&) = delete;

    // Re-reads the UI scene's live counts (elements can be added after construction); called from
    // RendererMetal::Update after SceneUI::Update, never after the UI scene is gone.
    void Update();

    ME::TextureMetal** spriteTextures = nullptr;
    uint32_t spriteTextureCount = 0;
    ME::Span<ME::TextureAtlasProperties> textureAtlasProperties;

    ME::Span<ME::UISpriteRendererInstanceData> uiSpriteInstanceData;
    ME::UploadBufferMetal* uiSpriteInstanceBuffer = nullptr;

    ME::Span<ME::TextRendererInstanceData> textInstanceData;
    ME::UploadBufferMetal* textInstanceBuffer = nullptr;

    uint32_t perPassCBCount = 0;
    uint32_t textureAtlasCBCount = 0;

    // [0] UI sprite pass, [1] UI text pass, as in SceneUIDX.
    ME::UploadBufferMetal** perPassCBs = nullptr;
    ME::UploadBufferMetal** textureAtlasCBs = nullptr;

   private:
    MTL::Device* device = nullptr;
    MTL::ResidencySet* residencySet = nullptr;
    ME::SceneUI* scene = nullptr;

    void MakeSpriteTextures();
    void MakeConstantBuffers();
    void MakeUISpriteInstanceBuffer();
    void MakeTextInstanceBuffer();

    // Adds the buffer to the residency set and returns it.
    ME::UploadBufferMetal* AddResident(ME::UploadBufferMetal* buffer);
};

}  // namespace ME

#endif  // ME_METAL
