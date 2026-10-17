#pragma once

/**
 * UI scene class implementation.
 * This will have ui related things like sprites and texts.
 */

#include "moksha/client/rendering/shared/camera.h"
#include "moksha/client/rendering/shared/sprite_renderer.h"
#include "moksha/client/rendering/shared/text_renderer.h"
#include "moksha/client/rendering/shared/texture.h"
#include "moksha/client/ui/ui_element.h"
#include "moksha/shared/datastructure/span.h"
#include "moksha/shared/physics/collider_aabb.h"

namespace ME {

class SceneUI {
   public:
    SceneUI();
    virtual ~SceneUI();

    /**
     * Updates the rendering related data in the UI scene, if any render element is marked as dirty.
     * This is called from Renderer.Update(), right before Renderer.Draw().
     * Like if sprite renderer's atlas index changed due to animation.
     * This is called every frame before rendering.
     * Everything that is not DX or MTL dependent is updated here, after which DX and MTL scene is updated from
     * Renderer.Update().
     */
    void Update();

   public:
    // Resources
    ME::Span<const char*> spriteTexturePaths;
    ME::Span<ME::TextureAtlasProperties> textureAtlasProperties;

    ME::Span<ME::Transform> uiSpriteTransforms;
    ME::Span<ME::SpriteRenderer> uiSpriteRenderers;
    ME::Span<ME::UISpriteRendererInstanceData> uiSpriteInstanceData;

    ME::Span<ME::Transform> textTransforms;
    ME::Span<ME::TextRenderer> textRenderers;
    ME::Span<ME::TextRendererInstanceData> textInstanceData;

    // Dumb registry of all UIElements (roots and children alike), populated/queried by
    // UISystem. Purely bookkeeping — no layout math, no rendering here.
    ME::Span<ME::UIElement*> uiElements;

    virtual void Init();
    virtual void CreateResources();
    virtual void BuildUISprites();
    virtual void BuildTextRenderers();

    /**
     * Override to construct this scene's dynamic UIElement tree (Panel/Label/Image/Container)
     * and register each root/child via AddUIElement. Called once from Init(), after
     * BuildTextRenderers() — mirrors that method's shape, but for the newer UIElement-based
     * path rather than the static sprite/text arrays.
     */
    virtual void BuildUIElements();

    // UIElement registry — false if AddUIElement is called at capacity (Constants::MaxUIElementCount).
    bool AddUIElement(ME::UIElement* element);
    void RemoveUIElement(ME::UIElement* element);
    ME::UIElement** GetUIElements() const;
    uint32_t GetUIElementCount() const;

    /**
     * Overwrites the live UI sprite/text arrays with externally-owned pointers (no allocation).
     * Existing entries beyond `count` are simply not iterated by Update() — no need to clear them.
     * Used by UISystem's per-frame rebuild of the dynamic UIElement tree; separate from the
     * additive AddUISprite/AddUIText path used by static Build*() overrides.
     */
    void RebuildUISprites(ME::Transform** transforms, ME::SpriteRenderer** renderers, uint32_t count);
    void RebuildUIText(ME::Transform** transforms, ME::TextRenderer** renderers, uint32_t count);

   protected:
    // Helper functions to add elements to the UI scene.

    /**
     * Creates and adds a UI sprite with the given position and scale.
     */
    void AddUISprite(ME::Vec3 position, ME::Vec3 scale, const ME::SpriteRenderer& spriteRenderer);

    /**
     * Creates and adds a Text renderer with the given parameters.
     */
    void AddUIText(ME::Vec3 position, ME::Vec3 scale, const ME::TextRenderer& textRenderer);

   private:
    /**
     * Updates UI sprite renderers that are marked as dirty.
     * After update, the dirty flag is cleared.
     */
    void UpdateUISpriteRenderers();

    /**
     * Updates text renderers that are marked as dirty.
     * After update, the dirty flag is cleared.
     */
    void UpdateTextRenderers();
};

}  // namespace ME
