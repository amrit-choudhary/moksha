#include "button.h"

#include "panel.h"

ME::Button::Button() {}

ME::Button::~Button() {}

ME::UIElementType ME::Button::GetType() const {
    return UIElementType::Button;
}

void ME::Button::UpdateInteraction(const Vec2& mousePosUISpace, bool mouseDown) {
    bool nowHovered = GetRect()->Contains(mousePosUISpace);

    if (nowHovered && !isHovered) {
        isHovered = true;
        onHoverEnter.Execute();
        ApplyPanelColorForCurrentState();
    } else if (!nowHovered && isHovered) {
        isHovered = false;
        onHoverExit.Execute();
        ApplyPanelColorForCurrentState();
    }

    if (isHovered && mouseDown && !isPressed) {
        isPressed = true;
        onPress.Execute();
        ApplyPanelColorForCurrentState();
    } else if (isPressed && !mouseDown) {
        isPressed = false;
        onRelease.Execute();
        if (isHovered) {
            onClick.Execute();
        }
        ApplyPanelColorForCurrentState();
    }
}

void ME::Button::ResetInteraction() {
    if (!isHovered && !isPressed) {
        return;
    }
    if (isPressed) {
        isPressed = false;
        onRelease.Execute();
    }
    if (isHovered) {
        isHovered = false;
        onHoverExit.Execute();
    }
    ApplyPanelColorForCurrentState();
}

bool ME::Button::IsHovered() const {
    return isHovered;
}

bool ME::Button::IsPressed() const {
    return isPressed;
}

void ME::Button::SetPanel(Panel* newPanel) {
    panel = newPanel;
    ApplyPanelColorForCurrentState();
}

void ME::Button::SetNormalColor(const Color& newColor) {
    normalColor = newColor;
    ApplyPanelColorForCurrentState();
}

void ME::Button::SetHoverColor(const Color& newColor) {
    hoverColor = newColor;
    ApplyPanelColorForCurrentState();
}

void ME::Button::SetPressedColor(const Color& newColor) {
    pressedColor = newColor;
    ApplyPanelColorForCurrentState();
}

void ME::Button::ApplyPanelColorForCurrentState() {
    if (panel == nullptr) {
        return;
    }

    if (isPressed && isHovered) {
        panel->SetColor(pressedColor);
    } else if (isHovered) {
        panel->SetColor(hoverColor);
    } else {
        panel->SetColor(normalColor);
    }
}
