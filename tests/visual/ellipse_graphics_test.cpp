#include <iostream>
#include <filesystem>
#include "raster/raster_canvas.h"
#include "algorithms/shapes/midpoint_ellipse.h"

int main() {
    std::filesystem::create_directories("output");

    RasterCanvas canvas(500, 500);

    auto drawEllipse = [&](int rx, int ry, int red, int green, int blue) {
        auto points = drawMidpointEllipse(250, 250, rx, ry);

        for (const auto& point : points) {
            canvas.setPixel(point.first, point.second, red, green, blue);
        }
    };

    drawEllipse(150, 80, 255, 0, 0);
    drawEllipse(110, 55, 0, 180, 0);
    drawEllipse(70, 30, 0, 0, 255);

    canvas.saveBMP("output/ellipse_graphics_test.bmp");

    std::cout << "Ellipse graphics test completed.\n";
    std::cout << "Image saved to output/ellipse_graphics_test.bmp\n";

    return 0;
}
