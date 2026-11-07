#include "ui_layout_engine.h"

#include "moksha/client/app/app_config.h"
#include "ui_element.h"

ME::UILayoutEngine::UILayoutEngine() {}

ME::UILayoutEngine::~UILayoutEngine() {}

void ME::UILayoutEngine::Init(const WindowState* inWindow) {
    window = inWindow;
}

void ME::UILayoutEngine::End() {}

void ME::UILayoutEngine::RecalculateLayouts(UIElement** elements, uint32_t count) {
    // Root elements resolve against the window shrunk by its safe area insets.
    Vec2 safeOrigin = Vec2::Zero;
    Vec2 safeSize = Vec2::Zero;
    if (window != nullptr) {
        const float width = static_cast<float>(window->width) - window->safeAreaLeft - window->safeAreaRight;
        const float height = static_cast<float>(window->height) - window->safeAreaTop - window->safeAreaBottom;
        safeOrigin = Vec2{window->safeAreaLeft, window->safeAreaTop};
        safeSize = Vec2{width > 0.0f ? width : 0.0f, height > 0.0f ? height : 0.0f};
    }

    // TopLeft anchor/pivot means the dummy parent's own (zero) size never factors in, so an empty UIRect is a safe
    // stand-in; the offset places the rect at the safe area's top-left corner.
    UIRect screenRect;
    screenRect.ComputeAbsolute(UIRect(), UIAnchor::TopLeft, UIPivot::TopLeft, safeOrigin, safeSize);

    for (uint32_t i = 0; i < count; ++i) {
        UIElement* element = elements[i];
        if (element->GetParent() == nullptr) {
            ResolveElement(element, screenRect);
        }
    }
}

void ME::UILayoutEngine::ResolveElement(UIElement* element, const UIRect& parentRect) {
    element->rect.ComputeAbsolute(parentRect, element->GetAnchor(), element->GetPivot(), element->GetOffset(),
                                  element->GetSize());

    UIElement** children = element->GetChildren();
    size_t childCount = element->GetChildCount();
    for (size_t i = 0; i < childCount; ++i) {
        ResolveElement(children[i], *element->GetRect());
    }
}
