// Unit tests for the 3D camera in scene/camera.h and src/camera/camera.cpp:
// its position, the three transformations (rotate, zoom, pan) and
// perspective projection.
//
// These use the project's CHECK macros, so they still run in Release
// builds (assert() is removed there).

#include <cmath>

#include "scene/camera.h"
#include "test_framework.h"

namespace {

const int WIDTH = 1200;
const int HEIGHT = 800;
const double PI = 3.14159265358979323846;

double distanceBetween(const Vec3& a, const Vec3& b) {
    return std::sqrt((a.x - b.x) * (a.x - b.x) +
                     (a.y - b.y) * (a.y - b.y) +
                     (a.z - b.z) * (a.z - b.z));
}

// A camera looking straight along -z at the origin from (0, 0, 1000).
Camera straightOnCamera() {
    Camera camera;
    camera.yaw = 0.0;
    camera.pitch = 0.0;
    camera.distance = 1000.0;
    camera.fov = 60.0;
    return camera;
}

// Focal length in pixels for a 60 degree field of view.
double focalLength() {
    return (HEIGHT / 2.0) / std::tan(30.0 * PI / 180.0);
}

}  // namespace

TEST_CASE("CAM-01 the target projects to the centre of the screen") {
    Camera camera;
    ScreenPoint sun = camera.project({0, 0, 0}, WIDTH, HEIGHT);

    CHECK(sun.visible);
    CHECK_NEAR(sun.x, 600.0, 1e-6);
    CHECK_NEAR(sun.y, 400.0, 1e-6);
    CHECK_NEAR(sun.depth, camera.distance, 1e-6);
}

TEST_CASE("CAM-02 the camera position follows yaw, pitch and distance") {
    Camera camera = straightOnCamera();

    Vec3 front = camera.getPosition();
    CHECK_NEAR(front.x, 0.0, 1e-9);
    CHECK_NEAR(front.y, 0.0, 1e-9);
    CHECK_NEAR(front.z, 1000.0, 1e-9);

    camera.yaw = 90.0;
    Vec3 side = camera.getPosition();
    CHECK_NEAR(side.x, 1000.0, 1e-9);
    CHECK_NEAR(side.z, 0.0, 1e-9);

    camera.yaw = 0.0;
    camera.pitch = 30.0;
    Vec3 raised = camera.getPosition();
    CHECK_NEAR(raised.y, 500.0, 1e-9);                     // 1000 sin 30
    CHECK_NEAR(raised.z, 1000.0 * std::cos(PI / 6), 1e-9);

    // Whatever the angles, the camera stays at its distance from the target.
    camera.yaw = 123.0;
    camera.pitch = -41.0;
    CHECK_NEAR(distanceBetween(camera.getPosition(), camera.target), 1000.0, 1e-9);
}

TEST_CASE("CAM-03 projection matches the hand-worked perspective formula") {
    // focal = 400 / tan 30 = 692.82; x' = 600 + focal * 100 / 1000.
    Camera camera = straightOnCamera();

    ScreenPoint right = camera.project({100, 0, 0}, WIDTH, HEIGHT);
    CHECK(right.visible);
    CHECK_NEAR(right.x, 600.0 + focalLength() * 0.1, 1e-6);
    CHECK_NEAR(right.x, 669.282, 1e-3);
    CHECK_NEAR(right.y, 400.0, 1e-6);

    // Up in the world is up on the screen (smaller y).
    ScreenPoint up = camera.project({0, 100, 0}, WIDTH, HEIGHT);
    CHECK_NEAR(up.x, 600.0, 1e-6);
    CHECK_NEAR(up.y, 400.0 - focalLength() * 0.1, 1e-6);
}

TEST_CASE("CAM-04 halving the depth doubles the distance from the centre") {
    Camera camera = straightOnCamera();

    ScreenPoint far = camera.project({50, 0, 0}, WIDTH, HEIGHT);      // depth 1000
    ScreenPoint near = camera.project({50, 0, 500}, WIDTH, HEIGHT);   // depth 500

    CHECK_NEAR(near.depth, 500.0, 1e-6);
    CHECK_NEAR((near.x - 600.0) / (far.x - 600.0), 2.0, 1e-9);
}

