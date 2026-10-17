#include "panel.h"

#include "moksha/client/rendering/shared/sprite_renderer.h"
#include "moksha/shared/math/transform.h"

ME::Panel::Panel(uint8_t quadId, uint8_t materialId, uint8_t textureId, uint8_t textureAtlasPropsId,
                 uint16_t atlasIndex, const Color& color)
    : quadId(quadId),
      materialId(materialId),
      textureId(textureId),
      textureAtlasPropsId(textureAtlasPropsId),
      atlasIndex(atlasIndex),
      color(color) {}

ME::Panel::~Panel() {
    delete spriteRenderer;
    delete transform;
}

void ME::Panel::Init() {
    UIElement::Init();
    spriteRenderer = new SpriteRenderer(quadId, materialId, textureId, textureAtlasPropsId, atlasIndex, color);
    transform = new Transform();
}

void ME::Panel::SetColor(const Color& newColor) {
    color = newColor;
    if (spriteRenderer != nullptr) {
        spriteRenderer->color = newColor;
        spriteRenderer->bDirty = true;
    }
}

ME::SpriteRenderer* ME::Panel::GetSpriteRenderer() {
    return spriteRenderer;
}

ME::Transform* ME::Panel::GetTransform() {
    return transform;
}

ME::UIElementType ME::Panel::GetType() const {
    return UIElementType::Panel;
}
