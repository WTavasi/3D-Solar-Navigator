
#pragma once

// 3D camera and perspective projection.
// No external graphics libraries required.

struct Vec3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

struct ScreenPoint {
    double x = 0.0;
    double y = 0.0;
    double depth = 0.0;
    bool visible = false;
};

class Camera {
public:
    Vec3 target{0.0, 0.0, 0.0};

    // Angles are stored in degrees.
    double yaw = 0.0;
    double pitch = 35.0;
    double distance = 1000.0;
    double fov = 60.0;

    // Camera movement
    void rotate(double yawOffset, double pitchOffset);
    void zoom(double factor);
    void pan(double rightAmount, double upAmount);
    void reset();

    // Camera mathematics
    Vec3 getPosition() const;

    // Project a world-space point onto the screen.
    ScreenPoint project(
        const Vec3& worldPoint,
        int screenWidth,
        int screenHeight
    ) const;
};
