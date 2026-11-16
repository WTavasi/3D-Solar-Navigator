// Integration tests for the Stellar application: the simulation, input
// handling and the renderer, which together use orbit_math, the Camera
// and the project's raster algorithms. No window is needed.

#include <cmath>
#include <ostream>
#include <string>
#include <vector>

#include "app/stellar_app.h"
#include "scene/solar_system_scene.h"
#include "test_framework.h"

namespace {

const int W = 1280;

std::ostream& operator<<(std::ostream& stream, BodyKind kind) {
    const char* names[] = {"Sun", "Planet", "Moon", "Comet"};
    return stream << names[static_cast<int>(kind)];
}
const int H = 800;

double distanceBetween(const Vec3& a, const Vec3& b) {
    return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) +
                     (a.z - b.z) * (a.z - b.z));
}

BodyView viewOf(const StellarApp& app, const std::string& name) {
    for (const BodyView& body : app.bodies()) {
        if (body.name == name) {
            return body;
        }
    }
    return BodyView{};
}

void runSeconds(StellarApp& app, double seconds) {
    for (double t = 0.0; t < seconds; t += 1.0 / 30.0) {
        app.update(1.0 / 30.0);
    }
}

int countPixels(const RasterCanvas& canvas, const Color& color) {
    int count = 0;
    for (int y = 0; y < canvas.getHeight(); y++) {
        for (int x = 0; x < canvas.getWidth(); x++) {
            if (canvas.getPixel(x, y) == color) {
                count++;
            }
        }
    }
    return count;
}

}  // namespace

TEST_CASE("APP-01 starts on day 0, running, with nothing selected") {
    StellarApp app(W, H);

    CHECK_NEAR(app.day(), 0.0, 1e-9);
    CHECK(!app.isPaused());
    CHECK(!app.tourActive());
    CHECK(app.selectedName().empty());
    CHECK_EQ(app.width(), W);
}

TEST_CASE("APP-02 has the Sun, eight planets, the Moon and the comet") {
    StellarApp app(W, H);
    std::vector<BodyView> bodies = app.bodies();

    CHECK_EQ(bodies.size(), static_cast<std::size_t>(11));
    CHECK_EQ(viewOf(app, "Sun").kind, BodyKind::Sun);
    CHECK_EQ(viewOf(app, "Moon").kind, BodyKind::Moon);
    CHECK_EQ(viewOf(app, "Saturn").planetIndex, 5);
}

TEST_CASE("APP-03 bodies are sorted far to near for the painter's algorithm") {
    StellarApp app(W, H);
    app.setDay(200);
    std::vector<BodyView> bodies = app.bodies();

    for (std::size_t i = 1; i < bodies.size(); i++) {
        CHECK(bodies[i - 1].screen.depth >= bodies[i].screen.depth);
    }
}

TEST_CASE("APP-04 time moves the planets along their orbits") {
    StellarApp app(W, H);
    Vec3 before = app.planetWorldPosition(2);

    runSeconds(app, 2.0);   // 5 days a second by default

    CHECK_NEAR(app.day(), 10.0, 0.3);
    CHECK(distanceBetween(before, app.planetWorldPosition(2)) > 1.0);
}

TEST_CASE("APP-05 planets keep their orbit plane and return after one period") {
    StellarApp app(W, H);
    std::vector<Planet> planets = solarSystemPlanets();
    std::vector<double> periods = solarSystemOrbitalPeriods();

    for (int i = 0; i < 8; i++) {
        app.setDay(0);
        Vec3 start = app.planetWorldPosition(i);
        app.setDay(periods[static_cast<std::size_t>(i)]);
        CHECK(distanceBetween(start, app.planetWorldPosition(i)) < 0.01);
        CHECK_NEAR(app.planetWorldPosition(i).y, 0.0, 1e-9);
    }
}

TEST_CASE("APP-06 the Moon stays the same distance from Earth as it orbits") {
    StellarApp app(W, H);
    app.setDay(0);
    double first = distanceBetween(app.moonWorldPosition(), app.planetWorldPosition(2));

    for (double day = 1; day < 400; day += 13.7) {
        app.setDay(day);
        CHECK_NEAR(distanceBetween(app.moonWorldPosition(), app.planetWorldPosition(2)),
                   first, 1e-6);
    }
}

