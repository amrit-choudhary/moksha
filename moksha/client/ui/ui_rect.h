#pragma once

/**
 * UIRect: a UI widget's resolved position + size, in top-left-origin, Y-down UI space (not the
 * engine's usual center-origin/Y-up space used elsewhere).
 */

#include <cstdint>

#include "moksha/shared/math/vec2.h"

namespace ME {

/**
 * 9-point anchor within a parent's rect.
 */
enum class UIAnchor : uint8_t {
    TopLeft = 0,
    TopCenter = 1,
    TopRight = 2,
    CenterLeft = 3,
    Center = 4,
    CenterRight = 5,
    BottomLeft = 6,
    BottomCenter = 7,
    BottomRight = 8,
};

/**
 * 9-point pivot on the widget itself — which of its own points gets placed at the anchor point.
 * Independent of UIAnchor (same value set, distinct type): a widget can anchor to one corner of
 * its parent while pivoting from a different point on itself.
 */
enum class UIPivot : uint8_t {
    TopLeft = 0,
    TopCenter = 1,
    TopRight = 2,
    CenterLeft = 3,
    Center = 4,
    CenterRight = 5,
    BottomLeft = 6,
    BottomCenter = 7,
    BottomRight = 8,
};

/**
 * UIRect: a UI widget's resolved position + size, in top-left-origin, Y-down UI space (not the
 * engine's usual center-origin/Y-up space used elsewhere).
 */
class UIRect {
   public:
    UIRect();
    ~UIRect();

    // Top-left corner of the rect, regardless of which anchor produced it.
    Vec2 GetPosition() const;
    // Resolved width/height — just `ownSize` as passed to ComputeAbsolute (no stretch/fill yet).
    Vec2 GetSize() const;

    // Point-in-rect test, in the same top-left-origin, Y-down UI space as GetPosition()/GetSize().
    bool Contains(const Vec2& point) const;

    /**
     * Compute this rect's absolute position and size, given the parent's rect, the anchor, the
     * pivot, the offset, and the ownSize.
     *
     * Resolves in two independent steps:
     *  1. anchorPoint: a point within the parent's rect, chosen by `anchor` (e.g. Center =
     *     parent's midpoint, BottomRight = parent's bottom-right corner).
     *  2. pivotCorrection: an ownSize-based offset, chosen by `pivot`, subtracted from
     *     anchorPoint so the corresponding point on THIS rect (not always its top-left) lands
     *     on anchorPoint — e.g. pivot BottomRight means this rect's own bottom-right corner is
     *     what gets placed at anchorPoint, rather than its top-left.
     * offset is then added as a plain (+x right, +y down) pixel nudge from that resolved point.
     */
    void ComputeAbsolute(const UIRect& parentRect, UIAnchor anchor, UIPivot pivot, const Vec2& offset,
                         const Vec2& ownSize);

   private:
    Vec2 position = Vec2::Zero;
    Vec2 size = Vec2::Zero;
};

}  // namespace ME
