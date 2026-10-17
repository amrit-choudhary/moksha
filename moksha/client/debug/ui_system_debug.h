#pragma once

/**
 * UISystemDebug: the umbrella manager for all UI concerns of Debug Overlay.
 * Separate from UISystem, which is used by the game ui.
 * This will be much simpler than UISystem, as it will only be used for debug overlay,
 * and will not have to handle input, mouse, screen scaling, or safe-area handling.
 */

#include <cstdint>

#include "moksha/client/ui/ui_layout_engine.h"

namespace ME {

class SceneUI;
class SpriteRenderer;
class TextRenderer;
class Transform;

class UISystemDebug {
   public:
    UISystemDebug();
    ~UISystemDebug();

    void Init();
    void Update(double deltaTime);
    void End();

    void SetUIScene(SceneUI* newUiScene);

   private:
    SceneUI* uiScene = nullptr;
    UILayoutEngine layoutEngine;

    // Per-frame scratch pointer arrays for the rebuild walk (sized Constants::MaxUIElementCount).
    // These just collect pointers to each widget's own already-owned Transform/renderer — no
    // Transform allocation happens here.
    Transform** spriteTransformScratch = nullptr;
    SpriteRenderer** spriteRendererScratch = nullptr;
    Transform** textTransformScratch = nullptr;
    TextRenderer** textRendererScratch = nullptr;

    void SyncToScene();
};

}  // namespace ME
