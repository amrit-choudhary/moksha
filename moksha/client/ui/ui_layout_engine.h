#pragma once

/**
 * UI Layout Engine: computes anchor-based absolute UIRects for a tree of UIElements.
 * Purpose-built subsystem owned by UISystem — stateless computation only, no element registry
 * of its own (that lives on SceneUI).
 */

#include <cstdint>

#include "ui_rect.h"

namespace ME {

class UIElement;

class UILayoutEngine {
   public:
    UILayoutEngine();
    ~UILayoutEngine();

    void Init();
    void End();

    // Walks `elements` (all of them, roots and children alike), finds roots
    // (GetParent() == nullptr), and computes each one's absolute UIRect top-down against the
    // screen rect, then recurses into children.
    void RecalculateLayouts(UIElement** elements, uint32_t count);

   private:
    // Computes `element`'s absolute UIRect from the given parent rect (accessing its protected
    // UIRect member directly via the friend grant on UIElement), then recurses into its children
    // using its own just-computed absolute rect as their parent rect.
    void ResolveElement(UIElement* element, const UIRect& parentRect);
};

}  // namespace ME
