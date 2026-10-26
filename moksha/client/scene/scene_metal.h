#pragma once
#ifdef ME_METAL

/**
 * Takes a cross platform scene and creates its Metal resources: the counterpart of SceneDX. Sprite textures and
 * constant buffers are created here and added to the renderer's residency set (Metal 4 makes nothing resident by
 * itself); the renderer commits the set. The game's Scene is read only while constructing.
 */

#include <cstdint>

#include "moksha/client/rendering/metal/texture_metal.h"
#include "moksha/client/rendering/metal/upload_buffer_metal.h"
#include "scene.h"

namespace MTL {
class Device;
class ResidencySet;
}  // namespace MTL

namespace ME {

class SceneMetal {
   public:
    SceneMetal() = delete;
    SceneMetal(MTL::Device* device, MTL::ResidencySet* residencySet, ME::Scene* gameScene);
    ~SceneMetal();

    SceneMetal(const SceneMetal&) = delete;
    SceneMetal& operator=(const SceneMetal&) = delete;

    // Updates Metal-side data before drawing; called from RendererMetal::Update after Scene::Update.
    void Update();

    ME::Light* ambientLight = nullptr;
    ME::Light* directionalLight = nullptr;
    ME::Camera* spriteCamera = nullptr;

    ME::TextureMetal** spriteTextures = nullptr;
    uint32_t spriteTextureCount = 0;
    ME::Span<ME::TextureAtlasProperties> textureAtlasProperties;
    ME::Span<ME::Transform> spriteTransforms;
    ME::Span<ME::SpriteRenderer> spriteRenderers;

    // Instanced sets, sized once from their counts at construction (game_guide: "Instanced sets are sized once").
    ME::Span<ME::SpriteRenderer> instancedSpriteRenderers0;
    ME::SpriteRendererInstanceData* spriteInstanceData0 = nullptr;
    ME::UploadBufferMetal* spriteInstanceBuffer0 = nullptr;

    ME::Span<ME::SpriteRenderer> instancedSpriteRenderers1;
    ME::SpriteRendererInstanceData* spriteInstanceData1 = nullptr;
    ME::UploadBufferMetal* spriteInstanceBuffer1 = nullptr;

    uint32_t perPassCBCount = 0;
    uint32_t textureAtlasCBCount = 0;
    uint32_t perSpriteCBCount = 0;

    ME::UploadBufferMetal** perPassCBs = nullptr;
    ME::UploadBufferMetal** perSpriteCBs = nullptr;
    ME::UploadBufferMetal** textureAtlasCBs = nullptr;

   private:
    MTL::Device* device = nullptr;
    MTL::ResidencySet* residencySet = nullptr;
    ME::Scene* scene = nullptr;

    void MakeSpriteTextures();
    void MakeConstantBuffers();
    void MakeSpriteInstanceBuffer();

    // Adds the buffer to the residency set and returns it.
    ME::UploadBufferMetal* AddResident(ME::UploadBufferMetal* buffer);
};

}  // namespace ME

#endif  // ME_METAL
