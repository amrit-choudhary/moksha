#include "vec2.h"

#include <cmath>

using namespace ME;

const Vec2 Vec2::Zero{0.0f, 0.0f};
const Vec2 Vec2::One{1.0f, 1.0f};
const Vec2 Vec2::Up{0.0f, 1.0f};
const Vec2 Vec2::Down{0.0f, -1.0f};
const Vec2 Vec2::Right{1.0f, 0.0f};
const Vec2 Vec2::Left{-1.0f, 0.0f};

float ME::Vec2::Length() {
    return std::sqrt(x * x + y * y);
}

void ME::Vec2::Normalise() {
    float len = Length();
    x = x / len;
    y = y / len;
}

void ME::Vec2::NormaliseSafe() {
    float len = Length();
    if (len == 0) {
        x = 0;
        y = 0;
    } else {
        x = x / len;
        y = y / len;
    }
}

Vec2 ME::Vec2::Normalised() {
    float len = Length();
    return (len > 0) ? Vec2{x / len, y / len} : Vec2{0, 0};
}

Vec2 ME::Vec2::operator-() const {
    return Vec2{-x, -y};
}

Vec2& ME::Vec2::operator+=(const Vec2& b) {
    x += b.x;
    y += b.y;
    return *this;
}

Vec2& ME::Vec2::operator-=(const Vec2& b) {
    x -= b.x;
    y -= b.y;
    return *this;
}

Vec2& ME::Vec2::operator*=(const Vec2& b) {
    x *= b.x;
    y *= b.y;
    return *this;
}

Vec2& ME::Vec2::operator/=(const Vec2& b) {
    x /= b.x;
    y /= b.y;
    return *this;
}

float ME::Vec2::Dot(const Vec2& a, const Vec2& b) {
    return (a.x * b.x + a.y * b.y);
}

Vec2 ME::operator+(const Vec2& a, const Vec2& b) {
    return Vec2{a.x + b.x, a.y + b.y};
}

Vec2 ME::operator-(const Vec2& a, const Vec2& b) {
    return Vec2{a.x - b.x, a.y - b.y};
}

Vec2 ME::operator*(const Vec2& a, const Vec2& b) {
    return Vec2{a.x * b.x, a.y * b.y};
}

Vec2 ME::operator/(const Vec2& a, const Vec2& b) {
    return Vec2{a.x / b.x, a.y / b.y};
}

bool ME::operator==(const Vec2& a, const Vec2& b) {
    return (a.x == b.x && a.y == b.y);
}

bool ME::operator!=(const Vec2& a, const Vec2& b) {
    return !(a == b);
}
