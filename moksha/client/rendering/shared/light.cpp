#include "light.h"

ME::Light::Light() {
    position = ME::Vec3(0.0f, 0.0f, 0.0f);
    direction = ME::Vec3(0.0f, 0.0f, 1.0f);
    color = ME::Color(1.0f, 1.0f, 1.0f);
    intensity = 1.0f;
    type = LightType::Directional;
}

ME::Light::~Light() {}

ME::LightDataAmbient ME::Light::GetLightDataAmbient() {
    return LightDataAmbient{
        color,
        intensity,
    };
}

ME::LightDataDirectional ME::Light::GetLightDataDirectional() {
    return LightDataDirectional{
        direction,
        0.0f,                         // Padding to align the structure
        intensity, 0.0f, 0.0f, 0.0f,  // Padding to align the structure
        color,
    };
}
