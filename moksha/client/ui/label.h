#pragma once

/**
 * Label UI widget: text-only, wraps a TextRenderer.
 */

#include <cstdint>

#include "moksha/client/rendering/shared/color.h"
#include "moksha/client/rendering/shared/text_renderer.h"
#include "ui_element.h"

namespace ME {

class Transform;

class Label : public UIElement {
   public:
    Label() = delete;
    Label(const char* text, uint8_t quadId, uint8_t textureId, uint8_t materialId, const Color& color, uint16_t height,
          uint16_t width, int16_t letterSpacing, int16_t lineGap, uint16_t charsPerLine,
          TextAlignment alignment = TextAlignment::Left);
    ~Label();

    void Init() override;

    void SetText(const char* newText);
    void SetColor(const Color& newColor);

    TextRenderer* GetTextRenderer() override;
    Transform* GetTransform() override;
    UIElementType GetType() const override;

   private:
    char* initialText = nullptr;
    const uint8_t quadId;
    const uint8_t textureId;
    const uint8_t materialId;
    const uint16_t height;
    const uint16_t width;
    const int16_t letterSpacing;
    const int16_t lineGap;
    const uint16_t charsPerLine;
    const TextAlignment alignment;
    Color color;

    TextRenderer* textRenderer = nullptr;
    Transform* transform = nullptr;
};

}  // namespace ME
