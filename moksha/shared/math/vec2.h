/**
 * Vector2 class with X and Y.
 * Coordinate Convention:
 * Looking at the screen:
 * X goes from left to right of the screen.
 * Y goes from bottom to top of the screen.
 */

#pragma once

namespace ME {

/**
 * Float Vector2 class.
 */
class Vec2 {
   public:
    float x;
    float y;

    // Vec2{0, 0}.
    static const Vec2 Zero;
    // Vec2{1, 1}.
    static const Vec2 One;
    // Vec2{0, 1}.
    static const Vec2 Up;
    // Vec2{0, -1}.
    static const Vec2 Down;
    // Vec2{1, 0}.
    static const Vec2 Right;
    // Vec2{-1, 0}.
    static const Vec2 Left;

    // Get length.
    float Length();
    // In place normalise this vector.
    void Normalise();
    // In place normalise this vector. Will also check for 0 length.
    void NormaliseSafe();
    // Returns a new vector that is normalised.
    Vec2 Normalised();

    // Operator overloads.
    Vec2 operator-() const;
    Vec2& operator+=(const Vec2& b);
    Vec2& operator-=(const Vec2& b);
    Vec2& operator*=(const Vec2& b);
    Vec2& operator/=(const Vec2& b);

    // Math functions
    static float Dot(const Vec2& a, const Vec2& b);

   private:
   protected:
};

// Operator overloads.
Vec2 operator+(const Vec2& a, const Vec2& b);
Vec2 operator-(const Vec2& a, const Vec2& b);
Vec2 operator*(const Vec2& a, const Vec2& b);
Vec2 operator/(const Vec2& a, const Vec2& b);

bool operator==(const Vec2& a, const Vec2& b);
bool operator!=(const Vec2& a, const Vec2& b);

Vec2 operator*(const Vec2& a, float b);
Vec2 operator*(float a, const Vec2& b);

////////////////////////////////////////
// Implementation of inline functions.
////////////////////////////////////////

inline Vec2 operator*(const Vec2& a, float b) {
    return Vec2{a.x * b, a.y * b};
}

inline Vec2 operator*(float a, const Vec2& b) {
    return Vec2{a * b.x, a * b.y};
}

}  // namespace ME
