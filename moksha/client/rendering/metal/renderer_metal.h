#pragma once
#ifdef ME_METAL

/**
 * Renderer class for Metal on macOS, on the Metal 4 API. Draws into the window's view through MetalView; for now it
 * only clears and presents. Must draw from inside the view's draw, where the view hands out a fresh drawable.
 */

#include <cstdint>

#include "moksha/client/rendering/shared/renderer.h"

namespace MTL {
class Device;
class SharedEvent;
}  // namespace MTL

namespace MTL4 {
class CommandAllocator;
class CommandBuffer;
class CommandQueue;
}  // namespace MTL4

namespace ME {

class MetalView;
class Scene;
class SceneUI;
class WindowState;

/**
 * Renderer class for Metal on macOS.
 */
class RendererMetal : public PlatformRenderer {
   public:
    // Init game
    void Init() override;

    // Sets scene for the renderer.
    void SetScene(ME::Scene* gameScene) override;

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

    void Draw();

    void SetVsyncEnabled(bool enabled) override;

   private:
    bool vsyncEnabled = true;

    ME::Scene* scene = nullptr;
    ME::SceneUI* uiScene = nullptr;
    ME::SceneUI* debugUiScene = nullptr;

    const MetalView* view = nullptr;
    const WindowState* window = nullptr;
    MTL::Device* device = nullptr;
    MTL4::CommandQueue* commandQueue = nullptr;

    // One frame in flight: the allocator is reset at the start of each frame, after the GPU finished the last.
    MTL4::CommandAllocator* commandAllocator = nullptr;
    MTL4::CommandBuffer* commandBuffer = nullptr;

    // Signalled by the queue with currentFence when a frame's work completes.
    MTL::SharedEvent* fence = nullptr;
    uint64_t currentFence = 0;

    // TEMP: drives the clear colour cycle in Draw.
    uint64_t frameCount = 0;

    // Blocks until the GPU has finished everything committed so far.
    void FlushCommandQueue();
};

}  // namespace ME

#endif  // ME_METAL
