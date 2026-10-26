#pragma once
#ifdef ME_METAL

/**
 * Renderer class for Metal on macOS. Draws into the window's view through MetalView; for now it only clears and
 * presents. Must draw from inside the view's draw, where the view hands out a fresh drawable each frame.
 */

#include "moksha/client/rendering/shared/renderer.h"

namespace MTL {
class CommandQueue;
class Device;
}  // namespace MTL

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

    // Releases the command queue and the device.
    void End() override;

    // Initialize Metal for view, using its device. view must outlive the renderer; window is the app's window state.
    bool InitMetal(const MetalView* inView, const WindowState* inWindow);

    void Draw();

    void SetVsyncEnabled(bool enabled) override;

   private:
    bool vsyncEnabled = true;
    // The layer is only reachable through a drawable, so a vsync change is applied on the next Draw.
    bool vsyncChanged = true;

    ME::Scene* scene = nullptr;
    ME::SceneUI* uiScene = nullptr;
    ME::SceneUI* debugUiScene = nullptr;

    const MetalView* view = nullptr;
    const WindowState* window = nullptr;
    MTL::Device* device = nullptr;
    MTL::CommandQueue* commandQueue = nullptr;
};

}  // namespace ME

#endif  // ME_METAL
