#pragma once

/**
 * Button: Interactive UI element with no owned visual (same shape as Container) — callers attach whatever
 * children they want via AddChild(). Generic across any game built on this engine.
 */

#include "moksha/client/rendering/shared/color.h"
#include "moksha/shared/misc/delegate.h"
#include "ui_element.h"

namespace ME {

class Panel;

class Button : public UIElement {
   public:
    Button();
    ~Button();

    UIElementType GetType() const override;

    // Called each frame by UISystem with the mouse position (in UI space, same as GetRect()) and
    // current left-mouse-button down state, to drive the hover/press/click state machine.
    void UpdateInteraction(const Vec2& mousePosUISpace, bool mouseDown);

    // Drops hover/press state, e.g. while hidden. Fires onHoverExit/onRelease if they were active, never onClick.
    void ResetInteraction();

    bool IsHovered() const;
    bool IsPressed() const;

    // Optional: if set, Button applies normal/hover/pressed colors to this Panel on state
    // transitions. Button doesn't own it — it must already be attached via AddChild().
    void SetPanel(Panel* newPanel);

    void SetNormalColor(const Color& newColor);
    void SetHoverColor(const Color& newColor);
    void SetPressedColor(const Color& newColor);

    Delegate onHoverEnter;
    Delegate onHoverExit;
    Delegate onPress;    // mouse button down while hovered
    Delegate onRelease;  // mouse button up, following onPress (regardless of whether still hovered)
    Delegate onClick;    // onRelease while still hovered — a completed click

   private:
    void ApplyPanelColorForCurrentState();

    bool isHovered = false;
    bool isPressed = false;

    Color normalColor = Color::White();
    Color hoverColor = Color::LightGray();
    Color pressedColor = Color::DarkGray();

    Panel* panel = nullptr;
};

}  // namespace ME
