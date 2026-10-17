/**
 * This is a simple sprite renderer class.
 * It is used to render 2D sprites in the game.
 */

#pragma once

#include <cstdint>

#include "color.h"
#include "moksha/client/anim/sprite_animator.h"
#include "moksha/shared/math/transform.h"
#include "moksha/shared/math/vec16.h"

namespace ME {

/**
 * Data to hold the sprite renderer instance data.
 * Used for instanced rendering.
 * This structure is sent to the GPU as a constant buffer, so some hlsl padding rules apply.
 * Flags Rules bit wise:
 * 0: Flip the sprite horizontally.
 * 1: Flip the sprite vertically.
 * 2: Flip the sprite diagonally.
 */
class SpriteRendererInstanceData {
   public:
    ME::Vec16 modelMatrixData = ME::Vec16();
    ME::Color color = ME::Color::White();
    uint32_t atlasIndex = 0;  // Index of the texture atlas
    uint32_t flags = 0;       // Additional flags for rendering options
};

/**
 * Data to hold the UI sprite renderer instance data.
 * Used for instanced rendering.
 * This structure is sent to the GPU as a constant buffer, so some hlsl padding rules apply. (4)
 * Flags Rules bit wise:
 * 0: Flip the sprite horizontally.
 * 1: Flip the sprite vertically.
 * 2: Flip the sprite diagonally.
 */
class UISpriteRendererInstanceData {
   public:
    ME::Vec16 modelMatrixData = ME::Vec16();
    ME::Color color = ME::Color::White();
    uint32_t atlasIndex = 0;  // Index of the texture atlas
    uint32_t flags = 0;       // Additional flags for rendering options
};

/**
 * Constant buffer structure for sprite renderer.
 * Used for non-instanced sprite rendering. Per sprite renderer.
 * This is sent to the GPU as a constant buffer, so some hlsl padding rules apply. (256)
 * For flag rules see SpriteRendererInstanceData.
 */
class CBPerSprite {
   public:
    ME::Vec16 modelMatrix = ME::Vec16();
    ME::Color color = ME::Color::White();
    uint32_t atlasIndex = 0;
    uint32_t flags = 0;      // Additional flags for rendering options
    uint32_t padding[2]{0};  // Padding to align the structure
};

class SpriteRenderer {
   public:
    SpriteRenderer() = default;
    SpriteRenderer(uint8_t quadId, uint8_t materialId, uint8_t textureId, uint8_t textureAtlasPropsId,
                   uint16_t atlasIndex, const Color& color = ME::Color::White(), uint32_t flags = 0);
    ~SpriteRenderer();

    /**
     * Flip the sprite horizontally.
     * @param bFlip Whether to flip the sprite horizontally.
     */
    void ToggleFlipHorizontal(bool bFlip);
    /**
     * Flip the sprite vertically.
     * @param bFlip Whether to flip the sprite vertically.
     */
    void ToggleFlipVertical(bool bFlip);

    // Get whether the sprite is flipped horizontally.
    bool GetFlipHorizontal() const;

    // Get whether the sprite is flipped vertically.
    bool GetFlipVertical() const;

    // Mark the sprite renderer as dirty to update rendering data before frame is drawn.
    bool bDirty = true;

    uint8_t quadId = 0;
    uint8_t textureId = 0;
    uint8_t textureAtlasPropsId = 0;
    uint16_t atlasIndex = 0;
    uint8_t materialId = 0;
    ME::Color color = ME::Color::White();
    uint32_t flags = 0;

    ME::SpriteAnimator* animator = nullptr;

   private:
    bool bFlippedH = false;
    bool bFlippedV = false;
};

}  // namespace ME
