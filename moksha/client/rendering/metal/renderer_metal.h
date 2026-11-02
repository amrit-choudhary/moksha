#pragma once
#ifdef ME_METAL

/**
 * Renderer class for Metal on macOS, on the Metal 4 API, mirroring RendererDX pass for pass. Draws into the window's
 * view through MetalView, from inside the view's draw, where the view hands out a fresh drawable.
 */

#include <cstdint>

#include "moksha/client/rendering/shared/renderer.h"
#include "moksha/shared/misc/engine_limits.h"

namespace MTL {
class DepthStencilState;
class Device;
class RenderPipelineState;
class ResidencySet;
class SharedEvent;
class Texture;
}  // namespace MTL

namespace MTL4 {
class ArgumentTable;
class CommandAllocator;
class CommandBuffer;
class CommandQueue;
class Compiler;
class RenderCommandEncoder;
}  // namespace MTL4

namespace ME {

class MetalView;
class QuadMetal;
class SpriteRendererInstanceData;
class UploadBufferMetal;
class Scene;
class SceneMetal;
class SceneUI;
class SceneUIMetal;
class WindowState;

/**
 * Renderer class for Metal on macOS.
 */
class RendererMetal : public PlatformRenderer {
   public:
    // Sets both game and UI scenes for the renderer.
    void SetScenes(ME::Scene* gameScene, ME::SceneUI* uiScene) override;

    // Sets the debug system's UI scene, drawn in its own pass after the main UI scene.
    void SetDebugUIScene(ME::SceneUI* debugUiScene) override;

    // Updates the scenes that were set.
    void Update() override;

    // Waits for the GPU, then releases every Metal object.
    void End() override;

    // Initialize Metal for view, using its device. view must outlive the renderer; window is the app's window state.
    bool InitMetal(const MetalView* inView, const WindowState* inWindow);

    void Draw() override;

    // Waits for the GPU, so nothing is left running if the OS suspends the app.
    void OnPause() override;

    void SetVsyncEnabled(bool enabled) override;

   private:
    bool vsyncEnabled = true;

    ME::Scene* scene = nullptr;
    ME::SceneMetal* sceneMetal = nullptr;
    ME::SceneUI* uiScene = nullptr;
    ME::SceneUIMetal* uiSceneMetal = nullptr;
    ME::SceneUI* debugUiScene = nullptr;
    ME::SceneUIMetal* debugUiSceneMetal = nullptr;

    const MetalView* view = nullptr;
    const WindowState* window = nullptr;
    MTL::Device* device = nullptr;
    MTL4::CommandQueue* commandQueue = nullptr;

    // One allocator per frame in flight, reset when its frame comes round again; the command buffer is reused.
    MTL4::CommandAllocator* commandAllocators[Constants::FramesInFlight] = {};
    MTL4::CommandBuffer* commandBuffer = nullptr;

    // Picks the allocator and upload buffer copies the frame being recorded uses.
    uint32_t frameIndex = 0;

    // Signalled by the queue with currentFence when a frame's work completes.
    MTL::SharedEvent* fence = nullptr;
    uint64_t currentFence = 0;
    // The fence value that frees each frame slot; 0 until the slot's first use.
    uint64_t frameFences[Constants::FramesInFlight] = {};

    // Compiles the shaders and pipelines at startup.
    MTL4::Compiler* compiler = nullptr;

    // Resource bindings for every pass, at the indices in MetalBindings; Metal snapshots it at each draw.
    MTL4::ArgumentTable* argumentTable = nullptr;

    // Every buffer and texture the renderer and its scenes create; added to the queue with the layer's set.
    MTL::ResidencySet* residencySet = nullptr;

    // Pipeline for 2D rendering with texture atlases. No instancing.
    MTL::RenderPipelineState* pso2DAtl = nullptr;
    // Pipeline for 2D rendering with instancing and texture atlases.
    MTL::RenderPipelineState* pso2DInsAtl = nullptr;
    // Pipeline for UI Text rendering.
    MTL::RenderPipelineState* pso2DUIText = nullptr;
    // Pipeline for UI Sprite rendering.
    MTL::RenderPipelineState* pso2DUISprite = nullptr;

    // Depth test and writes for the sprite passes; none for the UI passes.
    MTL::DepthStencilState* depthStateDefault = nullptr;
    MTL::DepthStencilState* depthStateDisabled = nullptr;

    // Memoryless depth buffer, sized to the drawable; recreated when the drawable's size changes.
    MTL::Texture* depthTexture = nullptr;

    ME::QuadMetal* quad = nullptr;

    // Sets the game scene, replacing the old one's Metal objects.
    void SetScene(ME::Scene* gameScene);

    // Blocks until the GPU has finished everything committed so far. No-op if InitMetal failed.
    void FlushCommandQueue();

    // Blocks until the queue has signalled value. False, with an error logged, if the timeout passes first.
    bool WaitForFence(uint64_t value, uint64_t timeoutMilliseconds);

    // Issues one instanced sprite set: its texture and atlas slot, instance data and count. Shared by both sets.
    void DrawInstancedSpriteSet(MTL4::RenderCommandEncoder* encoder, uint32_t textureIndex,
                                const ME::SpriteRendererInstanceData* instanceData,
                                ME::UploadBufferMetal* instanceBuffer, uint32_t instanceCount);

    // Issues the UI Sprite + UI Text draw passes for one SceneUIMetal. Shared by the main UI scene and the debug
    // system's UI scene: same pipelines, different data. No-op if uiSceneMetal is nullptr.
    void DrawUIScene(MTL4::RenderCommandEncoder* encoder, ME::SceneUIMetal* uiSceneMetal);

    // Makes depthTexture width x height pixels, waiting for the GPU and replacing it if the size changed.
    void EnsureDepthTexture(uint32_t width, uint32_t height);
};

}  // namespace ME

#endif  // ME_METAL
