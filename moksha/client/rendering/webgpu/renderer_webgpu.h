#pragma once
#ifdef ME_WEBGPU

/**
 * Renderer class for WebGPU. Draws into the view's surface once per Tick. The browser presents the frame when control
 * returns to it, and keeps resources alive for the GPU, so there are no fences or frames in flight here.
 */

#include <webgpu/webgpu.h>

#include "moksha/client/rendering/shared/renderer.h"

namespace ME {

class WebGPUView;
class WindowState;

class RendererWebGPU : public PlatformRenderer {
   public:
    // Configures the view's surface. view must outlive the renderer; window is the app's window state.
    bool InitWebGPU(const WebGPUView* inView, const WindowState* inWindow);

    void SetScenes(ME::Scene* gameScene, ME::SceneUI* inUiScene) override;
    void SetDebugUIScene(ME::SceneUI* inDebugUiScene) override;

    // Updates the scenes that were set.
    void Update() override;

    void Draw() override;

    void End() override;

    // Does nothing: the browser owns the GPU's work.
    void OnPause() override;

    // Does nothing: the browser always presents on the display's refresh.
    void SetVsyncEnabled(bool enabled) override;

   private:
    ME::Scene* scene = nullptr;
    ME::SceneUI* uiScene = nullptr;
    ME::SceneUI* debugUiScene = nullptr;

    const WebGPUView* view = nullptr;
    const WindowState* window = nullptr;
    WGPUDevice device = nullptr;
    WGPUQueue queue = nullptr;

    // sRGB view of the surface's format, which the renderer draws through.
    WGPUTextureFormat renderFormat = WGPUTextureFormat_Undefined;
};

}  // namespace ME

#endif  // ME_WEBGPU
