// Integration tests for the Solar System scene: the scene data, the
// layout and the rendered image, which together use the raster canvas,
// Bresenham lines and the midpoint circle and ellipse algorithms.
//
// The project models the real Solar System, so the scene is checked
// against real facts (planet order and relative size), not real scale.

#include <cmath>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "scene/solar_system_scene.h"
#include "test_framework.h"

namespace {

const int SUN_X = SOLAR_SYSTEM_WIDTH / 2;
const int SUN_Y = SOLAR_SYSTEM_HEIGHT / 2;
const Color BACKGROUND = {255, 255, 255};

std::ostream& operator<<(std::ostream& stream, const Color& color) {
    return stream << "rgb(" << static_cast<int>(color.red) << ", "
                  << static_cast<int>(color.green) << ", "
                  << static_cast<int>(color.blue) << ")";
}

const Planet& findPlanet(const std::vector<Planet>& planets,
                         const std::string& name) {
    for (const Planet& planet : planets) {
        if (name == planet.name) {
            return planet;
        }
    }

    static const Planet missing = {"missing", 0, 0, 0, 0, {0, 0, 0}, false};
    return missing;
}

// How far a body reaches from its centre, including any rings.
int outerRadius(const Planet& planet) {
    return planet.hasRings ? ringRadiusX(planet) : planet.radius;
}

double distanceBetween(const Pixel& first, const Pixel& second) {
    return std::hypot(first.x - second.x, first.y - second.y);
}

RasterCanvas renderedScene() {
    RasterCanvas canvas(SOLAR_SYSTEM_WIDTH, SOLAR_SYSTEM_HEIGHT);
    renderSolarSystem(canvas);
    return canvas;
}

}  // namespace

TEST_CASE("SCN-01 the eight planets are listed in order from the Sun") {
    const std::vector<std::string> expected = {
        "Mercury", "Venus", "Earth", "Mars",
        "Jupiter", "Saturn", "Uranus", "Neptune"
    };

    std::vector<std::string> actual;
    for (const Planet& planet : solarSystemPlanets()) {
        actual.push_back(planet.name);
    }

    CHECK_EQ(actual, expected);
}

TEST_CASE("SCN-02 each orbit is larger than the one inside it, so none cross") {
    std::vector<Planet> planets = solarSystemPlanets();

    for (std::size_t i = 1; i < planets.size(); i++) {
        CHECK(planets[i].orbitX > planets[i - 1].orbitX);
        CHECK(planets[i].orbitY > planets[i - 1].orbitY);
    }

    // The innermost orbit clears the Sun and its rays.
    CHECK(planets.front().orbitY - planets.front().radius > SUN_RAY_OUTER_RADIUS);
}

TEST_CASE("SCN-03 planet sizes follow the real Solar System ranking") {
    std::vector<Planet> planets = solarSystemPlanets();
    auto size = [&](const char* name) { return findPlanet(planets, name).radius; };

    // Gas and ice giants are all larger than every rocky planet.
    for (const char* giant : {"Jupiter", "Saturn", "Uranus", "Neptune"}) {
        for (const char* rocky : {"Mercury", "Venus", "Earth", "Mars"}) {
            CHECK(size(giant) > size(rocky));
        }
    }

    CHECK(size("Jupiter") > size("Saturn"));
    CHECK(size("Saturn") > size("Uranus"));
    CHECK(size("Uranus") >= size("Neptune"));   // 25,362 km vs 24,622 km
    CHECK(size("Earth") > size("Venus"));
    CHECK(size("Venus") > size("Mars"));
    CHECK(size("Mars") > size("Mercury"));
}

TEST_CASE("SCN-04 every planet has its own colour, different from the scenery") {
    std::vector<Planet> planets = solarSystemPlanets();

    for (std::size_t i = 0; i < planets.size(); i++) {
        CHECK(planets[i].color != BACKGROUND);
        CHECK(planets[i].color != ORBIT_COLOR);
        CHECK(planets[i].color != SUN_COLOR);

        for (std::size_t j = i + 1; j < planets.size(); j++) {
            CHECK(planets[i].color != planets[j].color);
        }
    }
}

TEST_CASE("SCN-05 Saturn is the only planet drawn with rings") {
    for (const Planet& planet : solarSystemPlanets()) {
        CHECK_EQ(planet.hasRings, std::string(planet.name) == "Saturn");
    }
}

TEST_CASE("SCN-06 each planet's centre sits on its own orbit") {
    for (const Planet& planet : solarSystemPlanets()) {
        Pixel center = planetPosition(planet, SUN_X, SUN_Y);
        double nearest = 1e9;

        for (const auto& orbitPixel :
             drawMidpointEllipse(SUN_X, SUN_Y, planet.orbitX, planet.orbitY)) {
            nearest = std::min(nearest,
                               std::hypot(center.x - orbitPixel.first,
                                          center.y - orbitPixel.second));
        }

        // Rounding the position to whole pixels can move it by up to
        // about one pixel diagonally.
        CHECK(nearest <= 1.5);
    }
}

