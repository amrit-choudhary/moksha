#include "matrix4.h"

#include <cmath>

#include "vec16.h"
#include "vec4.h"

// clang-format off

using namespace ME;

const Mat4 Mat4::Identity{
                        1.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 1.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 1.0f};

const Mat4 Mat4::Zero{
                        0.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 0.0f};

const Mat4 Mat4::One{
                        1.0f, 1.0f, 1.0f, 1.0f,
                        1.0f, 1.0f, 1.0f, 1.0f,
                        1.0f, 1.0f, 1.0f, 1.0f,
                        1.0f, 1.0f, 1.0f, 1.0f};
                        
Vec4 Mat4::GetRow(const int row) const {
    switch (row) {
        case 0:
            return Vec4{m00, m01, m02, m03};
        case 1:
            return Vec4{m10, m11, m12, m13};
        case 2:
            return Vec4{m20, m21, m22, m23};
        case 3:
            return Vec4{m30, m31, m32, m33};
        default:
            return Vec4{0.0f, 0.0f, 0.0f, 0.0f};
    }
}
Vec4 Mat4::GetColumn(const int column) const {
    switch (column) {
        case 0:
            return Vec4{m00, m10, m20, m30};
        case 1:
            return Vec4{m01, m11, m21, m31};
        case 2:
            return Vec4{m02, m12, m22, m32};
        case 3:
            return Vec4{m03, m13, m23, m33};
        default:
            return Vec4{0.0f, 0.0f, 0.0f, 0.0f};
    }
}

Vec16 ME::Mat4::GetData() const {
    return GetDataColumnMajor();
}

Vec16 ME::Mat4::GetDataRowMajor() const {
    return Vec16{
        m00, m01, m02, m03, 
        m10, m11, m12, m13, 
        m20, m21, m22, m23, 
        m30, m31, m32, m33};
}

Vec16 ME::Mat4::GetDataColumnMajor() const {
    return Vec16{
        m00, m10, m20, m30, 
        m01, m11, m21, m31, 
        m02, m12, m22, m32, 
        m03, m13, m23, m33};
}

Vec16 ME::Mat4::GetDataForShader() const {
    #ifdef ME_METAL
    return GetDataColumnMajor();
    #endif

    #ifdef ME_DX12
    return GetDataRowMajor();
    #endif

    return GetDataColumnMajor();
}

Mat4 ME::Mat4::Translation(const Vec4 & translation)
{
    return Mat4{
        1.0f, 0.0f, 0.0f, translation.x,
        0.0f, 1.0f, 0.0f, translation.y,
        0.0f, 0.0f, 1.0f, translation.z,
        0.0f, 0.0f, 0.0f, 1.0f};
}

Mat4 ME::Mat4::Rotation(const Vec4 & rotation)
{
    float cAlpha = std::cos(rotation.x);
    float sAlpha = std::sin(rotation.x);
    float cBeta = std::cos(rotation.y);
    float sBeta = std::sin(rotation.y);
    float cGamma = std::cos(rotation.z);
    float sGamma = std::sin(rotation.z);

    return Mat4{
        cBeta * cGamma, cAlpha * sGamma + sAlpha * sBeta * cGamma, sAlpha * sGamma - cAlpha * sBeta * cGamma, 0.0f,
        -cBeta * sGamma, cAlpha * cGamma - sAlpha * sBeta * sGamma, sAlpha * cGamma + cAlpha * sBeta * sGamma, 0.0f,
        sBeta, -sAlpha * cBeta, cAlpha * cBeta, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f};
}

Mat4 ME::Mat4::Scale(const Vec4 & scale)
{
    return Mat4{
        scale.x, 0.0f, 0.0f, 0.0f,
        0.0f, scale.y, 0.0f, 0.0f,
        0.0f, 0.0f, scale.z, 0.0f,
        0.0f, 0.0f, 0.0f, scale.w};
}

Mat4 ME::Mat4::Perspective(float fov, float aspect, float near, float far) {
    #ifdef ME_METAL
    return PerspectiveMTL(fov, aspect, near, far);
    #endif

    #ifdef ME_DX12
    return PerspectiveDX(fov, aspect, near, far);
    #endif

    return Mat4::Identity;
}

Mat4 ME::Mat4::Orthographic(float left, float right, float bottom, float top, float near, float far) {
    #ifdef ME_METAL
    return OrthographicMTL(left, right, bottom, top, near, far);
    #endif

    #ifdef ME_DX12
    return OrthographicDX(left, right, bottom, top, near, far);
    #endif

    return Mat4::Identity;
}

