/**
 * Texture sampler definition for shaders.
 */

#pragma once

#include <cstdint>

namespace ME {

// Enum class for texture filtering modes for shaders.
enum class TextureFilter : uint8_t {
    Nearest = 0,
    Linear = 1
};

// Enum class for texture wrapping modes for shaders.
enum class TextureWrap : uint8_t {
    Repeat = 0,
    ClampToEdge = 1,
    MirrorClampToEdge = 2,
    MirrorRepeat = 3,
    ClampToZero = 4,
    ClampToBorderColor = 5
};

// Enum class for texture border color for shaders.
enum class TextureBorderColor : uint8_t {
    TransparentBlack = 0,
    OpaqueBlack = 1,
    OpaqueWhite = 2
};

class TextureSampler {
   public:
    TextureSampler() = default;
    TextureSampler(TextureFilter filter, TextureWrap wrap) : filter(filter), wrap(wrap) {}
    ~TextureSampler() = default;

    TextureFilter filter = TextureFilter::Nearest;
    TextureWrap wrap = TextureWrap::Repeat;
};

}  // namespace ME