TEST_CASE("SCN-07 the whole scene fits on the canvas") {
    for (const Planet& planet : solarSystemPlanets()) {
        CHECK(SUN_X - planet.orbitX >= 0 && SUN_X + planet.orbitX < SOLAR_SYSTEM_WIDTH);
        CHECK(SUN_Y - planet.orbitY >= 0 && SUN_Y + planet.orbitY < SOLAR_SYSTEM_HEIGHT);

        Pixel center = planetPosition(planet, SUN_X, SUN_Y);
        int reach = outerRadius(planet);

        CHECK(center.x - reach >= 0 && center.x + reach < SOLAR_SYSTEM_WIDTH);
        CHECK(center.y - reach >= 0 && center.y + reach < SOLAR_SYSTEM_HEIGHT);
    }
}

TEST_CASE("SCN-08 no planet overlaps the Sun or another planet") {
    std::vector<Planet> planets = solarSystemPlanets();
    const Pixel sun = {SUN_X, SUN_Y};

    for (std::size_t i = 0; i < planets.size(); i++) {
        Pixel first = planetPosition(planets[i], SUN_X, SUN_Y);

        CHECK(distanceBetween(first, sun) >
              SUN_RAY_OUTER_RADIUS + outerRadius(planets[i]));

        for (std::size_t j = i + 1; j < planets.size(); j++) {
            Pixel second = planetPosition(planets[j], SUN_X, SUN_Y);

            CHECK(distanceBetween(first, second) >
                  outerRadius(planets[i]) + outerRadius(planets[j]));
        }
    }
}

TEST_CASE("SCN-09 the rendered image shows the Sun, every planet and Saturn's rings") {
    RasterCanvas canvas = renderedScene();

    CHECK_EQ(canvas.getPixel(SUN_X + SUN_RADIUS, SUN_Y), SUN_COLOR);
    CHECK_EQ(canvas.getPixel(SUN_X, SUN_Y), BACKGROUND);   // outline only

    for (const Planet& planet : solarSystemPlanets()) {
        Pixel center = planetPosition(planet, SUN_X, SUN_Y);

        // The leftmost point of the planet's outline is never covered by
        // its rings, which are wider.
        CHECK_EQ(canvas.getPixel(center.x - planet.radius, center.y),
                 planet.color);

        if (planet.hasRings) {
            CHECK_EQ(canvas.getPixel(center.x + ringRadiusX(planet), center.y),
                     RING_COLOR);
        }
    }
}

TEST_CASE("SCN-10 every orbit is visible where no body is drawn over it") {
    RasterCanvas canvas = renderedScene();
    std::vector<Planet> planets = solarSystemPlanets();

    for (const Planet& planet : planets) {
        // The leftmost and rightmost points of the orbit.
        for (int side : {-1, 1}) {
            Pixel orbitPoint = {SUN_X + side * planet.orbitX, SUN_Y};
            bool covered = false;

            for (const Planet& other : planets) {
                Pixel center = planetPosition(other, SUN_X, SUN_Y);
                if (distanceBetween(orbitPoint, center) <= outerRadius(other) + 1) {
                    covered = true;
                }
            }

            if (!covered) {
                CHECK_EQ(canvas.getPixel(orbitPoint.x, orbitPoint.y), ORBIT_COLOR);
            }
        }
    }
}

TEST_CASE("SCN-11 rendering is repeatable and leaves the corners empty") {
    RasterCanvas first = renderedScene();
    RasterCanvas second = renderedScene();
    int differences = 0;

    for (int y = 0; y < SOLAR_SYSTEM_HEIGHT; y++) {
        for (int x = 0; x < SOLAR_SYSTEM_WIDTH; x++) {
            if (first.getPixel(x, y) != second.getPixel(x, y)) {
                differences++;
            }
        }
    }

    CHECK_EQ(differences, 0);
    CHECK_EQ(first.getPixel(0, 0), BACKGROUND);
    CHECK_EQ(first.getPixel(SOLAR_SYSTEM_WIDTH - 1, 0), BACKGROUND);
    CHECK_EQ(first.getPixel(0, SOLAR_SYSTEM_HEIGHT - 1), BACKGROUND);
    CHECK_EQ(first.getPixel(SOLAR_SYSTEM_WIDTH - 1, SOLAR_SYSTEM_HEIGHT - 1),
             BACKGROUND);
}

