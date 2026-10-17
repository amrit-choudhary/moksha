#include "label.h"

#include <cstring>

#include "moksha/client/rendering/shared/text_renderer.h"
#include "moksha/shared/math/transform.h"

ME::Label::Label(const char* text, uint8_t quadId, uint8_t textureId, uint8_t materialId, const Color& color,
                 uint16_t height, uint16_t width, int16_t letterSpacing, int16_t lineGap, uint16_t charsPerLine,
                 TextAlignment alignment)
    : quadId(quadId),
      textureId(textureId),
      materialId(materialId),
      height(height),
      width(width),
      letterSpacing(letterSpacing),
      lineGap(lineGap),
      charsPerLine(charsPerLine),
      alignment(alignment),
      color(color) {
    size_t length = strlen(text) + 1;
    initialText = new char[length];
    strcpy(initialText, text);
}

ME::Label::~Label() {
    delete[] initialText;
    delete textRenderer;
    delete transform;
}

void ME::Label::Init() {
    UIElement::Init();
    textRenderer = new TextRenderer(initialText, quadId, textureId, materialId, color, height, width, letterSpacing,
                                    lineGap, charsPerLine, alignment);
    transform = new Transform();

    // TextRenderer's constructor already copies the string internally — no need to hold our
    // own copy for the widget's whole lifetime.
    delete[] initialText;
    initialText = nullptr;
}

void ME::Label::SetText(const char* newText) {
    if (textRenderer != nullptr) {
        textRenderer->SetText(newText);
    }
}

void ME::Label::SetColor(const Color& newColor) {
    color = newColor;
    if (textRenderer != nullptr) {
        textRenderer->color = newColor;
        textRenderer->bDirty = true;
    }
}

ME::TextRenderer* ME::Label::GetTextRenderer() {
    return textRenderer;
}

ME::Transform* ME::Label::GetTransform() {
    return transform;
}

ME::UIElementType ME::Label::GetType() const {
    return UIElementType::Label;
}
