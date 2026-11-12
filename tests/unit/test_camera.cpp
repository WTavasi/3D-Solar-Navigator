
#include "scene/camera.h"

#include <cassert>
#include <cmath>
#include <iostream>

bool approximatelyEqual(
    double a,
    double b,
    double tolerance = 0.001
) {
    return std::abs(a - b) < tolerance;
}

int main() {
    Camera camera;

    // Test 1: The Sun at the origin should
    // appear at the center of the screen.
    ScreenPoint sun = camera.project(
        {0.0, 0.0, 0.0}, 1200, 800
    );

    std::cout << "Sun X: " << sun.x << '\n';
    std::cout << "Sun Y: " << sun.y << '\n';
    std::cout << "Visible: " << sun.visible << '\n';
    std::cout << "Depth: " << sun.depth << '\n';

    assert(sun.visible);
    assert(approximatelyEqual(sun.x, 600.0));
    assert(approximatelyEqual(sun.y, 400.0));

    // Test 2: Zoom in decreases camera distance.
    double originalDistance = camera.distance;
    camera.zoom(0.5);

    assert(camera.distance < originalDistance);

    // Test 3: Rotation changes the camera position.
    Vec3 before = camera.getPosition();
    camera.rotate(30.0, 10.0);
    Vec3 after = camera.getPosition();

    assert(!approximatelyEqual(before.x, after.x));

    // Test 4: Pitch is constrained.
    camera.rotate(0.0, 200.0);
    assert(camera.pitch <= 89.0);

    // Test 5: Panning changes the target.
    Vec3 oldTarget = camera.target;
    camera.pan(50.0, 20.0);

    assert(
        !approximatelyEqual(oldTarget.x, camera.target.x) ||
        !approximatelyEqual(oldTarget.y, camera.target.y) ||
        !approximatelyEqual(oldTarget.z, camera.target.z)
    );

    // Test 6: Reset restores the default camera.
    camera.reset();

    assert(approximatelyEqual(camera.distance, 1000.0));
    assert(approximatelyEqual(camera.yaw, 0.0));
    assert(approximatelyEqual(camera.pitch, 35.0));

    // Test 7: Perspective makes closer objects
    // appear farther from the screen center.
    Camera perspectiveCamera;
    perspectiveCamera.pitch = 0.0;
    perspectiveCamera.distance = 1000.0;

    ScreenPoint nearPoint = perspectiveCamera.project(
        {100.0, 0.0, 500.0}, 1200, 800
    );

    ScreenPoint farPoint = perspectiveCamera.project(
        {100.0, 0.0, -500.0}, 1200, 800
    );

    assert(nearPoint.visible);
    assert(farPoint.visible);
    assert(
        std::abs(nearPoint.x - 600.0) >
        std::abs(farPoint.x - 600.0)
    );

    std::cout << "All camera tests passed!\n";
    return 0;
}
