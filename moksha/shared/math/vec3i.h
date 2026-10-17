/**
 * Vector3 class with X, Y, and Z as integers.
 * Coordinate Convention:
 * Looking at the screen:
 * X goes from left to right of the screen.
 * Y goes from bottom to top of the screen.
 * Z goes from viewer towards the screen.
 */

#pragma once

namespace ME {

class Vec3;

/**
 * Integer Vector3 class.
 */
class Vec3i {
   public:
    int x;
    int y;
    int z;

    // Vec3i{0, 0, 0}.
    static const Vec3i Zero;
    // Vec3i{1, 1, 1}.
    static const Vec3i One;
    // Vec3i{0, 1, 0}.
    static const Vec3i Up;
    // Vec3i{0, -1, 0}.
    static const Vec3i Down;
    // Vec3i{1, 0, 0}.
    static const Vec3i Right;
    // Vec3i{-1, 0, 0}.
    static const Vec3i Left;
    // Vec3i{0, 0, 1}.
    static const Vec3i Forward;
    // Vec3i{0, 0, -1}.
    static const Vec3i Backward;

    // Get length (returns a float since length is not necessarily an integer).
    float Length() const;
    // Returns a new vector that is normalized (returns a float vector).
    Vec3 Normalised() const;
    // Returns a new vector that is normalized (returns a float vector).
    Vec3i NormalisedInt() const;

    // Operator overloads.
    Vec3i operator-() const;
    Vec3i& operator+=(const Vec3i& b);
    Vec3i& operator-=(const Vec3i& b);
    Vec3i& operator*=(const Vec3i& b);
    Vec3i& operator/=(const Vec3i& b);

    // Math functions
    static int Dot(const Vec3i& a, const Vec3i& b);
    static Vec3i Cross(const Vec3i& a, const Vec3i& b);

   private:
   protected:
};

// Operator overloads.
Vec3i operator+(const Vec3i& a, const Vec3i& b);
Vec3i operator-(const Vec3i& a, const Vec3i& b);
Vec3i operator*(const Vec3i& a, const Vec3i& b);
Vec3i operator/(const Vec3i& a, const Vec3i& b);

bool operator==(const Vec3i& a, const Vec3i& b);
bool operator!=(const Vec3i& a, const Vec3i& b);

}  // namespace ME
