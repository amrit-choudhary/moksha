#pragma once

/**
 * Integer Vector2 class. Uses int32_t for components.
 */

#include <cmath>
#include <cstdint>

#include "vec2.h"

namespace ME {

class Vec2i {
   public:
    int32_t x;
    int32_t y;

    // Vec2i{0, 0}.
    static const Vec2i Zero;
    // Vec2i{1, 1}.
    static const Vec2i One;
    // Vec2i{0, 1}.
    static const Vec2i Up;
    // Vec2i{0, -1}.
    static const Vec2i Down;
    // Vec2i{1, 0}.
    static const Vec2i Right;
    // Vec2i{-1, 0}.
    static const Vec2i Left;

    // Get length (returns a float since length is not necessarily an integer).
    inline float Length() const;
    // Returns a new vector that is normalized (returns a float vector).
    inline Vec2 Normalised() const;
    // Returns a new vector that is normalized (returns an integer vector).
    inline Vec2i NormalisedInt() const;

    // Math functions
    static inline int32_t Dot(const Vec2i& a, const Vec2i& b);
    static inline Vec2i Cross(const Vec2i& a, const Vec2i& b);

   private:
   protected:
};

inline float ME::Vec2i::Length() const {
    return std::sqrt(static_cast<float>(x * x + y * y));
}

inline Vec2 ME::Vec2i::Normalised() const {
    float len = Length();
    if (len > 0) {
        return Vec2(static_cast<float>(x) / len, static_cast<float>(y) / len);
    } else {
        return Vec2::Zero;
    }
}

inline Vec2i ME::Vec2i::NormalisedInt() const {
    float len = Length();
    if (len > 0) {
        return Vec2i(static_cast<int32_t>(x / len), static_cast<int32_t>(y / len));
    } else {
        return Vec2i::Zero;
    }
}

inline int32_t Vec2i::Dot(const Vec2i& a, const Vec2i& b) {
    return a.x * b.x + a.y * b.y;
}

inline Vec2i Vec2i::Cross(const Vec2i& a, const Vec2i& b) {
    return Vec2i(a.x * b.y - a.y * b.x);
}

// Operator overloads.

inline Vec2i operator+(const Vec2i& a, const Vec2i& b) {
    return Vec2i(a.x + b.x, a.y + b.y);
};

inline Vec2i operator-(const Vec2i& a, const Vec2i& b) {
    return Vec2i(a.x - b.x, a.y - b.y);
};

inline Vec2i operator*(const Vec2i& a, const Vec2i& b) {
    return Vec2i(a.x * b.x, a.y * b.y);
};
inline Vec2i operator/(const Vec2i& a, const Vec2i& b) {
    return Vec2i(a.x / b.x, a.y / b.y);
};

inline bool operator==(const Vec2i& a, const Vec2i& b) {
    return (a.x == b.x) && (a.y == b.y);
};
inline bool operator!=(const Vec2i& a, const Vec2i& b) {
    return !(a == b);
};

}  // namespace ME
