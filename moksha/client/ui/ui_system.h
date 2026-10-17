#pragma once

/**
 * UISystem: the umbrella manager for all UI concerns (layout now; input/mouse, screen scaling,
 * and safe-area handling later, without changing this public surface). Matches PhysicsSystem's
 * shape: takes a SceneUI* (the dumb data owner of the UIElement tree) and runs logic on it each
 * frame.
 */

#include <cstdint>

#include "ui_layout_engine.h"

namespace ME {

class SceneUI;
class UIElement;
class SpriteRenderer;
class TextRenderer;
class Transform;

class UISystem {
   public:
    UISystem();
    ~UISystem();

    void Init();
    void Update(double deltaTime);
    void End();

    void SetUIScene(SceneUI* newUiScene);

    // Returns false if no SceneUI has been set (see SetUIScene) or it's at capacity
    // (Constants::MaxUIElementCount).
    bool AddElement(UIElement* element);
    void RemoveElement(UIElement* element);

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
    void UpdateButtonInteractions();
};

}  // namespace ME
