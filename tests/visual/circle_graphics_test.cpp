#include <iostream>
#include <filesystem>
#include "raster/raster_canvas.h"
#include "algorithms/shapes/midpoint_circle.h"

int main() {
    std::filesystem::create_directories("output");

    RasterCanvas canvas(500, 500);

    auto drawCircle = [&](int radius, int red, int green, int blue) {
        auto points = drawMidpointCircle(250, 250, radius);

        for (const auto& point : points) {
            canvas.setPixel(point.first, point.second, red, green, blue);
        }
    };

    drawCircle(100, 255, 0, 0);
    drawCircle(70, 0, 180, 0);
    drawCircle(40, 0, 0, 255);

    canvas.saveBMP("output/circle_graphics_test.bmp");

    std::cout << "Circle graphics test completed.\n";
    std::cout << "Image saved to output/circle_graphics_test.bmp\n";

    return 0;
}