// ---------------------------------------------------------------------
// The scene over time (orbital periods) and the filled planet bodies.
// ---------------------------------------------------------------------

namespace {

const Color BELT_COLOR = {140, 140, 150};

// The planets with their angles advanced by elapsedDays, the same way
// renderSolarSystem() moves them.
std::vector<Planet> planetsAtDay(double elapsedDays) {
    std::vector<Planet> planets = solarSystemPlanets();
    std::vector<double> periods = solarSystemOrbitalPeriods();

    for (std::size_t i = 0; i < planets.size() && i < periods.size(); i++) {
        planets[i].angle = orbit::advanceAngle(planets[i].angle, elapsedDays,
                                               periods[i]);
    }

    return planets;
}

RasterCanvas renderedSceneAtDay(double elapsedDays) {
    RasterCanvas canvas(SOLAR_SYSTEM_WIDTH, SOLAR_SYSTEM_HEIGHT);
    renderSolarSystem(canvas, elapsedDays);
    return canvas;
}

double orbitValue(int x, int y, int radiusX, int radiusY) {
    double dx = static_cast<double>(x - SUN_X) / radiusX;
    double dy = static_cast<double>(y - SUN_Y) / radiusY;
    return dx * dx + dy * dy;
}

}  // namespace

TEST_CASE("SCN-12 over a whole Neptune orbit nothing overlaps or leaves the canvas") {
    // Every day for 60,190 days. Jupiter and Saturn's rings come closest,
    // within about a pixel, so this guards against any change in sizes
    // or orbits that would make bodies collide.
    int overlaps = 0;
    int offCanvas = 0;
    const Pixel sun = {SUN_X, SUN_Y};

    for (int day = 0; day <= 60190; day++) {
        std::vector<Planet> planets = planetsAtDay(day);
        std::vector<Pixel> centers;

        for (const Planet& planet : planets) {
            centers.push_back(planetPosition(planet, SUN_X, SUN_Y));
        }

        for (std::size_t i = 0; i < planets.size(); i++) {
            int reach = outerRadius(planets[i]);

            if (centers[i].x - reach < 0 || centers[i].x + reach >= SOLAR_SYSTEM_WIDTH ||
                centers[i].y - reach < 0 || centers[i].y + reach >= SOLAR_SYSTEM_HEIGHT) {
                offCanvas++;
            }

            if (distanceBetween(centers[i], sun) <= SUN_RAY_OUTER_RADIUS + reach) {
                overlaps++;
            }

            for (std::size_t j = i + 1; j < planets.size(); j++) {
                if (distanceBetween(centers[i], centers[j]) <=
                    reach + outerRadius(planets[j])) {
                    overlaps++;
                }
            }
        }
    }

    CHECK_EQ(overlaps, 0);
    CHECK_EQ(offCanvas, 0);
}

TEST_CASE("SCN-13 at any time each planet stays on its own orbit") {
    std::vector<Planet> planets = solarSystemPlanets();
    std::vector<std::set<std::pair<int, int>>> orbits;

    for (const Planet& planet : planets) {
        orbits.push_back(drawMidpointEllipse(SUN_X, SUN_Y,
                                             planet.orbitX, planet.orbitY));
    }

    double worst = 0.0;

    for (int day = 0; day <= 60190; day += 97) {
        std::vector<Planet> moved = planetsAtDay(day);

        for (std::size_t i = 0; i < moved.size(); i++) {
            Pixel center = planetPosition(moved[i], SUN_X, SUN_Y);
            double nearest = 1e9;

            for (const auto& orbitPixel : orbits[i]) {
                nearest = std::min(nearest,
                                   std::hypot(center.x - orbitPixel.first,
                                              center.y - orbitPixel.second));
            }

            worst = std::max(worst, nearest);
        }
    }

    CHECK(worst <= 1.5);
}

TEST_CASE("SCN-14 the rendered image moves each planet to its new position") {
    const double day = 100.0;
    RasterCanvas canvas = renderedSceneAtDay(day);

    for (const Planet& planet : planetsAtDay(day)) {
        Pixel center = planetPosition(planet, SUN_X, SUN_Y);

        // The centre of a filled planet is its own colour.
        CHECK_EQ(canvas.getPixel(center.x, center.y), planet.color);
    }

    // Neptune takes 60,190 days to go round once, so in 100 days it
    // moves less than one degree.
    std::vector<Planet> start = solarSystemPlanets();
    std::vector<Planet> later = planetsAtDay(day);
    double neptuneMove = std::fabs(later.back().angle - start.back().angle);
    CHECK(neptuneMove < 1.0);
}

