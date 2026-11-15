// Visual test: the Solar System seen through the 3D camera from three
// angles, drawn with the project's own algorithms (Bresenham lines and
// midpoint circles) instead of an operating-system drawing library.
// Output: output/camera_projection_output.bmp
//
// Left: from above (pitch 89). Middle: the default view (pitch 35).
// Right: nearly edge-on (pitch 8). Planets are drawn at twice their
// projected size so they are easy to see.

#include <algorithm>
#include <cmath>
#include <map>
#include <vector>

#include "algorithms/shapes/midpoint_circle.h"
#include "scene/camera.h"
#include "scene/solar_system_scene.h"
#include "visual_output.h"

namespace {

const int PANEL = 400;

struct Body {
    Vec3 position;
    double radius;
    Color color;
    bool hasRings;
    double depth;
};

// A planet's place in 3D: its orbit lies flat in the x-z plane.
Vec3 worldPosition(const Planet& planet) {
    double radians = degreesToRadians(planet.angle);
    return {planet.orbitX * std::cos(radians), 0.0,
            planet.orbitY * std::sin(radians)};
}

// A filled disc built from the midpoint circle: each row is filled
// between the circle's leftmost and rightmost pixel.
void fillCircle(RasterCanvas& canvas, int centerX, int centerY, int radius,
                const Color& color) {
    std::map<int, std::pair<int, int>> rows;

    for (const auto& point : drawMidpointCircle(centerX, centerY, radius)) {
        auto found = rows.find(point.second);
        if (found == rows.end()) {
            rows[point.second] = {point.first, point.first};
        } else {
            found->second.first = std::min(found->second.first, point.first);
            found->second.second = std::max(found->second.second, point.first);
        }
    }

    for (const auto& row : rows) {
        canvas.drawLine(row.second.first, row.first, row.second.second,
                        row.first, color);
    }
}

// Projects and joins points round a flat circle or ellipse in 3D.
void drawProjectedLoop(RasterCanvas& canvas, const Camera& camera, int offsetX,
                       Vec3 center, double radiusX, double radiusZ,
                       const Color& color) {
    const int samples = 180;
    ScreenPoint previous;

    for (int i = 0; i <= samples; i++) {
        double angle = 2.0 * 3.14159265358979323846 * i / samples;
        Vec3 point = {center.x + radiusX * std::cos(angle), center.y,
                      center.z + radiusZ * std::sin(angle)};
        ScreenPoint current = camera.project(point, PANEL, PANEL);

        if (i > 0 && previous.visible && current.visible) {
            canvas.drawLine(offsetX + static_cast<int>(previous.x),
                            static_cast<int>(previous.y),
                            offsetX + static_cast<int>(current.x),
                            static_cast<int>(current.y), color);
        }

        previous = current;
    }
}

void drawView(RasterCanvas& canvas, double pitch, int offsetX) {
    Camera camera;
    camera.yaw = 0.0;
    camera.pitch = pitch;
    camera.distance = 1400.0;

    const double focal = (PANEL / 2.0) / std::tan(camera.fov * 3.14159265358979323846 / 360.0);

    for (const Planet& planet : solarSystemPlanets()) {
        drawProjectedLoop(canvas, camera, offsetX, {0, 0, 0},
                          planet.orbitX, planet.orbitY, VISUAL_GREY);
    }

    std::vector<Body> bodies;
    bodies.push_back({{0, 0, 0}, static_cast<double>(SUN_RADIUS), SUN_COLOR, false, 0});

    for (const Planet& planet : solarSystemPlanets()) {
        bodies.push_back({worldPosition(planet), 2.0 * planet.radius,
                          planet.color, planet.hasRings, 0});
    }

    for (Body& body : bodies) {
        body.depth = camera.project(body.position, PANEL, PANEL).depth;
    }

    // Painter's algorithm: farthest first.
    std::sort(bodies.begin(), bodies.end(),
              [](const Body& a, const Body& b) { return a.depth > b.depth; });

    for (const Body& body : bodies) {
        ScreenPoint p = camera.project(body.position, PANEL, PANEL);
        if (!p.visible) {
            continue;
        }

        int radius = std::max(2, static_cast<int>(body.radius * focal / p.depth));
        fillCircle(canvas, offsetX + static_cast<int>(p.x), static_cast<int>(p.y),
                   radius, body.color);

        if (body.hasRings) {
            drawProjectedLoop(canvas, camera, offsetX, body.position,
                              body.radius * 1.6, body.radius * 1.6, RING_COLOR);
        }
    }
}

}  // namespace

int main() {
    RasterCanvas canvas = makeVisualCanvas(3 * PANEL, PANEL);

    drawView(canvas, 89.0, 0);
    drawView(canvas, 35.0, PANEL);
    drawView(canvas, 8.0, 2 * PANEL);

    // Dividers between the three views.
    canvas.drawLine(PANEL, 0, PANEL, PANEL - 1, VISUAL_WHITE);
    canvas.drawLine(2 * PANEL, 0, 2 * PANEL, PANEL - 1, VISUAL_WHITE);

    return saveVisualOutput(canvas, "camera_projection_output.bmp");
}
