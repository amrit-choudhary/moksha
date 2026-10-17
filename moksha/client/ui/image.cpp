#include "image.h"

#include "moksha/client/rendering/shared/sprite_renderer.h"
#include "moksha/shared/math/transform.h"

ME::Image::Image(uint8_t quadId, uint8_t materialId, uint8_t textureId, uint8_t textureAtlasPropsId,
                 uint16_t atlasIndex, const Color& color)
    : quadId(quadId),
      materialId(materialId),
      textureId(textureId),
      textureAtlasPropsId(textureAtlasPropsId),
      atlasIndex(atlasIndex),
      color(color) {}

ME::Image::~Image() {
    delete spriteRenderer;
    delete transform;
}

void ME::Image::Init() {
    UIElement::Init();
    spriteRenderer = new SpriteRenderer(quadId, materialId, textureId, textureAtlasPropsId, atlasIndex, color);
    transform = new Transform();
}

ME::SpriteRenderer* ME::Image::GetSpriteRenderer() {
    return spriteRenderer;
}

ME::Transform* ME::Image::GetTransform() {
    return transform;
}

ME::UIElementType ME::Image::GetType() const {
    return UIElementType::Image;
}
