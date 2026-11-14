
#ifndef ORBIT_MATH_H
#define ORBIT_MATH_H

#include <cmath>
#include <stdexcept>

namespace orbit {

constexpr double PI = 3.14159265358979323846;

// A two-dimensional position on the rendering canvas.
struct Position {
    double x;
    double y;
};

// Convert degrees to radians.
inline double degreesToRadians(double degrees) {
    return degrees * PI / 180.0;
}

// Keep an angle between 0 and 360 degrees.
inline double normalizeAngle(double degrees) {
    double angle = std::fmod(degrees, 360.0);

    if (angle < 0.0) {
        angle += 360.0;
    }

    return angle;
}

// Calculate a position on an ellipse.
//
// centerX, centerY: centre of the orbit
// radiusX, radiusY: horizontal and vertical radii
// angleDegrees: current orbital angle
inline Position ellipticalPosition(
    double centerX,
    double centerY,
    double radiusX,
    double radiusY,
    double angleDegrees
) {
    double radians = degreesToRadians(angleDegrees);

    return {
        centerX + radiusX * std::cos(radians),
        centerY + radiusY * std::sin(radians)
    };
}

// Calculate angular speed in degrees per day.
inline double angularSpeed(double orbitalPeriodDays) {
    if (!std::isfinite(orbitalPeriodDays) ||
        orbitalPeriodDays <= 0.0) {
        throw std::invalid_argument(
            "Orbital period must be positive and finite"
        );
    }

    return 360.0 / orbitalPeriodDays;
}

// Update an orbital angle after elapsed simulation days.
inline double advanceAngle(
    double initialAngle,
    double elapsedDays,
    double orbitalPeriodDays
) {
    if (!std::isfinite(initialAngle) ||
        !std::isfinite(elapsedDays)) {
        throw std::invalid_argument(
            "Angle and elapsed time must be finite"
        );
    }

    return normalizeAngle(
        initialAngle +
        angularSpeed(orbitalPeriodDays) * elapsedDays
    );
}

// Calculate a planet's position at a given simulation time.
inline Position positionAtTime(
    double centerX,
    double centerY,
    double radiusX,
    double radiusY,
    double startingAngle,
    double elapsedDays,
    double orbitalPeriodDays
) {
    double newAngle = advanceAngle(
        startingAngle,
        elapsedDays,
        orbitalPeriodDays
    );

    return ellipticalPosition(
        centerX,
        centerY,
        radiusX,
        radiusY,
        newAngle
    );
}

} // namespace orbit

#endif // ORBIT_MATH_H
