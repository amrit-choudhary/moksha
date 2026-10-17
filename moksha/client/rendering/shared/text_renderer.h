/**
 * Used to render text in the game.
 * This class handles the rendering of text using a font atlas.
 * It uses a sprite renderer to render the text as sprites.
 * Can render ASCII characters using bitmap font map.
 */

#pragma once

#include <cstdint>

#include "color.h"
#include "moksha/shared/math/vec16.h"

namespace ME {

/**
 * Horizontal alignment of a text run within its owning widget's box (see UIElement::SetSize).
 * Anchor/pivot control where the box itself sits; this controls where the text sits within it.
 *
 * TODO: vertical alignment (Top/Middle/Bottom)
 */
enum class TextAlignment : uint8_t {
    Left = 0,
    Center = 1,
    Right = 2,
};

/**
 * Data to hold the text renderer instance data.
 * Used for instanced rendering.
 */
class TextRendererInstanceData {
   public:
    ME::Vec16 modelMatrixData = ME::Vec16();
    ME::Color color = ME::Color::White();
    uint32_t atlasIndex = 0;  // Index of the texture atlas
    uint32_t flags = 0;
};

class TextRenderer {
   public:
    // Default-constructs an empty (untextured) renderer — needed so TextRenderer can live in a
    // value array (e.g. Span<TextRenderer>). Slots beyond a Span's `count` are never read, so
    // an unset `text` is never dereferenced.
    TextRenderer();
    TextRenderer(const char* text, uint8_t quadId, uint8_t textureId, uint8_t materialId, const ME::Color& color,
                 uint16_t height, uint16_t width, int16_t letterSpacing, int16_t lineGap, uint16_t charsPerLine,
                 TextAlignment alignment = TextAlignment::Left);
    // Deep-copies `text` so each instance owns its own buffer.
    TextRenderer(const TextRenderer& other);
    TextRenderer& operator=(const TextRenderer& other);
    ~TextRenderer();

    // Marks the text renderer as dirty, indicating that its data has changed and needs to be updated.
    bool bDirty = true;

    char* text = nullptr;                           // Text to render
    uint8_t quadId = 0;                             // ID of the quad to render
    uint8_t textureId = 0;                          // ID of the texture to use
    uint8_t materialId = 0;                         // ID of the material to use
    ME::Color color = ME::Color::White();           // Color of the text
    uint16_t height = 12;                           // Height of the text in pixels
    uint16_t width = 12;                            // Width of the text in pixels
    int16_t letterSpacing = 2;                      // Spacing between letters in pixels
    int16_t lineGap = 2;                            // Gap between lines in pixels
    uint16_t charsPerLine = 80;                     // Number of characters per line
    TextAlignment alignment = TextAlignment::Left;  // Horizontal alignment within the owning box

    // Sets new text to render and marks the renderer as dirty.
    void SetText(const char* newText);

    // Returns the number of characters in the text.
    uint16_t GetCount() const;

    // Returns the width of the rendered text based on the width and letter spacing.
    uint16_t GetRenderWidth() const;

    // Returns the height of the rendered text.
    uint16_t GetRenderHeight() const;
};

}  // namespace ME
