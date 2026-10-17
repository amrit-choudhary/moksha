/**
 * Vector4 class with X, Y, Z, and W.
 * Coordinate Convention:
 * Looking at the screen:
 * X goes from left to right of the screen.
 * Y goes from bottom to top of the screen.
 * Z goes from viewer towards the screen.
 * W is the fourth dimension, often used for homogeneous coordinates.
 */

#pragma once

namespace ME {

/**
 * Float Vector4 class.
 */
class Vec4 {
   public:
    float x;
    float y;
    float z;
    float w;

    // Vec4{0, 0, 0, 0}.
    static const Vec4 Zero;
    // Vec4{1, 1, 1, 1}.
    static const Vec4 One;

    // Get length.
    float Length();
    // In place normalise this vector.
    void Normalise();
    // In place normalise this vector. Will also check for 0 length.
    void NormaliseSafe();
    // Returns a new vector that is normalised.
    Vec4 Normalised();

    // Operator overloads.
    Vec4 operator-() const;
    Vec4& operator+=(const Vec4& b);
    Vec4& operator-=(const Vec4& b);
    Vec4& operator*=(const Vec4& b);
    Vec4& operator/=(const Vec4& b);

    // Math functions
    static float Dot(const Vec4& a, const Vec4& b);
    static Vec4 Cross(const Vec4& a, const Vec4& b);

   private:
   protected:
};

// Operator overloads.
Vec4 operator+(const Vec4& a, const Vec4& b);
Vec4 operator-(const Vec4& a, const Vec4& b);
Vec4 operator*(const Vec4& a, const Vec4& b);
Vec4 operator/(const Vec4& a, const Vec4& b);

bool operator==(const Vec4& a, const Vec4& b);
bool operator!=(const Vec4& a, const Vec4& b);

// Math functions

}  // namespace ME