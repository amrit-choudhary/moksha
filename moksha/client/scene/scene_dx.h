#pragma once
#ifdef ME_DX12

/**
 * It takes a cross platform scene and converts it to a DirectX 12 scene.
 * This will create DirectX specific objects like buffers, textures, shaders, etc.
 */

#include <d3d12.h>

#include "moksha/client/rendering/directx/d3dx12.h"
#include "moksha/client/rendering/directx/desc_heap_manager_dx.h"
#include "moksha/client/rendering/directx/mesh_dx.h"
#include "moksha/client/rendering/directx/quad_dx.h"
#include "moksha/client/rendering/directx/texture_dx.h"
#include "moksha/client/rendering/directx/upload_buffer_dx.h"
#include "scene.h"

namespace ME {

class SceneDX {
   public:
    SceneDX() = delete;
    SceneDX(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, ME::DescHeapManagerDX* descHeapManager,
            ME::Scene* gameScene);
    ~SceneDX();

    // Init game
    void Init();

    // Cleanup upload buffers and other temp resources right after initialization.
    void PostInitCleanup();

    /**
     * Updates the rendering related data in the scene, if any render element is marked as dirty.
     * This is called from Renderer.Update(), right before Renderer.Draw().
     * First Scene::Update() is called, then this DX specific update is called.
     */
    void Update();

    void End();

    ME::Light* ambientLight;
    ME::Light* directionalLight;
    ME::Camera* camera;
    ME::Camera* spriteCamera;

    ME::MeshDX** meshes;
    ME::QuadDX** quads;
    ME::TextureDX** textures;
    ME::TextureDX** spriteTextures;
    ME::Span<ME::TextureAtlasProperties> textureAtlasProperties;
    ME::Span<ME::Transform> transforms;
    ME::Span<ME::MeshRenderer> meshRenderers;
    ME::Span<ME::Transform> spriteTransforms;
    ME::Span<ME::SpriteRenderer> spriteRenderers;

    ME::Span<ME::Transform> instancedSpriteTransforms0;
    ME::Span<ME::SpriteRenderer> instancedSpriteRenderers0;

    ME::Span<ME::Transform> instancedSpriteTransforms1;
    ME::Span<ME::SpriteRenderer> instancedSpriteRenderers1;

    uint32_t perPassCBCount = 0;
    uint32_t textureAtlasCBCount = 0;
    uint32_t perSpriteCBCount = 0;

    ME::SpriteRendererInstanceData* spriteInstanceData0;
    ME::UploadBufferDX* spriteInstanceBuffer0 = nullptr;
    uint32_t spriteInstanceBufferHeapIndex0 = 0;

    ME::SpriteRendererInstanceData* spriteInstanceData1;
    ME::UploadBufferDX* spriteInstanceBuffer1 = nullptr;
    uint32_t spriteInstanceBufferHeapIndex1 = 0;

    ME::UploadBufferDX** perPassCBs = nullptr;
    uint32_t* perPassCBHeapIndices = nullptr;  // Map of heap indices for perPassCBs for gpu binding later.

    ME::UploadBufferDX** perSpriteCBs = nullptr;
    uint32_t* perSpriteCBHeapIndices = nullptr;  // Map of heap indices for perSpriteCBs for gpu binding later.

    ME::UploadBufferDX** textureAtlasCBs = nullptr;
    uint32_t* textureAtlasCBHeapIndices = nullptr;  // Map of heap indices for texture atlas CBs for gpu binding later.

   private:
    ID3D12Device* device = nullptr;
    ID3D12GraphicsCommandList* cmdList = nullptr;
    ME::DescHeapManagerDX* descHeapManager = nullptr;
    ME::Scene* scene = nullptr;

    void MakeMeshes();
    void MakeQuads();
    void MakeTextures();
    void MakeSpriteTextures();
    void MakeShaders();
    void MakeTextureSamplers();
    void MakeConstantBuffers();

    void MakeSpriteInstanceBuffer();
};

}  // namespace ME

#endif  // ME_DX12