TEST_CASE("APP-07 the comet's Bezier path is smooth and closed") {
    StellarApp app(W, H);
    app.setDay(0);
    Vec3 start = app.cometWorldPosition();
    Vec3 previous = start;
    double largestStep = 0.0;

    for (int day = 1; day <= 400; day++) {
        app.setDay(day);
        Vec3 now = app.cometWorldPosition();
        largestStep = std::max(largestStep, distanceBetween(previous, now));
        previous = now;
    }

    CHECK(distanceBetween(start, previous) < 1.0);   // back after 400 days
    CHECK(largestStep < 40.0);                        // no jumps
}

TEST_CASE("APP-08 speed steps up and down within limits") {
    StellarApp app(W, H);
    std::vector<double> steps = StellarApp::speedSteps();

    CHECK_NEAR(app.daysPerSecond(), steps[2], 1e-9);
    for (int i = 0; i < 20; i++) app.keyDown('+');
    CHECK_NEAR(app.daysPerSecond(), steps.back(), 1e-9);
    for (int i = 0; i < 20; i++) app.keyDown('-');
    CHECK_NEAR(app.daysPerSecond(), steps.front(), 1e-9);

    for (std::size_t i = 1; i < steps.size(); i++) {
        CHECK(steps[i] > steps[i - 1]);
    }
}

TEST_CASE("APP-09 space pauses time but the app keeps responding") {
    StellarApp app(W, H);
    app.keyDown(' ');
    CHECK(app.isPaused());

    runSeconds(app, 1.0);
    CHECK_NEAR(app.day(), 0.0, 1e-9);

    app.keyDown(' ');
    runSeconds(app, 1.0);
    CHECK(app.day() > 0.0);
}

TEST_CASE("APP-10 planets spin, and Venus spins the other way to Earth") {
    StellarApp app(W, H);
    runSeconds(app, 0.5);

    double earth = app.spinDegrees(2);
    double venus = app.spinDegrees(1);

    CHECK(earth > 0.0 && earth < 180.0);
    CHECK(venus > 180.0);    // a small backwards turn wraps below 360
}

TEST_CASE("APP-11 clicking a planet selects it and clicking space clears it") {
    StellarApp app(W, H);
    app.setPaused(true);
    BodyView jupiter = viewOf(app, "Jupiter");
    int x = static_cast<int>(jupiter.screen.x);
    int y = static_cast<int>(jupiter.screen.y);

    app.mouseDown(x, y);
    app.mouseUp(x, y);
    CHECK_EQ(app.selectedName(), std::string("Jupiter"));

    app.mouseDown(5, H / 2);
    app.mouseUp(5, H / 2);
    CHECK(app.selectedName().empty());
}

TEST_CASE("APP-12 dragging rotates the camera instead of selecting") {
    StellarApp app(W, H);
    double yaw = app.camera().yaw;
    BodyView sun = viewOf(app, "Sun");

    app.mouseDown(static_cast<int>(sun.screen.x), static_cast<int>(sun.screen.y));
    app.mouseMove(static_cast<int>(sun.screen.x) + 100, static_cast<int>(sun.screen.y));
    app.mouseUp(static_cast<int>(sun.screen.x) + 100, static_cast<int>(sun.screen.y));

    CHECK(std::fabs(app.camera().yaw - yaw) > 10.0);
    CHECK(app.selectedName().empty());
}

TEST_CASE("APP-13 the camera eases onto the selected body and back to the Sun") {
    StellarApp app(W, H);
    app.setPaused(true);
    app.keyDown('5');   // Jupiter
    runSeconds(app, 3.0);

    CHECK(distanceBetween(app.camera().target, app.planetWorldPosition(4)) < 1.0);
    BodyView jupiter = viewOf(app, "Jupiter");
    CHECK_NEAR(jupiter.screen.x, W / 2.0, 2.0);
    CHECK_NEAR(jupiter.screen.y, H / 2.0, 2.0);

    app.keyDown(27);
    runSeconds(app, 4.0);
    CHECK(distanceBetween(app.camera().target, {0, 0, 0}) < 0.01);
}

