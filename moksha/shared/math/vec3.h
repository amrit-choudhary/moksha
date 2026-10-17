/**
 * Vector3 class with X, Y and Z.
 * Coordinate Convention:
 * Looking at the screen:
 * X goes from left to right of the screen.
 * Y goes from bottom to top of the screen.
 * Z goes from viewer towards the screen.
 */

#pragma once

namespace ME {

/**
 * Float Vector3 class.
 */
class Vec3 {
   public:
    // Constructors
    Vec3();
    Vec3(float x, float y, float z);
    Vec3(const Vec3& v);
    Vec3(const Vec3&& v);
    Vec3& operator=(const Vec3& v);
    Vec3& operator=(const Vec3&& v);
    Vec3(float scalar);

    float x;
    float y;
    float z;

    // Vec3{0, 0, 0}.
    static const Vec3 Zero;
    // Vec3{1, 1, 1}.
    static const Vec3 One;
    // Vec3{0, 1, 0}.
    static const Vec3 Up;
    // Vec3{0, -1, 0}.
    static const Vec3 Down;
    // Vec3{1, 0, 0}.
    static const Vec3 Right;
    // Vec3{-1, 0, 0}.
    static const Vec3 Left;
    // Vec3{0, 0, 1}.
    static const Vec3 Forward;
    // Vec3{0, 0, -1}.
    static const Vec3 Backward;

    // Get length.
    float Length();
    // In place normalise this vector.
    void Normalise();
    // In place normalise this vector. Will also check for 0 length;
    void NormaliseSafe();
    // Returns a new vector that is normalised.
    Vec3 Normalised();

    // Operator overloads.
    Vec3 operator-() const;
    Vec3& operator+=(const Vec3& b);
    Vec3& operator-=(const Vec3& b);
    Vec3& operator*=(const Vec3& b);
    Vec3& operator/=(const Vec3& b);

    // Math functions
    static float Dot(const Vec3& a, const Vec3& b);
    static Vec3 Cross(const Vec3& a, const Vec3& b);

   private:
   protected:
};

// Operator overloads.
Vec3 operator+(const Vec3& a, const Vec3& b);
Vec3 operator-(const Vec3& a, const Vec3& b);
Vec3 operator*(const Vec3& a, const Vec3& b);
Vec3 operator/(const Vec3& a, const Vec3& b);

bool operator==(const Vec3& a, const Vec3& b);
bool operator!=(const Vec3& a, const Vec3& b);

// Math functions

}  // namespace ME