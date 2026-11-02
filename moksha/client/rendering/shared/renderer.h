/**
 * What every graphics backend implements. ClientApp holds its platform's backend directly and creates it with the
 * backend's own Init (InitDX, InitMetal, InitNull), whose parameters differ; everything after that is on this
 * interface.
 */

#pragma once

#include <cstdint>

#include "moksha/client/scene/scene.h"
#include "moksha/client/scene/scene_ui.h"

namespace ME {

class PlatformRenderer {
   public:
    virtual ~PlatformRenderer() = default;

    // Updates the scenes that were set. Called each frame right before Draw.
    virtual void Update() = 0;

    // Draws the scenes that were set.
    virtual void Draw() = 0;

    // Waits for the GPU, then releases the backend's GPU resources.
    virtual void End() = 0;

    // The app is pausing and may be suspended next, so no GPU work may be left running.
    virtual void OnPause() = 0;

    // Sets both game and UI scenes for the renderer.
    virtual void SetScenes(ME::Scene* gameScene, ME::SceneUI* uiScene) = 0;

    // Sets the debug system's UI scene, drawn in its own pass after the main UI scene.
    virtual void SetDebugUIScene(ME::SceneUI* debugUiScene) = 0;

    virtual void SetVsyncEnabled(bool enabled) = 0;
};

}  // namespace ME
