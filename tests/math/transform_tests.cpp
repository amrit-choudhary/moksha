/**
 * Tests for Transform: the dirty flag set by every setter, and the model matrix (the closed-form z-only path and the
 * general path both equal Translation * Rotation * Scale).
 */

#include "moksha/shared/math/matrix4.h"
#include "moksha/shared/math/transform.h"
#include "moksha/shared/math/vec3.h"
#include "moksha/shared/math/vec4.h"
#include "moksha/test_framework/test_framework.h"

static constexpr float MATRIX_EPSILON = 0.0001f;

static ME::Mat4 ReferenceModelMatrix(const ME::Transform& transform) {
    const ME::Vec3 p = transform.GetPosition();
    const ME::Vec3 s = transform.GetScale();
    return ME::Mat4::Translation(ME::Vec4{p.x, p.y, p.z, 1.0f}) * ME::Mat4::Rotation(transform.GetRotation()) *
           ME::Mat4::Scale(ME::Vec4{s.x, s.y, s.z, 1.0f});
}

static bool MatricesNear(const ME::Mat4& a, const ME::Mat4& b, float epsilon) {
    const float* pa = &a.m00;
    const float* pb = &b.m00;
    for (int i = 0; i < 16; ++i) {
        const float d = pa[i] - pb[i];
        if (d > epsilon || -d > epsilon) {
            return false;
        }
    }
    return true;
}

// Fixed sets of positions, scales (including negative, mirrored ones) and z angles.
static constexpr float POSITIONS[][3] = {{0.0f, 0.0f, 0.0f}, {12.5f, -3.0f, 4.0f}, {-250.0f, 900.0f, 0.5f}};
static constexpr float SCALES[][3] = {{1.0f, 1.0f, 1.0f}, {32.0f, 8.0f, 1.0f}, {-2.0f, 3.0f, 1.0f}, {0.0f, 0.0f, 1.0f}};
static constexpr float ANGLES[] = {0.3f, -1.2f, 1.5707964f, 3.1415927f, 5.0f, -7.5f};

TEST(Transform, NewIsDirty) {
    ME::Transform transform;
    EXPECT(transform.IsDirty());
}

TEST(Transform, ClearDirty) {
    ME::Transform transform;
    transform.ClearDirty();
    EXPECT(!transform.IsDirty());
}

TEST(Transform, SettersMarkDirty) {
    ME::Transform transform;

    transform.ClearDirty();
    transform.SetPosition(1.0f, 2.0f, 3.0f);
    EXPECT(transform.IsDirty());

    transform.ClearDirty();
    transform.SetPosition(1.0f, 2.0f);
    EXPECT(transform.IsDirty());

    transform.ClearDirty();
    transform.SetPosition(ME::Vec3(1.0f, 2.0f, 3.0f));
    EXPECT(transform.IsDirty());

    transform.ClearDirty();
    transform.SetRotation(0.0f, 0.0f, 1.0f);
    EXPECT(transform.IsDirty());

    transform.ClearDirty();
    transform.SetScale(1.0f, 2.0f, 3.0f);
    EXPECT(transform.IsDirty());

    transform.ClearDirty();
    transform.SetScale(1.0f, 2.0f);
    EXPECT(transform.IsDirty());

    transform.ClearDirty();
    transform.SetScale(2.0f);
    EXPECT(transform.IsDirty());

    transform.ClearDirty();
    transform.SetScale(ME::Vec3(1.0f, 2.0f, 3.0f));
    EXPECT(transform.IsDirty());
}

TEST(Transform, GettersKeepClean) {
    ME::Transform transform;
    transform.ClearDirty();
    (void)transform.GetPosition();
    (void)transform.GetRotation();
    (void)transform.GetScale();
    (void)transform.GetModelMatrix();
    EXPECT(!transform.IsDirty());
}

TEST(Transform, ModelMatrixZRotationMatchesReference) {
    for (const auto& p : POSITIONS) {
        for (const auto& s : SCALES) {
            for (float angle : ANGLES) {
                ME::Transform transform;
                transform.SetPosition(p[0], p[1], p[2]);
                transform.SetScale(s[0], s[1], s[2]);
                transform.SetRotation(0.0f, 0.0f, angle);
                EXPECT(MatricesNear(transform.GetModelMatrix(), ReferenceModelMatrix(transform), MATRIX_EPSILON));
            }
        }
    }
}

TEST(Transform, ModelMatrixNoRotationIsExact) {
    for (const auto& p : POSITIONS) {
        for (const auto& s : SCALES) {
            ME::Transform transform;
            transform.SetPosition(p[0], p[1], p[2]);
            transform.SetScale(s[0], s[1], s[2]);
            EXPECT(MatricesNear(transform.GetModelMatrix(), ReferenceModelMatrix(transform), 0.0f));
        }
    }
}

TEST(Transform, ModelMatrixXYRotationMatchesReference) {
    ME::Transform transform;
    transform.SetPosition(5.0f, -2.0f, 3.0f);
    transform.SetScale(2.0f, 4.0f, 0.5f);

    transform.SetRotation(0.7f, 0.0f, 0.4f);
    EXPECT(MatricesNear(transform.GetModelMatrix(), ReferenceModelMatrix(transform), MATRIX_EPSILON));

    transform.SetRotation(0.0f, -1.1f, 0.0f);
    EXPECT(MatricesNear(transform.GetModelMatrix(), ReferenceModelMatrix(transform), MATRIX_EPSILON));

    transform.SetRotation(0.25f, 0.5f, 2.0f);
    EXPECT(MatricesNear(transform.GetModelMatrix(), ReferenceModelMatrix(transform), MATRIX_EPSILON));
}
