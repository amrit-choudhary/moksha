/**
 * Fixed point number.
 * Uses a int32. 24 bits used for whole part and 4 bits for fractional part.
 * Based on library:
 * https://github.com/MikeLankamp/fpm
 *
 * Range:
 * whole part = 24 bits =-8,388,608 to 8,388,607
 * fractional part = 8 bits = 1/256 to 255/256.
 * Smallest fraction = 0.00390625
 * Bigget fraction = 0.99609375
 *
 * Rounding is enabled.
 *
 * Notes:
 * 1. Using += operator will not instantiate new objects, compared to + operator.
 */

#pragma once

#include <cstdint>

namespace ME {
class FP_24_8 {
   public:
    // Constructors.
    FP_24_8();
    // Create a fp with an int.
    FP_24_8(int32_t val);
    // Create a fp with an int.
    FP_24_8(int32_t val, bool asRaw);
    // Create a fp with a float.
    FP_24_8(float val);
    // Create a fp with double.
    FP_24_8(double val);
    // Create from other FP.
    FP_24_8(const FP_24_8& val);

    // Helper Functions.
    // Get the raw value. Do not use.
    int32_t GetRaw() const;
    int32_t ToInt() const;
    float ToFloat() const;
    double ToDouble() const;

    // Create a FP using another FPs raw value
    static FP_24_8 FromRawValue(int32_t val);

    // Operator overloads for directly manipulating the FP.
    FP_24_8 operator-() const;
    FP_24_8& operator+=(const FP_24_8& b);
    FP_24_8& operator+=(int32_t b);
    FP_24_8& operator+=(float b);
    FP_24_8& operator+=(double b);

    FP_24_8& operator-=(const FP_24_8& b);
    FP_24_8& operator-=(int32_t b);
    FP_24_8& operator-=(float b);
    FP_24_8& operator-=(double b);

    FP_24_8& operator*=(const FP_24_8& b);
    FP_24_8& operator/=(const FP_24_8& b);

   private:
    // integer representing the fixed point value.
    int32_t rawValue;
    // fraction multiplier will be common for all fp_24_8 types.
    static constexpr int32_t fractionMult = 256;
};

FP_24_8 operator+(const FP_24_8& a, const FP_24_8& b);
FP_24_8 operator+(const FP_24_8& a, int32_t b);
FP_24_8 operator+(const FP_24_8& a, float b);
FP_24_8 operator+(const FP_24_8& a, double b);
FP_24_8 operator-(const FP_24_8& a, const FP_24_8& b);
FP_24_8 operator*(const FP_24_8& a, const FP_24_8& b);
FP_24_8 operator/(const FP_24_8& a, const FP_24_8& b);

bool operator==(const FP_24_8& a, const FP_24_8& b);
bool operator!=(const FP_24_8& a, const FP_24_8& b);
bool operator<(const FP_24_8& a, const FP_24_8& b);
bool operator>(const FP_24_8& a, const FP_24_8& b);
bool operator<=(const FP_24_8& a, const FP_24_8& b);
bool operator>=(const FP_24_8& a, const FP_24_8& b);

}  // namespace ME