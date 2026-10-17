#include "vec3i.h"

#include <cmath>

#include "vec3.h"

using namespace ME;

const Vec3i Vec3i::Zero{0, 0, 0};
const Vec3i Vec3i::One{1, 1, 1};

const Vec3i Vec3i::Up{0, 1, 0};
const Vec3i Vec3i::Down{0, -1, 0};

const Vec3i Vec3i::Right{1, 0, 0};
const Vec3i Vec3i::Left{-1, 0, 0};

const Vec3i Vec3i::Forward{0, 0, 1};
const Vec3i Vec3i::Backward{0, 0, -1};

float ME::Vec3i::Length() const {
    return std::sqrt(x * x + y * y + z * z);
}

Vec3 ME::Vec3i::Normalised() const {
    float len = Length();
    if (len > 0) {
        return Vec3(static_cast<float>(x) / len, static_cast<float>(y) / len, static_cast<float>(z) / len);
    } else {
        return Vec3::Zero;
    }
}

Vec3i ME::Vec3i::NormalisedInt() const {
    float len = Length();
    if (len > 0) {
        return Vec3i{static_cast<int>(x / len), static_cast<int>(y / len), static_cast<int>(z / len)};
    } else {
        return Vec3i::Zero;
    }
}

Vec3i ME::Vec3i::operator-() const {
    return Vec3i{-x, -y, -z};
}

Vec3i& ME::Vec3i::operator+=(const Vec3i& b) {
    x += b.x;
    y += b.y;
    z += b.z;
    return *this;
}

Vec3i& ME::Vec3i::operator-=(const Vec3i& b) {
    x -= b.x;
    y -= b.y;
    z -= b.z;
    return *this;
}

Vec3i& ME::Vec3i::operator*=(const Vec3i& b) {
    x *= b.x;
    y *= b.y;
    z *= b.z;
    return *this;
}

Vec3i& ME::Vec3i::operator/=(const Vec3i& b) {
    x /= b.x;
    y /= b.y;
    z /= b.z;
    return *this;
}

int ME::Vec3i::Dot(const Vec3i& a, const Vec3i& b) {
    return (a.x * b.x + a.y * b.y + a.z * b.z);
}

Vec3i ME::Vec3i::Cross(const Vec3i& a, const Vec3i& b) {
    return Vec3i{a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

Vec3i ME::operator+(const Vec3i& a, const Vec3i& b) {
    return Vec3i{a.x + b.x, a.y + b.y, a.z + b.z};
}

Vec3i ME::operator-(const Vec3i& a, const Vec3i& b) {
    return Vec3i{a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec3i ME::operator*(const Vec3i& a, const Vec3i& b) {
    return Vec3i{a.x * b.x, a.y * b.y, a.z * b.z};
}

Vec3i ME::operator/(const Vec3i& a, const Vec3i& b) {
    return Vec3i{a.x / b.x, a.y / b.y, a.z / b.z};
}

bool ME::operator==(const Vec3i& a, const Vec3i& b) {
    return (a.x == b.x && a.y == b.y && a.z == b.z);
}

bool ME::operator!=(const Vec3i& a, const Vec3i& b) {
    return !(a == b);
}