TEST_CASE("SCN-15 day 0 is the default render and a year brings Earth back") {
    RasterCanvas byDefault(SOLAR_SYSTEM_WIDTH, SOLAR_SYSTEM_HEIGHT);
    renderSolarSystem(byDefault);
    RasterCanvas dayZero = renderedSceneAtDay(0.0);

    int differences = 0;
    for (int y = 0; y < SOLAR_SYSTEM_HEIGHT; y++) {
        for (int x = 0; x < SOLAR_SYSTEM_WIDTH; x++) {
            if (byDefault.getPixel(x, y) != dayZero.getPixel(x, y)) {
                differences++;
            }
        }
    }
    CHECK_EQ(differences, 0);

    // Keep the lists alive: findPlanet returns a reference into them.
    const std::vector<Planet> startPlanets = solarSystemPlanets();
    const std::vector<Planet> yearLaterPlanets = planetsAtDay(365.25);
    const Planet& earthStart = findPlanet(startPlanets, "Earth");
    const Planet& earthAfterYear = findPlanet(yearLaterPlanets, "Earth");
    Pixel before = planetPosition(earthStart, SUN_X, SUN_Y);
    Pixel after = planetPosition(earthAfterYear, SUN_X, SUN_Y);
    CHECK(std::abs(before.x - after.x) <= 1 && std::abs(before.y - after.y) <= 1);
}

TEST_CASE("SCN-16 planets are solid discs of exactly their radius") {
    RasterCanvas canvas = renderedScene();
    int wrongInside = 0;
    int spillOutside = 0;

    for (const Planet& planet : solarSystemPlanets()) {
        Pixel center = planetPosition(planet, SUN_X, SUN_Y);
        int r = planet.radius;
        Pixel highlight = {center.x - r / 3, center.y - r / 3};

        for (int dy = -r - 3; dy <= r + 3; dy++) {
            for (int dx = -r - 3; dx <= r + 3; dx++) {
                Color pixel = canvas.getPixel(center.x + dx, center.y + dy);
                bool inside = dx * dx + dy * dy <= r * r;
                bool isHighlight = center.x + dx == highlight.x &&
                                   center.y + dy == highlight.y;

                if (inside && !isHighlight && pixel != planet.color &&
                    !(planet.hasRings && pixel == RING_COLOR)) {
                    wrongInside++;
                }

                if (!inside && pixel == planet.color) {
                    spillOutside++;
                }
            }
        }
    }

    CHECK_EQ(wrongInside, 0);
    CHECK_EQ(spillOutside, 0);
}

TEST_CASE("SCN-17 each planet's highlight is white and sits inside the planet") {
    RasterCanvas canvas = renderedScene();

    for (const Planet& planet : solarSystemPlanets()) {
        Pixel center = planetPosition(planet, SUN_X, SUN_Y);
        int offset = planet.radius / 3;

        CHECK(2 * offset * offset <= planet.radius * planet.radius);
        CHECK_EQ(canvas.getPixel(center.x - offset, center.y - offset),
                 HIGHLIGHT_COLOR);
        CHECK(HIGHLIGHT_COLOR != planet.color);
    }
}

TEST_CASE("SCN-18 the 90 asteroid belt points lie between Mars and Jupiter") {
    RasterCanvas canvas = renderedScene();
    const std::vector<Planet> planets = solarSystemPlanets();
    const Planet& mars = findPlanet(planets, "Mars");
    const Planet& jupiter = findPlanet(planets, "Jupiter");
    int beltPixels = 0;
    int misplaced = 0;

    for (int y = 0; y < SOLAR_SYSTEM_HEIGHT; y++) {
        for (int x = 0; x < SOLAR_SYSTEM_WIDTH; x++) {
            if (canvas.getPixel(x, y) != BELT_COLOR) {
                continue;
            }

            beltPixels++;

            if (orbitValue(x, y, mars.orbitX, mars.orbitY) <= 1.0 ||
                orbitValue(x, y, jupiter.orbitX, jupiter.orbitY) >= 1.0) {
                misplaced++;
            }
        }
    }

    CHECK_EQ(beltPixels, 90);   // one point every 4 degrees
    CHECK_EQ(misplaced, 0);
}

TEST_CASE("SCN-19 Saturn's rings are two ellipses, one pixel apart") {
    RasterCanvas canvas = renderedScene();
    const std::vector<Planet> planets = solarSystemPlanets();
    const Planet& saturn = findPlanet(planets, "Saturn");
    Pixel center = planetPosition(saturn, SUN_X, SUN_Y);

    for (int side : {-1, 1}) {
        CHECK_EQ(canvas.getPixel(center.x + side * ringRadiusX(saturn), center.y),
                 RING_COLOR);
        CHECK_EQ(canvas.getPixel(center.x + side * (ringRadiusX(saturn) - 1), center.y),
                 RING_COLOR);
    }
}

int main() {
    return testing::runAllTests();
}
