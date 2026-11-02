/**
 * Renderer that draws nothing and needs no GPU: the headless client's, and a windowed platform's until its GPU
 * renderer is ported. Updates the game, UI and debug scenes every frame like the GPU renderers do.
 */
#pragma once

#include "moksha/client/rendering/shared/renderer.h"

namespace ME {

class WindowState;

class RendererNull : public PlatformRenderer {
   public:
    // Keeps the app's window state; the size is only used for layout, nothing is created.
    bool InitNull(const WindowState* inWindow);

    void SetScenes(ME::Scene* gameScene, ME::SceneUI* inUiScene) override;
    void SetDebugUIScene(ME::SceneUI* inDebugUiScene) override;

    // Updates the scenes that were set.
    void Update() override;

    // Does nothing; kept so the app drives every renderer the same way.
    void Draw() override;

    void End() override;

    // Does nothing: there is no GPU to wait for.
    void OnPause() override;

    void SetVsyncEnabled(bool enabled) override;

   private:
    ME::Scene* scene = nullptr;
    ME::SceneUI* uiScene = nullptr;
    ME::SceneUI* debugUiScene = nullptr;
    const WindowState* window = nullptr;
};

}  // namespace ME
