// Unit tests for the orbit mathematics in algorithms/orbit/orbit_math.h
// and the orbital periods used by the Solar System scene.
//
// These use the project's CHECK macros, so they still run in Release
// builds (assert() is removed there).

#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

#include "algorithms/orbit/orbit_math.h"
#include "scene/solar_system_scene.h"
#include "test_framework.h"

namespace {

const double TOLERANCE = 1e-9;

bool throwsInvalidArgument(double initialAngle, double elapsedDays,
                           double periodDays) {
    try {
        orbit::advanceAngle(initialAngle, elapsedDays, periodDays);
    } catch (const std::invalid_argument&) {
        return true;
    }

    return false;
}

}  // namespace

TEST_CASE("ORB-01 angles convert to radians and wrap into 0 to 360") {
    CHECK_NEAR(orbit::degreesToRadians(180.0), orbit::PI, TOLERANCE);
    CHECK_NEAR(orbit::degreesToRadians(90.0), orbit::PI / 2.0, TOLERANCE);

    CHECK_NEAR(orbit::normalizeAngle(0.0), 0.0, TOLERANCE);
    CHECK_NEAR(orbit::normalizeAngle(360.0), 0.0, TOLERANCE);
    CHECK_NEAR(orbit::normalizeAngle(450.0), 90.0, TOLERANCE);
    CHECK_NEAR(orbit::normalizeAngle(720.0), 0.0, TOLERANCE);
    CHECK_NEAR(orbit::normalizeAngle(-90.0), 270.0, TOLERANCE);
    CHECK_NEAR(orbit::normalizeAngle(-450.0), 270.0, TOLERANCE);
}

TEST_CASE("ORB-02 elliptical positions at the four quarter angles") {
    const double cx = 100, cy = 50, rx = 40, ry = 25;

    orbit::Position right = orbit::ellipticalPosition(cx, cy, rx, ry, 0);
    orbit::Position down = orbit::ellipticalPosition(cx, cy, rx, ry, 90);
    orbit::Position left = orbit::ellipticalPosition(cx, cy, rx, ry, 180);
    orbit::Position up = orbit::ellipticalPosition(cx, cy, rx, ry, 270);

    CHECK_NEAR(right.x, 140, 1e-9);
    CHECK_NEAR(right.y, 50, 1e-9);
    CHECK_NEAR(down.x, 100, 1e-9);
    CHECK_NEAR(down.y, 75, 1e-9);
    CHECK_NEAR(left.x, 60, 1e-9);
    CHECK_NEAR(left.y, 50, 1e-9);
    CHECK_NEAR(up.x, 100, 1e-9);
    CHECK_NEAR(up.y, 25, 1e-9);
}

TEST_CASE("ORB-03 every elliptical position lies on its ellipse") {
    for (int angle = 0; angle < 360; angle += 7) {
        orbit::Position p = orbit::ellipticalPosition(600, 400, 340, 220, angle);
        double dx = (p.x - 600) / 340.0;
        double dy = (p.y - 400) / 220.0;

        CHECK_NEAR(dx * dx + dy * dy, 1.0, 1e-9);
    }
}

TEST_CASE("ORB-04 the angle advances by 360 degrees per orbital period") {
    CHECK_NEAR(orbit::angularSpeed(100.0), 3.6, TOLERANCE);
    CHECK_NEAR(orbit::advanceAngle(0.0, 25.0, 100.0), 90.0, TOLERANCE);
    CHECK_NEAR(orbit::advanceAngle(0.0, 50.0, 100.0), 180.0, TOLERANCE);
    CHECK_NEAR(orbit::advanceAngle(45.0, 100.0, 100.0), 45.0, 1e-6);
    CHECK_NEAR(orbit::advanceAngle(45.0, 250.0, 100.0), 225.0, 1e-6);
}

