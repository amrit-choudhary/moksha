#pragma once

/**
 * Image UI widget: a single sprite, kept distinct from Panel per the widget set even though it's
 * near-identical today (they may diverge later, e.g. Image getting sub-rect/UV support).
 */

#include <cstdint>

#include "moksha/client/rendering/shared/color.h"
#include "ui_element.h"

namespace ME {

class SpriteRenderer;
class Transform;

class Image : public UIElement {
   public:
    Image() = delete;
    Image(uint8_t quadId, uint8_t materialId, uint8_t textureId, uint8_t textureAtlasPropsId, uint16_t atlasIndex,
          const Color& color = ME::Color::White());
    ~Image();

    void Init() override;

    SpriteRenderer* GetSpriteRenderer() override;
    Transform* GetTransform() override;
    UIElementType GetType() const override;

   private:
    const uint8_t quadId;
    const uint8_t materialId;
    const uint8_t textureId;
    const uint8_t textureAtlasPropsId;
    const uint16_t atlasIndex;
    const Color color;

    SpriteRenderer* spriteRenderer = nullptr;
    Transform* transform = nullptr;
};

}  // namespace ME
