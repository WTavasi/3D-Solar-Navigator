
#include <iostream>
#include <filesystem>
#include "raster/raster_canvas.h"

int main() {
    std::filesystem::create_directories("output");

    RasterCanvas canvas(800, 600);

    // Draw lines with different slopes and directions.
    canvas.drawLine(100, 100, 700, 100, 255, 0, 0);
    canvas.drawLine(100, 150, 700, 450, 0, 180, 0);
    canvas.drawLine(100, 500, 700, 200, 0, 0, 255);
    canvas.drawLine(400, 80, 400, 520, 255, 140, 0);
    canvas.drawLine(150, 500, 650, 500, 180, 0, 180);

    const std::string filename = "output/bresenham_output.bmp";

    if (!canvas.saveBMP(filename)) {
        std::cerr << "Failed to save Bresenham image.\n";
        return 1;
    }

    std::cout << "Bresenham graphics test completed.\n";
    std::cout << "Image saved to " << filename << '\n';

    return 0;
}