TEST_CASE("ORB-05 negative elapsed time runs the orbit backwards") {
    // 10 degrees, minus a quarter orbit (90 degrees), wraps to 280.
    CHECK_NEAR(orbit::advanceAngle(10.0, -25.0, 100.0), 280.0, 1e-9);
    CHECK_NEAR(orbit::advanceAngle(30.0, -100.0, 100.0), 30.0, 1e-6);
}

TEST_CASE("ORB-06 invalid periods and times are rejected") {
    const double infinity = std::numeric_limits<double>::infinity();
    const double notANumber = std::numeric_limits<double>::quiet_NaN();

    CHECK(throwsInvalidArgument(0.0, 10.0, 0.0));
    CHECK(throwsInvalidArgument(0.0, 10.0, -365.0));
    CHECK(throwsInvalidArgument(0.0, 10.0, infinity));
    CHECK(throwsInvalidArgument(0.0, 10.0, notANumber));
    CHECK(throwsInvalidArgument(notANumber, 10.0, 365.0));
    CHECK(throwsInvalidArgument(0.0, infinity, 365.0));
    CHECK(!throwsInvalidArgument(0.0, 10.0, 365.0));
}

TEST_CASE("ORB-07 positionAtTime equals advancing the angle then placing it") {
    for (double day : {0.0, 17.5, 88.0, 400.0, -30.0}) {
        orbit::Position direct = orbit::positionAtTime(600, 400, 210, 135,
                                                       145, day, 365.25);
        double angle = orbit::advanceAngle(145, day, 365.25);
        orbit::Position stepwise = orbit::ellipticalPosition(600, 400, 210,
                                                             135, angle);

        CHECK_NEAR(direct.x, stepwise.x, TOLERANCE);
        CHECK_NEAR(direct.y, stepwise.y, TOLERANCE);
    }
}

TEST_CASE("ORB-08 there is one orbital period for every planet") {
    // renderSolarSystem reads periods[i] for each planet, so a planet
    // added without a period would read past the end of the list.
    CHECK_EQ(solarSystemOrbitalPeriods().size(), solarSystemPlanets().size());
}

TEST_CASE("ORB-09 periods follow the real Solar System") {
    std::vector<double> periods = solarSystemOrbitalPeriods();

    for (std::size_t i = 1; i < periods.size(); i++) {
        CHECK(periods[i] > periods[i - 1]);   // outer planets are slower
    }

    if (periods.size() == 8) {
        CHECK_NEAR(periods[2], 365.25, 1e-9);   // Earth: one year
        CHECK(periods[0] > 87 && periods[0] < 89);        // Mercury
        CHECK(periods[7] > 60000 && periods[7] < 60300);  // Neptune
    }
}

TEST_CASE("ORB-10 each planet is back at its start after one full period") {
    std::vector<Planet> planets = solarSystemPlanets();
    std::vector<double> periods = solarSystemOrbitalPeriods();

    for (std::size_t i = 0; i < planets.size() && i < periods.size(); i++) {
        double after = orbit::advanceAngle(planets[i].angle, periods[i],
                                           periods[i]);
        double difference = std::fabs(after - orbit::normalizeAngle(planets[i].angle));

        // 359.999... and 0 are the same angle.
        CHECK(difference < 1e-6 || std::fabs(difference - 360.0) < 1e-6);
    }
}

TEST_CASE("ORB-11 orbit maths and the scene place planets at the same pixel") {
    // planetPosition rounds towards zero, so they agree to within 1 pixel.
    for (const Planet& planet : solarSystemPlanets()) {
        Pixel scene = planetPosition(planet, 600, 400);
        orbit::Position exact = orbit::ellipticalPosition(
            600, 400, planet.orbitX, planet.orbitY, planet.angle);

        CHECK(std::fabs(scene.x - exact.x) < 1.0);
        CHECK(std::fabs(scene.y - exact.y) < 1.0);
    }
}

int main() {
    return testing::runAllTests();
}
