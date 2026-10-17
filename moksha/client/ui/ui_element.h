#pragma once

/**
 * Base class for all UI widgets.
 */

#include <cstddef>
#include <cstdint>

#include "moksha/shared/math/vec2.h"
#include "ui_rect.h"

namespace ME {

class SpriteRenderer;
class TextRenderer;
class Transform;

/**
 * Identifies a UIElement's concrete widget type.
 */
enum class UIElementType : uint8_t {
    Base = 0,
    Container = 1,
    Panel = 2,
    Label = 3,
    Image = 4,
    Button = 5,
};

/**
 * Base class for all UI widgets. Deliberately concrete, not abstract: Init()/End() default to
 * no-ops and GetSpriteRenderer()/GetTextRenderer() default to nullptr, so a plain UIElement (or
 * Container, see container.h) is a layout-only grouping node with no owned visual.
 */
class UIElement {
   public:
    UIElement();
    virtual ~UIElement();

    // Only UILayoutEngine may obtain a mutable UIRect* (to call ComputeAbsolute()) — see the
    // const GetRect() below for what everyone else gets.
    friend class UILayoutEngine;

    virtual void Init();
    virtual void End();

    void SetParent(UIElement* newParent);
    UIElement* GetParent() const;

    // Returns false if the children array is already full (sentinel-style, no exceptions).
    bool AddChild(UIElement* child);
    void RemoveChild(UIElement* child);
    UIElement** GetChildren() const;
    size_t GetChildCount() const;

    void SetAnchor(UIAnchor newAnchor);
    UIAnchor GetAnchor() const;
    void SetPivot(UIPivot newPivot);
    UIPivot GetPivot() const;
    void SetOffset(const Vec2& newOffset);
    Vec2 GetOffset() const;
    void SetSize(const Vec2& newSize);
    Vec2 GetSize() const;

    // Read-only for everyone except UILayoutEngine (see the friend declaration above) — nothing
    // outside the layout engine can reach a mutable UIRect* to write an absolute position
    // directly.
    const UIRect* GetRect() const;

    void SetVisible(bool newVisible);
    bool IsVisible() const;

    // Render-sync seam: base returns nullptr for both. Panel/Image override GetSpriteRenderer(),
    // Label overrides GetTextRenderer(). This lets UISystem walk a mixed tree and pull out
    // whichever visual each concrete widget owns, without dynamic_cast or a type tag.
    virtual SpriteRenderer* GetSpriteRenderer();
    virtual TextRenderer* GetTextRenderer();
    virtual Transform* GetTransform();

    virtual UIElementType GetType() const;

   protected:
    UIRect rect;
    UIAnchor anchor = UIAnchor::TopLeft;
    UIPivot pivot = UIPivot::TopLeft;
    Vec2 offset = Vec2::Zero;
    Vec2 size = Vec2::Zero;

    UIElement* parent = nullptr;
    UIElement** children = nullptr;
    size_t childCount = 0;

    bool visible = true;
};

}  // namespace ME