TEST_CASE("APP-14 number keys select bodies and unknown names are refused") {
    StellarApp app(W, H);

    app.keyDown('0');
    CHECK_EQ(app.selectedName(), std::string("Sun"));
    app.keyDown('3');
    CHECK_EQ(app.selectedName(), std::string("Earth"));
    app.keyDown('8');
    CHECK_EQ(app.selectedName(), std::string("Neptune"));
    app.keyDown('9');
    CHECK_EQ(app.selectedName(), std::string("Moon"));

    CHECK(!app.select("Pluto"));
    CHECK_EQ(app.selectedName(), std::string("Moon"));
}

TEST_CASE("APP-15 the wheel zooms and R resets the view") {
    StellarApp app(W, H);
    double start = app.camera().distance;

    app.mouseWheel(3);
    CHECK(app.camera().distance < start);
    app.mouseWheel(-6);
    CHECK(app.camera().distance > start);

    app.keyDown('d');
    CHECK(distanceBetween(app.camera().target, {0, 0, 0}) > 1.0);

    app.keyDown('r');
    CHECK_NEAR(app.camera().distance, start, 1e-9);
    CHECK(distanceBetween(app.camera().target, {0, 0, 0}) < 1e-9);
}

TEST_CASE("APP-16 the tour flies through its viewpoints and then ends") {
    StellarApp app(W, H);
    app.keyDown('f');
    CHECK(app.tourActive());

    double lowestPitch = 90.0;
    double highestPitch = -90.0;
    for (double t = 0; t < StellarApp::tourDurationSeconds() + 1.0; t += 0.1) {
        app.update(0.1);
        lowestPitch = std::min(lowestPitch, app.camera().pitch);
        highestPitch = std::max(highestPitch, app.camera().pitch);
    }

    CHECK(!app.tourActive());
    CHECK(lowestPitch < 20.0);     // dips low past the planets
    CHECK(highestPitch > 75.0);    // rises to look down from above
}

TEST_CASE("APP-17 any camera input stops the tour") {
    StellarApp app(W, H);
    app.startTour();
    app.update(1.0);
    app.mouseWheel(1);
    CHECK(!app.tourActive());
}

TEST_CASE("APP-18 renders for many days and camera angles without failing") {
    StellarApp app(640, 400);
    RasterCanvas canvas(640, 400);

    for (int day = 0; day < 3000; day += 250) {
        app.setDay(day);
        app.mouseDown(100, 100);
        app.mouseMove(100 + day % 300, 100 + day % 97);
        app.mouseUp(100 + day % 300, 100 + day % 97);
        app.render(canvas);
        CHECK(countPixels(canvas, {255, 255, 255}) < 640 * 400 / 2);
    }
}

TEST_CASE("APP-19 the Sun is drawn in warm, bright colours at the centre") {
    StellarApp app(W, H);
    RasterCanvas canvas(W, H);
    app.render(canvas);

    Color centre = canvas.getPixel(W / 2, H / 2);
    CHECK(centre.red > 200);
    CHECK(centre.green > 150);
    CHECK(centre.red >= centre.blue);   // warm, not blue
}

TEST_CASE("APP-20 the information panel is drawn for a body away from the centre") {
    StellarApp app(W, H);
    RasterCanvas canvas(W, H);
    app.setPaused(true);
    app.select("Neptune");
    app.render(canvas);   // camera has not moved yet, so Neptune is off-centre

    // The panel title is drawn in the accent colour.
    CHECK(countPixels(canvas, {255, 200, 60}) > 100);
}

TEST_CASE("APP-21 a resized app projects into the new size") {
    StellarApp app(W, H);
    app.resize(600, 600);
    BodyView sun = viewOf(app, "Sun");

    CHECK_NEAR(sun.screen.x, 300.0, 1.0);
    CHECK_NEAR(sun.screen.y, 300.0, 1.0);
}

int main() {
    return testing::runAllTests();
}
