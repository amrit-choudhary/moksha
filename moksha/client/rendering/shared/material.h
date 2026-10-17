/**
 * Material class definition.
 * This class represents a material in a 3D rendering engine.
 * It contains properties such as color, texture, and shader information.
 */

#pragma once

#include <cstdint>

#include "color.h"
#include "moksha/shared/math/vec3.h"

namespace ME {

// Enum class for depth compare functions
enum class DepthCompareFunction : uint8_t {
    Never,
    Less,
    Equal,
    LessEqual,
    Greater,
    NotEqual,
    GreaterEqual,
    Always
};

class Material {
   public:
    Material() = delete;
    ~Material();

    Material(const char* texturePath, const char* shaderPath, const Color& color = Color::White(),
             DepthCompareFunction depthCompareFunction = DepthCompareFunction::Less, bool depthWriteEnabled = true)
        : texturePath(texturePath),
          shaderPath(shaderPath),
          color(color),
          depthCompareFunction(depthCompareFunction),
          depthWriteEnabled(depthWriteEnabled) {}

    const char* texturePath;
    const char* shaderPath;
    ME::Color color;
    ME::DepthCompareFunction depthCompareFunction;
    bool depthWriteEnabled;
};

}  // namespace ME
