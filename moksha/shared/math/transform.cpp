#include "transform.h"

#include <cmath>

ME::Mat4 ME::Transform::GetModelMatrix() const {
    if (rotation.x != 0.0f || rotation.y != 0.0f) {
        ME::Mat4 translationMat = ME::Mat4::Translation(ME::Vec4{position.x, position.y, position.z, 1.0f});
        ME::Mat4 rotationMat = ME::Mat4::Rotation(rotation);
        ME::Mat4 scaleMat = ME::Mat4::Scale(ME::Vec4{scale.x, scale.y, scale.z, 1.0f});
        return translationMat * rotationMat * scaleMat;
    }

    // Mat4::Rotation with x = y = 0, times scale, with the translation in the last column.
    float c = 1.0f;
    float s = 0.0f;
    if (rotation.z != 0.0f) {
        c = std::cos(rotation.z);
        s = std::sin(rotation.z);
    }

    // clang-format off
    return ME::Mat4{
        c * scale.x,  s * scale.y, 0.0f,    position.x,
        -s * scale.x, c * scale.y, 0.0f,    position.y,
        0.0f,         0.0f,        scale.z, position.z,
        0.0f,         0.0f,        0.0f,    1.0f};
    // clang-format on
}

ME::Transform::Transform() {
    position = ME::Vec3(0.0f, 0.0f, 0.0f);
    rotation = ME::Vec4(0.0f, 0.0f, 0.0f, 1.0f);
    scale = ME::Vec3(1.0f, 1.0f, 1.0f);
}

ME::Transform::~Transform() {}

void ME::Transform::SetPosition(float x, float y, float z) {
    position.x = x;
    position.y = y;
    position.z = z;
    bDirty = true;
}

void ME::Transform::SetPosition(float x, float y) {
    SetPosition(x, y, 0.0f);
}

void ME::Transform::SetPosition(const ME::Vec3& pos) {
    position = pos;
    bDirty = true;
}

void ME::Transform::SetRotation(float x, float y, float z, float w) {
    rotation.x = x;
    rotation.y = y;
    rotation.z = z;
    rotation.w = w;
    bDirty = true;
}

void ME::Transform::SetScale(float x, float y, float z) {
    scale.x = x;
    scale.y = y;
    scale.z = z;
    bDirty = true;
}

void ME::Transform::SetScale(float x, float y) {
    SetScale(x, y, 1.0f);
}

void ME::Transform::SetScale(float scale) {
    SetScale(scale, scale, scale);
}

void ME::Transform::SetScale(const ME::Vec3& scale) {
    this->scale = scale;
    bDirty = true;
}
