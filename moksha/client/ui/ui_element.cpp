#include "ui_element.h"

#include "moksha/shared/misc/engine_limits.h"

// children stays nullptr here — lazily allocated on first AddChild() call, since most widgets
// (e.g. leaf Labels) never have children and shouldn't pay for the array up front.
ME::UIElement::UIElement() {}

ME::UIElement::~UIElement() {
    delete[] children;
}

void ME::UIElement::Init() {}

void ME::UIElement::End() {}

void ME::UIElement::SetParent(UIElement* newParent) {
    parent = newParent;
    if (parent != nullptr) {
        parent->AddChild(this);
    }
}

ME::UIElement* ME::UIElement::GetParent() const {
    return parent;
}

bool ME::UIElement::AddChild(UIElement* child) {
    if (childCount >= Constants::MaxUIChildrenPerElement) {
        return false;
    }
    // Lazily allocated: most widgets (e.g. leaf Labels) never have children, so this array is
    // skipped entirely for them rather than reserved up front by every UIElement.
    if (children == nullptr) {
        children = new UIElement*[Constants::MaxUIChildrenPerElement];
    }
    children[childCount] = child;
    ++childCount;
    return true;
}

void ME::UIElement::RemoveChild(UIElement* child) {
    for (size_t i = 0; i < childCount; ++i) {
        if (children[i] == child) {
            children[i] = children[childCount - 1];
            --childCount;
            return;
        }
    }
}

ME::UIElement** ME::UIElement::GetChildren() const {
    return children;
}

size_t ME::UIElement::GetChildCount() const {
    return childCount;
}

void ME::UIElement::SetAnchor(UIAnchor newAnchor) {
    anchor = newAnchor;
}

ME::UIAnchor ME::UIElement::GetAnchor() const {
    return anchor;
}

void ME::UIElement::SetPivot(UIPivot newPivot) {
    pivot = newPivot;
}

ME::UIPivot ME::UIElement::GetPivot() const {
    return pivot;
}

void ME::UIElement::SetOffset(const Vec2& newOffset) {
    offset = newOffset;
}

ME::Vec2 ME::UIElement::GetOffset() const {
    return offset;
}

void ME::UIElement::SetSize(const Vec2& newSize) {
    size = newSize;
}

ME::Vec2 ME::UIElement::GetSize() const {
    return size;
}

const ME::UIRect* ME::UIElement::GetRect() const {
    return &rect;
}

void ME::UIElement::SetVisible(bool newVisible) {
    visible = newVisible;
}

bool ME::UIElement::IsVisible() const {
    return visible;
}

ME::SpriteRenderer* ME::UIElement::GetSpriteRenderer() {
    return nullptr;
}

ME::TextRenderer* ME::UIElement::GetTextRenderer() {
    return nullptr;
}

ME::Transform* ME::UIElement::GetTransform() {
    return nullptr;
}

ME::UIElementType ME::UIElement::GetType() const {
    return UIElementType::Base;
}
