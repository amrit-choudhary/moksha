#include "vec3.h"

#include <cmath>

using namespace ME;

const Vec3 Vec3::Zero(0.0f, 0.0f, 0.0f);
const Vec3 Vec3::One(1.0f, 1.0f, 1.0f);

const Vec3 Vec3::Up(0.0f, 1.0f, 0.0f);
const Vec3 Vec3::Down(0.0f, -1.0f, 0.0f);

const Vec3 Vec3::Right(1.0f, 0.0f, 0.0f);
const Vec3 Vec3::Left(-1.0f, 0.0f, 0.0f);

const Vec3 Vec3::Forward(0.0f, 0.0f, 1.0f);
const Vec3 Vec3::Backward(0.0f, 0.0f, -1.0f);

float ME::Vec3::Length() {
    return std::sqrt(x * x + y * y + z * z);
}

void ME::Vec3::Normalise() {
    float len = Length();
    x = x / len;
    y = y / len;
    z = z / len;
}

void ME::Vec3::NormaliseSafe() {
    float len = Length();
    if (len == 0) {
        x = 0;
        y = 0;
        z = 0;
    } else {
        x = x / len;
        y = y / len;
        z = z / len;
    }
}

Vec3 ME::Vec3::Normalised() {
    float len = Length();
    return (len > 0) ? Vec3(x / len, y / len, z / len) : Vec3(0, 0, 0);
}

Vec3 ME::Vec3::operator-() const {
    return Vec3(-x, -y, -z);
}

Vec3& ME::Vec3::operator+=(const Vec3& b) {
    x += b.x;
    y += b.y;
    z += b.z;
    return *this;
}

Vec3& ME::Vec3::operator-=(const Vec3& b) {
    x -= b.x;
    y -= b.y;
    z -= b.z;
    return *this;
}

Vec3& ME::Vec3::operator*=(const Vec3& b) {
    x *= b.x;
    y *= b.y;
    z *= b.z;
    return *this;
}

Vec3& ME::Vec3::operator/=(const Vec3& b) {
    x /= b.x;
    y /= b.y;
    z /= b.z;
    return *this;
}

float ME::Vec3::Dot(const Vec3& a, const Vec3& b) {
    return (a.x * b.x + a.y * b.y + a.z * b.z);
}

Vec3 ME::Vec3::Cross(const Vec3& a, const Vec3& b) {
    return Vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

Vec3 ME::operator+(const Vec3& a, const Vec3& b) {
    return Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vec3 ME::operator-(const Vec3& a, const Vec3& b) {
    return Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vec3 ME::operator*(const Vec3& a, const Vec3& b) {
    return Vec3(a.x * b.x, a.y * b.y, a.z * b.z);
}

Vec3 ME::operator/(const Vec3& a, const Vec3& b) {
    return Vec3(a.x / b.x, a.y / b.y, a.z / b.z);
}

bool ME::operator==(const Vec3& a, const Vec3& b) {
    if (a.x == b.x && a.y == b.y && a.z == b.z) {
        return true;
    } else {
        return false;
    }
}

bool ME::operator!=(const Vec3& a, const Vec3& b) {
    return !(a == b);
}

ME::Vec3::Vec3() {
    x = 0;
    y = 0;
    z = 0;
}

ME::Vec3::Vec3(float x, float y, float z) {
    this->x = x;
    this->y = y;
    this->z = z;
}

ME::Vec3::Vec3(const Vec3& v) {
    x = v.x;
    y = v.y;
    z = v.z;
}

ME::Vec3::Vec3(const Vec3&& v) {
    x = v.x;
    y = v.y;
    z = v.z;
}

Vec3& ME::Vec3::operator=(const Vec3& v) {
    x = v.x;
    y = v.y;
    z = v.z;
    return *this;
}

Vec3& ME::Vec3::operator=(const Vec3&& v) {
    x = v.x;
    y = v.y;
    z = v.z;
    return *this;
}

ME::Vec3::Vec3(float scalar) {
    x = scalar;
    y = scalar;
    z = scalar;
}
