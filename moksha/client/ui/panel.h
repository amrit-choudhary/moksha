#pragma once

/**
 * Panel UI widget: a background/container sprite with no text.
 */

#include <cstdint>

#include "moksha/client/rendering/shared/color.h"
#include "ui_element.h"

namespace ME {

class SpriteRenderer;
class Transform;

class Panel : public UIElement {
   public:
    Panel() = delete;
    Panel(uint8_t quadId, uint8_t materialId, uint8_t textureId, uint8_t textureAtlasPropsId, uint16_t atlasIndex,
          const Color& color = ME::Color::White());
    ~Panel();

    void Init() override;

    void SetColor(const Color& newColor);

    SpriteRenderer* GetSpriteRenderer() override;
    Transform* GetTransform() override;
    UIElementType GetType() const override;

   private:
    const uint8_t quadId;
    const uint8_t materialId;
    const uint8_t textureId;
    const uint8_t textureAtlasPropsId;
    const uint16_t atlasIndex;
    Color color;

    SpriteRenderer* spriteRenderer = nullptr;
    Transform* transform = nullptr;
};

}  // namespace ME
