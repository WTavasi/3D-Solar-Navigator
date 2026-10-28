#include <iostream>

#include "raster/raster_canvas.h"
#include "scene/solar_system_scene.h"

int main() {
    RasterCanvas canvas(SOLAR_SYSTEM_WIDTH, SOLAR_SYSTEM_HEIGHT);

    renderSolarSystem(canvas);

    if (canvas.saveBMP("solar_system_output.bmp")) {
        std::cout << "Solar System image saved successfully!\n";
        std::cout << "File: solar_system_output.bmp\n";
        std::cout << "Planets: " << solarSystemPlanets().size() << "\n";
        std::cout << "Circle algorithm: Midpoint Circle Drawing\n";
    } else {
        std::cerr << "Failed to save Solar System image.\n";
        return 1;
    }

    return 0;
}
