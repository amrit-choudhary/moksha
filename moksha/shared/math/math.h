/**
 * The base header for all common math libraries.
 * Also used as common header to incldue for getting math libraries.
 */

#pragma once

// Including the math libraries. So users will need to include this one header.
#include "fp_24_8.h"
#include "matrix4.h"
#include "vec3.h"
#include "vec3i.h"
#include "vec4.h"

namespace ME {

using FP = ME::FP_24_8;

// Math constants
inline constexpr double PI = 3.14159f;
inline constexpr double TAU = 2.0f * PI;
inline constexpr double HALF_PI = PI * 0.5f;
inline constexpr double QUARTER_PI = PI * 0.25f;
inline constexpr double DEG2RAD = PI / 180.0f;
inline constexpr double RAD2DEG = 180.0f / PI;

inline constexpr uint32_t POW2(uint32_t x) {
    return 1u << x;
}

// Test add function.
template <typename T>
T add(T a, T b) {
    return a + b;
}

/**
 * Math class provides common math functions.
 */
class Math {
   public:
    // Clamps a value between min and max.
    static inline float Clamp(float value, float min, float max) {
        if (value < min) {
            return min;
        } else if (value > max) {
            return max;
        }
        return value;
    }

    // Minimum of two values.
    static inline float Min(float a, float b) {
        return (a < b) ? a : b;
    }

    // Maximum of two values.
    static inline float Max(float a, float b) {
        return (a > b) ? a : b;
    }
};

}  // namespace ME
