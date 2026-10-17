/**
 * Light class definition.
 * This class represents a light source in a 3D scene.
 * It contains properties such as position, color, intensity, and type of light.
 */

#pragma once

#include "color.h"
#include "moksha/shared/math/vec3.h"

namespace ME {

enum class LightType {
    Ambient,
    Directional,
    Point,
    Spotlight,
};

struct LightDataAmbient {
    ME::Color color;
    float intensity;
    float padding[3];  // Padding to align the structure
};

struct LightDataDirectional {
    ME::Vec3 direction;
    float padding1;  // Padding to align the structure
    float intensity;
    float padding2[3];  // Padding to align the structure
    ME::Color color;
};

class Light {
   public:
    Light();
    ~Light();

    LightDataAmbient GetLightDataAmbient();
    LightDataDirectional GetLightDataDirectional();

    ME::Vec3 position;
    // Direction the light is pointing to. For directional and spotlight lights.
    ME::Vec3 direction;
    ME::Color color;
    float intensity;
    LightType type;
};

}  // namespace ME
