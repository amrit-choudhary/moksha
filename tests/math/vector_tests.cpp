/**
 * Tests for Vector classes.
 */

#include "moksha/shared/math/math.h"
#include "moksha/shared/math/vec2.h"
#include "moksha/test_framework/test_framework.h"

static constexpr float EPSILON = 0.0001f;

TEST(Vector3, Up) {
    ME::Vec3 vec3 = ME::Vec3::Up;
    EXPECT(vec3.x == 0.0f);
    EXPECT(vec3.y == 1.0f);
    EXPECT(vec3.z == 0.0f);
}

TEST(Vector3, Normalise) {
    ME::Vec3 vec3(3.0f, 4.0f, 0.0f);
    vec3.Normalise();
    EXPECT_NEAR(vec3.x, 0.6f, EPSILON);
    EXPECT_NEAR(vec3.y, 0.8f, EPSILON);
    EXPECT_NEAR(vec3.z, 0.0f, EPSILON);
}

TEST(Vector3, Dot) {
    ME::Vec3 vec1{1.0f, 2.0f, 3.0f};
    ME::Vec3 vec2{4.0f, -5.0f, 6.0f};
    EXPECT(ME::Vec3::Dot(vec1, vec2) == 12.0f);
}

TEST(Vector3, Cross) {
    ME::Vec3 a{1.0f, 0.0f, 0.0f};
    ME::Vec3 b{0.0f, 1.0f, 0.0f};
    ME::Vec3 c = ME::Vec3::Cross(a, b);
    EXPECT(c.x == 0.0f);
    EXPECT(c.y == 0.0f);
    EXPECT(c.z == 1.0f);
}

TEST(Vector4, One) {
    ME::Vec4 vec4 = ME::Vec4::One;
    EXPECT(vec4.x == 1.0f);
    EXPECT(vec4.y == 1.0f);
    EXPECT(vec4.z == 1.0f);
    EXPECT(vec4.w == 1.0f);
}

TEST(Vector4, Normalise) {
    ME::Vec4 vec4{3.0f, 4.0f, 0.0f, 0.0f};
    vec4.Normalise();
    EXPECT_NEAR(vec4.x, 0.6f, EPSILON);
    EXPECT_NEAR(vec4.y, 0.8f, EPSILON);
    EXPECT_NEAR(vec4.z, 0.0f, EPSILON);
    EXPECT_NEAR(vec4.w, 0.0f, EPSILON);
}

TEST(Vector4, Dot) {
    ME::Vec4 vec1{1.0f, 2.0f, 3.0f, 1.0f};
    ME::Vec4 vec2{4.0f, -5.0f, 6.0f, 1.0f};
    EXPECT(ME::Vec4::Dot(vec1, vec2) == 13.0f);
}

TEST(Vector3i, Up) {
    ME::Vec3i vec3i = ME::Vec3i::Up;
    EXPECT(vec3i.x == 0);
    EXPECT(vec3i.y == 1);
    EXPECT(vec3i.z == 0);
}

TEST(Vector3i, Length) {
    ME::Vec3i vec3i{3, 4, 0};
    EXPECT_NEAR(vec3i.Length(), 5.0f, EPSILON);
}

TEST(Vector3i, Dot) {
    ME::Vec3i vec1{1, 2, 3};
    ME::Vec3i vec2{4, -5, 6};
    EXPECT(ME::Vec3i::Dot(vec1, vec2) == 12);
}

TEST(Vector2, LengthAndNormalise) {
    ME::Vec2 vec2{3.0f, 4.0f};
    EXPECT_NEAR(vec2.Length(), 5.0f, EPSILON);
    vec2.Normalise();
    EXPECT_NEAR(vec2.x, 0.6f, EPSILON);
    EXPECT_NEAR(vec2.y, 0.8f, EPSILON);
}

TEST(Vector2, NormaliseSafeZeroStaysZero) {
    ME::Vec2 vec2{0.0f, 0.0f};
    vec2.NormaliseSafe();
    EXPECT(vec2.x == 0.0f);
    EXPECT(vec2.y == 0.0f);
}

TEST(Vector2, Operators) {
    ME::Vec2 a{1.0f, 2.0f};
    ME::Vec2 b{3.0f, 5.0f};

    EXPECT((a + b) == (ME::Vec2{4.0f, 7.0f}));
    EXPECT((b - a) == (ME::Vec2{2.0f, 3.0f}));
    EXPECT((a * 2.0f) == (ME::Vec2{2.0f, 4.0f}));
    EXPECT((-a) == (ME::Vec2{-1.0f, -2.0f}));
    EXPECT(ME::Vec2::Dot(a, b) == 13.0f);
}
