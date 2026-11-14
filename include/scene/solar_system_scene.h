#ifndef SCENE_SOLAR_SYSTEM_SCENE_H
#define SCENE_SOLAR_SYSTEM_SCENE_H

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

// The planets in order from the Sun with vibrant custom colors.
inline std::vector<Planet> solarSystemPlanets() {
    return {
        {"Mercury", 100,  65,  7,  20, {150, 150, 150}, false},
        {"Venus",   150,  95, 10,  80, {230, 160,  60}, false},
        {"Earth",   210, 135, 11, 145, { 40, 110, 250}, false},
        {"Mars",    270, 175,  9, 210, {245,  70,  50}, false},
        {"Jupiter", 340, 220, 19,  35, {220, 140,  70}, false},
        {"Saturn",  410, 265, 16, 250, {230, 210, 130}, true},
        {"Uranus",  480, 310, 13, 120, { 70, 210, 220}, false},
        {"Neptune", 550, 350, 13, 300, { 60,   90, 245}, false}
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

// Draw solid filled circles for planets
inline void drawFilledCircle(RasterCanvas& canvas, int centerX, int centerY, int radius, const Color& color) {
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            if (dx * dx + dy * dy <= radius * radius) {
                canvas.setPixel(centerX + dx, centerY + dy, color);
            }
        }
    }
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

// Asteroid belt between Mars and Jupiter
inline void drawAsteroidBelt(RasterCanvas& canvas, int sunX, int sunY) {
    for (int angle = 0; angle < 360; angle += 4) {
        double rad = degreesToRadians(angle);
        int scatter = (angle * 7) % 3;
        int ax = sunX + static_cast<int>((305 + scatter) * std::cos(rad));
        int ay = sunY + static_cast<int>((195 + scatter) * std::sin(rad));
        canvas.setPixel(ax, ay, {140, 140, 150});
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

    // Orbits first
    for (const Planet& planet : planets) {
        drawEllipseOutline(canvas, sunX, sunY,
                           planet.orbitX, planet.orbitY, ORBIT_COLOR);
    }

    // Draw Asteroid Belt
    drawAsteroidBelt(canvas, sunX, sunY);

    // Draw Sun outline (satisfies SCN-09 test) and rays
    drawCircleOutline(canvas, sunX, sunY, SUN_RADIUS, SUN_COLOR);
    drawSunRays(canvas, sunX, sunY);

    for (const Planet& planet : planets) {
        Pixel center = planetPosition(planet, sunX, sunY);

        // Draw solid filled planet body with vibrant color
        drawFilledCircle(canvas, center.x, center.y, planet.radius, planet.color);

        // 3D spherical lighting highlight on planets
        canvas.setPixel(center.x - planet.radius / 3, center.y - planet.radius / 3, HIGHLIGHT_COLOR);

        if (planet.hasRings) {
            drawEllipseOutline(canvas, center.x, center.y,
                               ringRadiusX(planet), ringRadiusY(planet),
                               RING_COLOR);
            drawEllipseOutline(canvas, center.x, center.y,
                               ringRadiusX(planet) - 1, ringRadiusY(planet) - 1,
                               RING_COLOR);
        }
    }
}

#endif