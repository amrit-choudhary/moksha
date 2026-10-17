#include "ui_rect.h"

ME::UIRect::UIRect() {}

ME::UIRect::~UIRect() {}

ME::Vec2 ME::UIRect::GetPosition() const {
    return position;
}

ME::Vec2 ME::UIRect::GetSize() const {
    return size;
}

bool ME::UIRect::Contains(const Vec2& point) const {
    return point.x >= position.x && point.x <= position.x + size.x && point.y >= position.y &&
           point.y <= position.y + size.y;
}

void ME::UIRect::ComputeAbsolute(const UIRect& parentRect, UIAnchor anchor, UIPivot pivot, const Vec2& offset,
                                 const Vec2& ownSize) {
    // See the ComputeAbsolute doc comment in ui_rect.h: anchorPoint (step 1) and pivotCorrection
    // (step 2) are resolved independently, from `anchor` and `pivot` respectively.
    Vec2 parentPosition = parentRect.GetPosition();
    Vec2 parentSize = parentRect.GetSize();

    Vec2 anchorPoint = parentPosition;
    switch (anchor) {
        case UIAnchor::TopLeft:
            anchorPoint = parentPosition;
            break;
        case UIAnchor::TopCenter:
            anchorPoint = Vec2{parentPosition.x + parentSize.x / 2.0f, parentPosition.y};
            break;
        case UIAnchor::TopRight:
            anchorPoint = Vec2{parentPosition.x + parentSize.x, parentPosition.y};
            break;
        case UIAnchor::CenterLeft:
            anchorPoint = Vec2{parentPosition.x, parentPosition.y + parentSize.y / 2.0f};
            break;
        case UIAnchor::Center:
            anchorPoint = Vec2{parentPosition.x + parentSize.x / 2.0f, parentPosition.y + parentSize.y / 2.0f};
            break;
        case UIAnchor::CenterRight:
            anchorPoint = Vec2{parentPosition.x + parentSize.x, parentPosition.y + parentSize.y / 2.0f};
            break;
        case UIAnchor::BottomLeft:
            anchorPoint = Vec2{parentPosition.x, parentPosition.y + parentSize.y};
            break;
        case UIAnchor::BottomCenter:
            anchorPoint = Vec2{parentPosition.x + parentSize.x / 2.0f, parentPosition.y + parentSize.y};
            break;
        case UIAnchor::BottomRight:
            anchorPoint = Vec2{parentPosition.x + parentSize.x, parentPosition.y + parentSize.y};
            break;
    }

    Vec2 pivotCorrection = Vec2::Zero;
    switch (pivot) {
        case UIPivot::TopLeft:
            pivotCorrection = Vec2::Zero;
            break;
        case UIPivot::TopCenter:
            pivotCorrection = Vec2{ownSize.x / 2.0f, 0.0f};
            break;
        case UIPivot::TopRight:
            pivotCorrection = Vec2{ownSize.x, 0.0f};
            break;
        case UIPivot::CenterLeft:
            pivotCorrection = Vec2{0.0f, ownSize.y / 2.0f};
            break;
        case UIPivot::Center:
            pivotCorrection = Vec2{ownSize.x / 2.0f, ownSize.y / 2.0f};
            break;
        case UIPivot::CenterRight:
            pivotCorrection = Vec2{ownSize.x, ownSize.y / 2.0f};
            break;
        case UIPivot::BottomLeft:
            pivotCorrection = Vec2{0.0f, ownSize.y};
            break;
        case UIPivot::BottomCenter:
            pivotCorrection = Vec2{ownSize.x / 2.0f, ownSize.y};
            break;
        case UIPivot::BottomRight:
            pivotCorrection = Vec2{ownSize.x, ownSize.y};
            break;
    }

    position = anchorPoint - pivotCorrection + offset;
    size = ownSize;
}
