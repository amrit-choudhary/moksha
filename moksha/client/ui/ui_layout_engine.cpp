#include "ui_layout_engine.h"

#include "moksha/client/misc/global_vars.h"
#include "ui_element.h"

ME::UILayoutEngine::UILayoutEngine() {}

ME::UILayoutEngine::~UILayoutEngine() {}

void ME::UILayoutEngine::Init() {}

void ME::UILayoutEngine::End() {}

void ME::UILayoutEngine::RecalculateLayouts(UIElement** elements, uint32_t count) {
    Vec2 screenSize =
        Vec2{static_cast<float>(GlobalVars::GetWindowWidth()), static_cast<float>(GlobalVars::GetWindowHeight())};

    // Root elements resolve against the screen itself. TopLeft anchor/pivot + zero offset means
    // the dummy parent's own (zero) size never factors in, so an empty UIRect is a safe stand-in.
    UIRect screenRect;
    screenRect.ComputeAbsolute(UIRect(), UIAnchor::TopLeft, UIPivot::TopLeft, Vec2::Zero, screenSize);

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
