
#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>

#include "algorithms/orbit/orbit_math.h"

// Helper to compare decimal values.
bool nearlyEqual(double a, double b) {
    return std::abs(a - b) < 0.000001;
}

int main() {
    // Test 1: Degrees to radians
    assert(nearlyEqual(
        orbit::degreesToRadians(180.0),
        orbit::PI
    ));

    // Test 2: Angle normalization
    assert(nearlyEqual(
        orbit::normalizeAngle(450.0),
        90.0
    ));

    assert(nearlyEqual(
        orbit::normalizeAngle(-90.0),
        270.0
    ));

    // Test 3: Elliptical coordinates at 0 degrees
    orbit::Position p1 = orbit::ellipticalPosition(
        100.0, 100.0, 50.0, 30.0, 0.0
    );

    assert(nearlyEqual(p1.x, 150.0));
    assert(nearlyEqual(p1.y, 100.0));

    // Test 4: Elliptical coordinates at 90 degrees
    orbit::Position p2 = orbit::ellipticalPosition(
        100.0, 100.0, 50.0, 30.0, 90.0
    );

    assert(nearlyEqual(p2.x, 100.0));
    assert(nearlyEqual(p2.y, 130.0));

    // Test 5: Half an orbital period
    assert(nearlyEqual(
        orbit::advanceAngle(0.0, 50.0, 100.0),
        180.0
    ));

    // Test 6: Full orbital period
    assert(nearlyEqual(
        orbit::advanceAngle(45.0, 100.0, 100.0),
        45.0
    ));

    // Test 7: Position at half an orbit
    orbit::Position p3 = orbit::positionAtTime(
        100.0, 100.0,
        50.0, 30.0,
        0.0, 50.0, 100.0
    );

    assert(nearlyEqual(p3.x, 50.0));
    assert(nearlyEqual(p3.y, 100.0));

    // Test 8: Reject an invalid orbital period
    bool exceptionCaught = false;

    try {
        orbit::angularSpeed(0.0);
    }
    catch (const std::invalid_argument&) {
        exceptionCaught = true;
    }

    assert(exceptionCaught);

    std::cout
        << "All orbit mathematics tests passed!"
        << std::endl;

    return 0;
}
