#include "sprite_renderer.h"

ME::SpriteRenderer::SpriteRenderer(uint8_t quadId, uint8_t materialId, uint8_t textureId, uint8_t textureAtlasPropsId,
                                   uint16_t atlasIndex, const Color& color, uint32_t flags)
    : quadId(quadId),
      materialId(materialId),
      textureId(textureId),
      textureAtlasPropsId(textureAtlasPropsId),
      atlasIndex(atlasIndex),
      color(color),
      flags(flags) {}

ME::SpriteRenderer::~SpriteRenderer() {}

void ME::SpriteRenderer::ToggleFlipHorizontal(bool bFlip) {
    bFlippedH = bFlip;

    if (bFlippedH) {
        flags |= 0x1u;  // Set bit 0
    } else {
        flags &= ~0x1u;  // Clear bit 0
    }
    bDirty = true;
}

void ME::SpriteRenderer::ToggleFlipVertical(bool bFlip) {
    bFlippedV = bFlip;

    if (bFlippedV) {
        flags |= 0x2u;  // Set bit 1
    } else {
        flags &= ~0x2u;  // Clear bit 1
    }
    bDirty = true;
}

bool ME::SpriteRenderer::GetFlipHorizontal() const {
    return bFlippedH;
}

bool ME::SpriteRenderer::GetFlipVertical() const {
    return bFlippedV;
}