TEST_CASE("CAM-05 points behind or too close to the camera are not visible") {
    Camera camera = straightOnCamera();

    CHECK(!camera.project({0, 0, 2000}, WIDTH, HEIGHT).visible);   // behind
    CHECK(!camera.project({0, 0, 1000}, WIDTH, HEIGHT).visible);   // at the camera
    CHECK(!camera.project({0, 0, 999.5}, WIDTH, HEIGHT).visible);  // inside the near plane
    CHECK(camera.project({0, 0, 998.0}, WIDTH, HEIGHT).visible);   // just past it
}

TEST_CASE("CAM-06 points that land off the screen are not visible") {
    Camera camera = straightOnCamera();

    CHECK(!camera.project({5000, 0, 0}, WIDTH, HEIGHT).visible);
    CHECK(!camera.project({0, -5000, 0}, WIDTH, HEIGHT).visible);
}

TEST_CASE("CAM-07 an invalid screen size or field of view projects nothing") {
    Camera camera;

    CHECK(!camera.project({0, 0, 0}, 0, HEIGHT).visible);
    CHECK(!camera.project({0, 0, 0}, WIDTH, -5).visible);

    camera.fov = 0.0;
    CHECK(!camera.project({0, 0, 0}, WIDTH, HEIGHT).visible);

    camera.fov = 180.0;
    CHECK(!camera.project({0, 0, 0}, WIDTH, HEIGHT).visible);
}

TEST_CASE("CAM-08 zoom scales the distance within 20 to 10,000") {
    Camera camera;

    camera.zoom(0.5);
    CHECK_NEAR(camera.distance, 500.0, 1e-9);

    camera.zoom(1e-9);
    CHECK_NEAR(camera.distance, 20.0, 1e-9);

    camera.zoom(1e12);
    CHECK_NEAR(camera.distance, 10000.0, 1e-9);

    // Zero or negative factors are ignored.
    camera.zoom(0.0);
    camera.zoom(-2.0);
    CHECK_NEAR(camera.distance, 10000.0, 1e-9);
}

TEST_CASE("CAM-09 pitch stops at plus and minus 89 degrees; yaw does not") {
    Camera camera;

    camera.rotate(0.0, 500.0);
    CHECK_NEAR(camera.pitch, 89.0, 1e-9);

    camera.rotate(0.0, -500.0);
    CHECK_NEAR(camera.pitch, -89.0, 1e-9);

    camera.rotate(400.0, 0.0);
    CHECK_NEAR(camera.yaw, 400.0, 1e-9);
}

TEST_CASE("CAM-10 panning moves the target along the camera's right and up axes") {
    Camera camera = straightOnCamera();

    camera.pan(50.0, 0.0);
    CHECK_NEAR(camera.target.x, 50.0, 1e-9);
    CHECK_NEAR(camera.target.y, 0.0, 1e-9);
    CHECK_NEAR(camera.target.z, 0.0, 1e-9);

    camera.pan(0.0, 30.0);
    CHECK_NEAR(camera.target.y, 30.0, 1e-9);

    // The camera moves with its target, so the distance between them
    // is unchanged and the target still projects to the centre.
    CHECK_NEAR(distanceBetween(camera.getPosition(), camera.target), 1000.0, 1e-9);
    ScreenPoint centre = camera.project(camera.target, WIDTH, HEIGHT);
    CHECK_NEAR(centre.x, 600.0, 1e-6);
    CHECK_NEAR(centre.y, 400.0, 1e-6);
}

TEST_CASE("CAM-11 reset restores every camera setting") {
    Camera camera;

    camera.rotate(40.0, 20.0);
    camera.zoom(0.3);
    camera.pan(80.0, -15.0);
    camera.fov = 90.0;
    camera.reset();

    Camera fresh;
    CHECK_NEAR(camera.yaw, fresh.yaw, 1e-9);
    CHECK_NEAR(camera.pitch, fresh.pitch, 1e-9);
    CHECK_NEAR(camera.distance, fresh.distance, 1e-9);
    CHECK_NEAR(camera.fov, fresh.fov, 1e-9);
    CHECK_NEAR(camera.target.x, 0.0, 1e-9);
    CHECK_NEAR(camera.target.y, 0.0, 1e-9);
    CHECK_NEAR(camera.target.z, 0.0, 1e-9);
}

int main() {
    return testing::runAllTests();
}