Mat4 ME::Mat4::PerspectiveMTL(float fov, float aspect, float near, float far) {
    float f = 1.0f / std::tan(fov / 2.0f);
    return Mat4{
        f / aspect, 0.0f, 0.0f, 0.0f,
        0.0f, f, 0.0f, 0.0f,
        0.0f, 0.0f, far / (far - near), - far * near / (far - near),
        0.0f, 0.0f, 1.0f, 0.0f};
}

Mat4 ME::Mat4::PerspectiveDX(float fov, float aspect, float near, float far) {
    float f = 1.0f / std::tan(fov / 2.0f);
    return Mat4{
        f / aspect, 0.0f, 0.0f, 0.0f,
        0.0f, f, 0.0f, 0.0f,
        0.0f, 0.0f, far / (far - near), -near * far / (far - near),
        0.0f, 0.0f, 1.0f, 0.0f};
}

Mat4 ME::Mat4::OrthographicMTL(float left, float right, float bottom, float top, float near, float far) {
    return Mat4{
        2.0f / (right - left), 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
        0.0f, 0.0f, -2.0f / (far - near), 0.0f,
        -(right + left) / (right - left), -(top + bottom) / (top - bottom), -(far + near) / (far - near), 1.0f};
}

Mat4 ME::Mat4::OrthographicDX(float left, float right, float bottom, float top, float near, float far) {
    return Mat4{
        2.0f / (right - left), 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f / (far - near), 0.0f,
        -(right + left) / (right - left), -(top + bottom) / (top - bottom), -near / (far - near), 1.0f};
}

Mat4 ME::Mat4::View(const Vec4& position, const Vec4& target, const Vec4& up) {
    Vec4 zaxis = (target - position).Normalised();
    Vec4 xaxis = Vec4::Cross(up, zaxis).Normalised();
    Vec4 yaxis = Vec4::Cross(zaxis, xaxis);
    float a = Vec4::Dot(-xaxis, position);
    float b = Vec4::Dot(-yaxis, position);
    float c = Vec4::Dot(-zaxis, position);

    return Mat4{
        xaxis.x, xaxis.y, xaxis.z, a,
        yaxis.x, yaxis.y, yaxis.z, b,
        zaxis.x, zaxis.y, zaxis.z, c,
        0.0f, 0.0f, 0.0f, 1.0f};
}

Mat4 ME::Mat4::GetTranspose() const
{
    return Mat4{
        m00, m10, m20, m30,
        m01, m11, m21, m31,
        m02, m12, m22, m32,
        m03, m13, m23, m33};
}

Mat4 Mat4::operator*(const Mat4& other) const {
    Mat4 result;

    result.m00 = m00 * other.m00 + m01 * other.m10 + m02 * other.m20 + m03 * other.m30;
    result.m01 = m00 * other.m01 + m01 * other.m11 + m02 * other.m21 + m03 * other.m31;
    result.m02 = m00 * other.m02 + m01 * other.m12 + m02 * other.m22 + m03 * other.m32;
    result.m03 = m00 * other.m03 + m01 * other.m13 + m02 * other.m23 + m03 * other.m33;

    result.m10 = m10 * other.m00 + m11 * other.m10 + m12 * other.m20 + m13 * other.m30;
    result.m11 = m10 * other.m01 + m11 * other.m11 + m12 * other.m21 + m13 * other.m31;
    result.m12 = m10 * other.m02 + m11 * other.m12 + m12 * other.m22 + m13 * other.m32;
    result.m13 = m10 * other.m03 + m11 * other.m13 + m12 * other.m23 + m13 * other.m33;

    result.m20 = m20 * other.m00 + m21 * other.m10 + m22 * other.m20 + m23 * other.m30;
    result.m21 = m20 * other.m01 + m21 * other.m11 + m22 * other.m21 + m23 * other.m31;
    result.m22 = m20 * other.m02 + m21 * other.m12 + m22 * other.m22 + m23 * other.m32;
    result.m23 = m20 * other.m03 + m21 * other.m13 + m22 * other.m23 + m23 * other.m33;

    result.m30 = m30 * other.m00 + m31 * other.m10 + m32 * other.m20 + m33 * other.m30;
    result.m31 = m30 * other.m01 + m31 * other.m11 + m32 * other.m21 + m33 * other.m31;
    result.m32 = m30 * other.m02 + m31 * other.m12 + m32 * other.m22 + m33 * other.m32;
    result.m33 = m30 * other.m03 + m31 * other.m13 + m32 * other.m23 + m33 * other.m33;

    return result;
}

// clang-format on
