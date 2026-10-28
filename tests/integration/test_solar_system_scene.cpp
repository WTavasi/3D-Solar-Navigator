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

int main() {
    return testing::runAllTests();
}
