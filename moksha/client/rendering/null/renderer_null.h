/**
 * Renderer of the headless client: draws nothing and needs no window or GPU, but updates the game, UI and debug
 * scenes every frame like the GPU renderers do, so scene state behaves the same without a screen.
 */
#pragma once

#include "moksha/client/rendering/shared/renderer.h"

namespace ME {

class WindowState;

class RendererNull : public PlatformRenderer {
   public:
    void Init() override;

    // Keeps the app's window state; the size is only used for layout, nothing is created.
    bool InitHeadless(const WindowState* inWindow);

    void SetScene(ME::Scene* gameScene) override;
    void SetScenes(ME::Scene* gameScene, ME::SceneUI* inUiScene) override;
    void SetDebugUIScene(ME::SceneUI* inDebugUiScene) override;

    // Updates the scenes that were set.
    void Update() override;

    // Does nothing; kept so the app drives every renderer the same way.
    void Draw();

    void End() override;

    void SetVsyncEnabled(bool enabled) override;

   private:
    ME::Scene* scene = nullptr;
    ME::SceneUI* uiScene = nullptr;
    ME::SceneUI* debugUiScene = nullptr;
    const WindowState* window = nullptr;
};

}  // namespace ME
