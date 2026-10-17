#include "camera.h"

#include <cmath>

#include "moksha/shared/math/math.h"

ME::Camera::Camera() {
    projectionType = ProjectionType::Perspective;

    position = ME::Vec3::Zero;
    rotation = ME::Vec4{0.0f, 0.0f, 0.0f, 1.0f};
    up = ME::Vec3::Up;
    viewPosition = ME::Vec3(0.0f, 0.0f, 1000.0f);

    fov = 90.0f;
    aspectRatio = 1.3333f;
    nearPlane = 0.1f;
    farPlane = 10'000.0f;
}

ME::Camera::~Camera() {}

ME::Mat4 ME::Camera::GetViewMatrix() {
    return ME::Mat4::View(ME::Vec4{position.x, position.y, position.z, 1.0f},
                          ME::Vec4{viewPosition.x, viewPosition.y, viewPosition.z, 1.0f},
                          ME::Vec4{up.x, up.y, up.z, 1.0f});
}

ME::Mat4 ME::Camera::GetProjectionMatrix() {
    if (projectionType == ProjectionType::Perspective) {
        return ME::Mat4::Perspective(fov * (ME::PI / 180.0f), aspectRatio, nearPlane, farPlane);
    } else if (projectionType == ProjectionType::Orthographic) {
        return ME::Mat4::Orthographic(-orthographicSize * aspectRatio, orthographicSize * aspectRatio,
                                      -orthographicSize, orthographicSize, nearPlane, farPlane);
    } else {
        return ME::Mat4::Identity;
    }
}
