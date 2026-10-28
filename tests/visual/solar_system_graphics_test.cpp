// Visual test: the Solar System scene.
// Output: output/solar_system_output.bmp
//
// The scene keeps its own white background (a scene design choice), so
// the image is checked against white instead of the visual test colour.

#include <iostream>

#include "raster/raster_canvas.h"
#include "scene/solar_system_scene.h"
#include "visual_output.h"

int main() {
    RasterCanvas canvas(SOLAR_SYSTEM_WIDTH, SOLAR_SYSTEM_HEIGHT);

    renderSolarSystem(canvas);

    std::cout << "Planets: " << solarSystemPlanets().size() << "\n";

    return saveVisualOutput(canvas, "solar_system_output.bmp",
                            {255, 255, 255});
}
