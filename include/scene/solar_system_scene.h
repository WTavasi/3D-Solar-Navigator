#ifndef SCENE_SOLAR_SYSTEM_SCENE_H
#define SCENE_SOLAR_SYSTEM_SCENE_H

// The Solar System scene: the Sun, the eight planets and their orbits,
// drawn with the project's own midpoint circle and ellipse algorithms.
// Keeping the scene data and drawing code here lets the graphics test
// render it and the integration tests check it.

#include <cmath>
#include <vector>

#include "algorithms/geometry.h"
#include "algorithms/shapes/midpoint_circle.h"
#include "algorithms/shapes/midpoint_ellipse.h"
#include "raster/raster_canvas.h"

struct Planet {
    const char* name;
    int orbitX;      // horizontal radius of the orbit, in pixels
    int orbitY;      // vertical radius of the orbit, in pixels
    int radius;      // drawn radius of the planet, in pixels
    double angle;    // position along the orbit, in degrees
    Color color;
    bool hasRings;
};

constexpr int SOLAR_SYSTEM_WIDTH = 1200;
constexpr int SOLAR_SYSTEM_HEIGHT = 800;

constexpr int SUN_RADIUS = 35;
constexpr int SUN_RAY_INNER_RADIUS = 40;
constexpr int SUN_RAY_OUTER_RADIUS = 55;
constexpr int SUN_RAY_SPACING_DEGREES = 30;

constexpr Color SUN_COLOR = {255, 220, 0};
constexpr Color ORBIT_COLOR = {75, 75, 90};
constexpr Color RING_COLOR = {210, 190, 130};
constexpr Color HIGHLIGHT_COLOR = {255, 255, 255};

// The planets in order from the Sun.
inline std::vector<Planet> solarSystemPlanets() {
    return {
        {"Mercury", 100,  65,  7,  20, {180, 180, 180}, false},
        {"Venus",   150,  95, 10,  80, {230, 170,  50}, false},
        {"Earth",   210, 135, 11, 145, { 40, 100, 255}, false},
        {"Mars",    270, 175,  9, 210, {255,  70,  50}, false},
        {"Jupiter", 340, 220, 19,  35, {230, 150,  70}, false},
        {"Saturn",  410, 265, 16, 250, {240, 220, 120}, true},
        {"Uranus",  480, 310, 13, 120, { 70, 220, 220}, false},
        {"Neptune", 550, 350, 13, 300, { 70, 100, 255}, false}
    };
}

inline double degreesToRadians(double degrees) {
    const double pi = 3.14159265358979323846;
    return degrees * pi / 180.0;
}

// Centre of a planet on its elliptical orbit around (sunX, sunY).
inline Pixel planetPosition(const Planet& planet, int sunX, int sunY) {
    double radians = degreesToRadians(planet.angle);

    return {
        sunX + static_cast<int>(planet.orbitX * std::cos(radians)),
        sunY + static_cast<int>(planet.orbitY * std::sin(radians))
    };
}

inline void drawEllipseOutline(RasterCanvas& canvas, int centerX, int centerY,
                               int radiusX, int radiusY, const Color& color) {
    for (const auto& point :
         drawMidpointEllipse(centerX, centerY, radiusX, radiusY)) {
        canvas.setPixel(point.first, point.second, color);
    }
}

inline void drawCircleOutline(RasterCanvas& canvas, int centerX, int centerY,
                              int radius, const Color& color) {
    for (const auto& point : drawMidpointCircle(centerX, centerY, radius)) {
        canvas.setPixel(point.first, point.second, color);
    }
}

// Short straight rays around the Sun.
inline void drawSunRays(RasterCanvas& canvas, int centerX, int centerY) {
    for (int angle = 0; angle < 360; angle += SUN_RAY_SPACING_DEGREES) {
        double radians = degreesToRadians(angle);

        int x1 = centerX + static_cast<int>(SUN_RAY_INNER_RADIUS * std::cos(radians));
        int y1 = centerY + static_cast<int>(SUN_RAY_INNER_RADIUS * std::sin(radians));
        int x2 = centerX + static_cast<int>(SUN_RAY_OUTER_RADIUS * std::cos(radians));
        int y2 = centerY + static_cast<int>(SUN_RAY_OUTER_RADIUS * std::sin(radians));

        canvas.drawLine(x1, y1, x2, y2, SUN_COLOR);
    }
}

// Horizontal and vertical radii of a ringed planet's rings.
inline int ringRadiusX(const Planet& planet) {
    return planet.radius + 9;
}

inline int ringRadiusY(const Planet& planet) {
    return planet.radius / 2 + 4;
}

// Draws the whole scene centred on the canvas.
inline void renderSolarSystem(RasterCanvas& canvas) {
    const int sunX = canvas.getWidth() / 2;
    const int sunY = canvas.getHeight() / 2;
    const std::vector<Planet> planets = solarSystemPlanets();

    // Orbits first, so the Sun and planets are drawn on top of them.
    for (const Planet& planet : planets) {
        drawEllipseOutline(canvas, sunX, sunY,
                           planet.orbitX, planet.orbitY, ORBIT_COLOR);
    }

    drawCircleOutline(canvas, sunX, sunY, SUN_RADIUS, SUN_COLOR);
    drawSunRays(canvas, sunX, sunY);

    for (const Planet& planet : planets) {
        Pixel center = planetPosition(planet, sunX, sunY);

        drawCircleOutline(canvas, center.x, center.y,
                          planet.radius, planet.color);

        // A small highlight to make the planet stand out.
        drawCircleOutline(canvas,
                          center.x - planet.radius / 3,
                          center.y - planet.radius / 3,
                          1, HIGHLIGHT_COLOR);

        if (planet.hasRings) {
            drawEllipseOutline(canvas, center.x, center.y,
                               ringRadiusX(planet), ringRadiusY(planet),
                               RING_COLOR);
        }
    }
}

#endif
