/**
 * 4 x 4 Matrix
 */

#pragma once

namespace ME {

class Vec4;
class Vec16;

/**
 * 4 x 4 Matrix class.
 */

class Mat4 {
   public:
    float m00, m01, m02, m03;
    float m10, m11, m12, m13;
    float m20, m21, m22, m23;
    float m30, m31, m32, m33;

    static const Mat4 Identity;
    static const Mat4 Zero;
    static const Mat4 One;

    Vec4 GetRow(const int row) const;

    Vec4 GetColumn(const int column) const;

    // Returns the matrix as a Vec16. Column major order. Default is column major.
    Vec16 GetData() const;

    // Returns the matrix as a Vec16. Row major order.
    Vec16 GetDataRowMajor() const;

    // Returns the matrix as a Vec16. Column major order.
    Vec16 GetDataColumnMajor() const;

    /*
     * Returns the matrix as a Vec16 formatted for shader usage.
     * DirectX: Row Major
     * Metal/Vulkan: Column Major
     */
    Vec16 GetDataForShader() const;

    // Creates a Translation matrix based on provided translation vector.
    static Mat4 Translation(const Vec4& translation);

    // Creates a Rotation matrix based on provided rotation vector.
    static Mat4 Rotation(const Vec4& rotation);

    // Creates a Scale matrix based on provided scale vector.
    static Mat4 Scale(const Vec4& scale);

    // Creates a platform dependent Orthographic matrix based on provided left, right, bottom, top, near and far planes.
    // For Metal and Vulkan, the resulting matrix will have a clip space of z: [0, 1]. For DirectX, the resulting matrix
    // will have a clip space of z: [0, 1].
    static Mat4 Orthographic(float left, float right, float bottom, float top, float near, float far);

    // Creates an Orthographic matrix based on provided left, right, bottom, top, near and far planes.
    static Mat4 OrthographicMTL(float left, float right, float bottom, float top, float near, float far);

    // Creates an Orthographic matrix based on provided left, right, bottom, top, near and far planes.
    static Mat4 OrthographicDX(float left, float right, float bottom, float top, float near, float far);

    /**
     * Creates a platform dependent Perspective matrix based on provided field of view, aspect ratio, near and far
     * planes. For Metal and Vulkan, the resulting matrix will have a clip space of z: [0, 1]. For DirectX, the
     * resulting matrix will have a clip space of z: [0, 1].
     */
    static Mat4 Perspective(float fov, float aspect, float near, float far);

    // Creates a Perspective matrix based on provided field of view, aspect ratio, near and far planes.
    static Mat4 PerspectiveMTL(float fov, float aspect, float near, float far);

    // Creates a Perspective matrix based on provided field of view, aspect ratio, near and far planes.
    static Mat4 PerspectiveDX(float fov, float aspect, float near, float far);

    // Create a View matrix based on the camera position, target position and up vector.
    static Mat4 View(const Vec4& position, const Vec4& target, const Vec4& up);

    // Returns a new matrix that is the transpose of this matrix.
    Mat4 GetTranspose() const;

    // Multiplies this matrix by another matrix and returns the result.
    Mat4 operator*(const Mat4& other) const;
};

}  // namespace ME
