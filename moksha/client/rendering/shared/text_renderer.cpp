#include "text_renderer.h"

#include <cstring>

ME::TextRenderer::TextRenderer() {}

ME::TextRenderer::TextRenderer(const char* text, uint8_t quadId, uint8_t textureId, uint8_t materialId,
                               const ME::Color& color, uint16_t height, uint16_t width, int16_t letterSpacing,
                               int16_t lineGap, uint16_t charsPerLine, TextAlignment alignment)
    : quadId(quadId), textureId(textureId), materialId(materialId) {
    size_t length = strlen(text) + 1;
    this->text = new char[length];
    strcpy(this->text, text);

    this->color = color;
    this->height = height;
    this->width = width;
    this->letterSpacing = letterSpacing;
    this->lineGap = lineGap;
    this->charsPerLine = charsPerLine;
    this->alignment = alignment;
}

ME::TextRenderer::TextRenderer(const TextRenderer& other)
    : bDirty(other.bDirty),
      quadId(other.quadId),
      textureId(other.textureId),
      materialId(other.materialId),
      color(other.color),
      height(other.height),
      width(other.width),
      letterSpacing(other.letterSpacing),
      lineGap(other.lineGap),
      charsPerLine(other.charsPerLine),
      alignment(other.alignment) {
    if (other.text != nullptr) {
        size_t length = strlen(other.text) + 1;
        text = new char[length];
        strcpy(text, other.text);
    }
}

ME::TextRenderer& ME::TextRenderer::operator=(const TextRenderer& other) {
    if (this == &other) {
        return *this;
    }

    delete[] text;
    text = nullptr;
    if (other.text != nullptr) {
        size_t length = strlen(other.text) + 1;
        text = new char[length];
        strcpy(text, other.text);
    }

    bDirty = other.bDirty;
    quadId = other.quadId;
    textureId = other.textureId;
    materialId = other.materialId;
    color = other.color;
    height = other.height;
    width = other.width;
    letterSpacing = other.letterSpacing;
    lineGap = other.lineGap;
    charsPerLine = other.charsPerLine;
    alignment = other.alignment;
    return *this;
}

ME::TextRenderer::~TextRenderer() {
    delete[] text;
    text = nullptr;
}

void ME::TextRenderer::SetText(const char* newText) {
    delete[] text;
    text = nullptr;

    size_t length = strlen(newText) + 1;
    text = new char[length];
    strcpy(text, newText);

    bDirty = true;
}

uint16_t ME::TextRenderer::GetCount() const {
    return strlen(text);
}

uint16_t ME::TextRenderer::GetRenderWidth() const {
    return width * GetCount() + letterSpacing * (GetCount() - 1);
}

uint16_t ME::TextRenderer::GetRenderHeight() const {
    return height;
}
