/**
 * Constant buffer layouts the sprite and UI shaders read, shared by every graphics API (hlsl and metal mirror them
 * in their structs). Per-sprite and instance data live with their renderers (sprite_renderer.h, text_renderer.h).
 */
#pragma once

#include <cstdint>

#include "light.h"
#include "moksha/shared/math/vec16.h"

namespace ME {

/** Constant buffer structure for shaders. */
class CBPerPass {
   public:
    ME::Vec16 viewMatrix;
    ME::Vec16 projectionMatrix;
    ME::LightDataAmbient ambientLightData;
    ME::LightDataDirectional directionalLightData;
};

/** Constant buffer structure for UI Text shaders. */
class CBPerPassUIText {
   public:
    // Packed screen dimension: high 16 bits = width, low 16 bits = height.
    uint32_t screenDimension;
};

/** Constant buffer structure for UI Sprite shaders. */
class CBPerPassUISprite {
   public:
    // Packed screen dimension: high 16 bits = width, low 16 bits = height.
    uint32_t screenDimension;
};

/** Constant buffer structure for shaders. */
class CBPerObject {
   public:
    ME::Vec16 modelMatrix;
};

}  // namespace ME
