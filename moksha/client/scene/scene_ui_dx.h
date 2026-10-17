#pragma once
#ifdef ME_WIN

/**
 * It takes a cross platform scene and converts it to a DirectX 12 scene.
 * This will create DirectX specific objects like buffers, textures, shaders, etc.
 */
#include <d3d12.h>

#include "moksha/client/rendering/directx/d3dx12.h"
#include "moksha/client/rendering/directx/desc_heap_manager_dx.h"
#include "moksha/client/rendering/directx/texture_dx.h"
#include "moksha/client/rendering/directx/upload_buffer_dx.h"
#include "moksha/shared/datastructure/span.h"
#include "scene_ui.h"

namespace ME {

class SceneUIDX {
   public:
    SceneUIDX() = delete;
    SceneUIDX(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, ME::DescHeapManagerDX* descHeapManager,
              ME::SceneUI* uiScene);
    ~SceneUIDX();

    // Init game
    void Init();

    // Cleanup upload buffers and other temp resources right after initialization.
    void PostInitCleanup();

    /**
     * Updates the rendering related data in the scene, if any render element is marked as dirty.
     * This is called from Renderer.Update(), right before Renderer.Draw().
     * First SceneUI::Update() is called, then this DX specific update is called.
     */
    void Update();

    void End();

    ME::TextureDX** spriteTextures;
    ME::Span<ME::TextureAtlasProperties> textureAtlasProperties;

    ME::Span<ME::Transform> uiSpriteTransforms;
    ME::Span<ME::SpriteRenderer> uiSpriteRenderers;
    ME::Span<ME::Transform> textTransforms;
    ME::Span<ME::TextRenderer> textRenderers;

    uint32_t perPassCBCount = 0;
    uint32_t textureAtlasCBCount = 0;

    ME::Span<ME::UISpriteRendererInstanceData> uiSpriteInstanceData;
    ME::UploadBufferDX* uiSpriteInstanceBuffer = nullptr;
    uint32_t uiSpriteInstanceBufferHeapIndex = 0;

    ME::Span<ME::TextRendererInstanceData> textInstanceData;
    ME::UploadBufferDX* textInstanceBuffer = nullptr;
    uint32_t textInstanceBufferHeapIndex = 0;

    ME::UploadBufferDX** perPassCBs = nullptr;
    uint32_t* perPassCBHeapIndices = nullptr;  // Map of heap indices for perPassCBs for gpu binding later.

    ME::UploadBufferDX** textureAtlasCBs = nullptr;
    uint32_t* textureAtlasCBHeapIndices = nullptr;  // Map of heap indices for texture atlas CBs for gpu binding later.

   private:
    ID3D12Device* device = nullptr;
    ID3D12GraphicsCommandList* cmdList = nullptr;
    ME::DescHeapManagerDX* descHeapManager = nullptr;
    ME::SceneUI* scene = nullptr;

    void MakeSpriteTextures();
    void MakeConstantBuffers();

    void MakeUISpriteInstanceBuffer();
    void MakeTextInstanceBuffer();
};

}  // namespace ME

#endif  // ME_WIN
