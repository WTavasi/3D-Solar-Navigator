
#include "scene/camera.h"

#include <algorithm>
#include <cmath>

namespace {

constexpr double PI = 3.14159265358979323846;

double radians(double degrees) {
    return degrees * PI / 180.0;
}

Vec3 add(Vec3 a, Vec3 b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

Vec3 subtract(Vec3 a, Vec3 b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec3 multiply(Vec3 a, double value) {
    return {a.x * value, a.y * value, a.z * value};
}

double dot(Vec3 a, Vec3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

Vec3 cross(Vec3 a, Vec3 b) {
    return {
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x
    };
}

Vec3 normalize(Vec3 v) {
    double length = std::sqrt(dot(v, v));

    if (length < 1e-10) {
        return {0.0, 0.0, 0.0};
    }

    return multiply(v, 1.0 / length);
}

} // namespace

// Rotate around the camera target.
void Camera::rotate(double yawOffset, double pitchOffset) {
    yaw += yawOffset;
    pitch = std::clamp(
        pitch + pitchOffset,
        -89.0,
        89.0
    );
}

// factor below 1 zooms in.
// factor above 1 zooms out.
void Camera::zoom(double factor) {
    if (factor <= 0.0) {
        return;
    }

    distance = std::clamp(
        distance * factor,
        20.0,
        10000.0
    );
}

// Calculate the camera position from spherical coordinates.
Vec3 Camera::getPosition() const {
    double yawRad = radians(yaw);
    double pitchRad = radians(pitch);

    return {
        target.x + distance *
            std::cos(pitchRad) * std::sin(yawRad),

        target.y + distance *
            std::sin(pitchRad),

        target.z + distance *
            std::cos(pitchRad) * std::cos(yawRad)
    };
}

// Move the camera target along the camera's local
// horizontal and vertical directions.
void Camera::pan(double rightAmount, double upAmount) {
    Vec3 position = getPosition();

    Vec3 forward = normalize(subtract(target, position));
    Vec3 worldUp{0.0, 1.0, 0.0};

    Vec3 right = normalize(cross(forward, worldUp));
    Vec3 up = normalize(cross(right, forward));

    target = add(target, add(
        multiply(right, rightAmount),
        multiply(up, upAmount)
    ));
}

// Reset camera to its starting view.
void Camera::reset() {
    target = {0.0, 0.0, 0.0};
    yaw = 0.0;
    pitch = 35.0;
    distance = 1000.0;
    fov = 60.0;
}

// Perspective projection from 3D world coordinates
// to 2D screen coordinates.
ScreenPoint Camera::project(
    const Vec3& worldPoint,
    int screenWidth,
    int screenHeight
) const {
    ScreenPoint result;

    if (screenWidth <= 0 || screenHeight <= 0 ||
        fov <= 0.0 || fov >= 180.0) {
        return result;
    }

    Vec3 position = getPosition();

    // Construct the camera coordinate system.
    Vec3 forward = normalize(subtract(target, position));
    Vec3 worldUp{0.0, 1.0, 0.0};

    Vec3 right = normalize(cross(forward, worldUp));
    Vec3 up = normalize(cross(right, forward));

    Vec3 relative = subtract(worldPoint, position);

    // Transform world coordinates into camera space.
    double cameraX = dot(relative, right);
    double cameraY = dot(relative, up);
    double cameraZ = dot(relative, forward);

    result.depth = cameraZ;

    // Reject points behind or too close to the camera.
    constexpr double nearPlane = 1.0;

    if (cameraZ <= nearPlane) {
        return result;
    }

    // Perspective focal length in pixels.
    double focalLength =
        (screenHeight * 0.5) /
        std::tan(radians(fov) * 0.5);

    // Perspective divide.
    result.x = screenWidth * 0.5 +
        focalLength * cameraX / cameraZ;

    result.y = screenHeight * 0.5 -
        focalLength * cameraY / cameraZ;

    result.visible =
        std::isfinite(result.x) &&
        std::isfinite(result.y) &&
        result.x >= 0.0 &&
        result.x < screenWidth &&
        result.y >= 0.0 &&
        result.y < screenHeight;

    return result;
}
