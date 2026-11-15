// Visual test: 120 days of orbital motion. The scene is drawn at day 120
// and each planet's path since day 0 is marked in its own colour, using
// the orbital periods and orbit::positionAtTime().
// Output: output/orbit_motion_output.bmp
//
// Inner planets leave long trails (Mercury goes more than once round);
// the outer planets barely move, as in the real Solar System.

#include <cmath>
#include <vector>

#include "algorithms/orbit/orbit_math.h"
#include "scene/solar_system_scene.h"
#include "visual_output.h"

int main() {
    const double days = 120.0;
    const Color white = {255, 255, 255};

    RasterCanvas canvas(SOLAR_SYSTEM_WIDTH, SOLAR_SYSTEM_HEIGHT);
    renderSolarSystem(canvas, days);

    const int sunX = SOLAR_SYSTEM_WIDTH / 2;
    const int sunY = SOLAR_SYSTEM_HEIGHT / 2;
    std::vector<Planet> planets = solarSystemPlanets();
    std::vector<double> periods = solarSystemOrbitalPeriods();

    for (std::size_t i = 0; i < planets.size() && i < periods.size(); i++) {
        const Planet& planet = planets[i];
        orbit::Position end = orbit::positionAtTime(
            sunX, sunY, planet.orbitX, planet.orbitY, planet.angle, days,
            periods[i]);

        // A dot every 2 days, kept clear of the planet's final position.
        for (double day = 0.0; day < days; day += 2.0) {
            orbit::Position p = orbit::positionAtTime(
                sunX, sunY, planet.orbitX, planet.orbitY, planet.angle, day,
                periods[i]);

            if (std::hypot(p.x - end.x, p.y - end.y) <= planet.radius + 2) {
                continue;
            }

            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    canvas.setPixel(static_cast<int>(p.x) + dx,
                                    static_cast<int>(p.y) + dy, planet.color);
                }
            }
        }
    }

    return saveVisualOutput(canvas, "orbit_motion_output.bmp", white);
}
