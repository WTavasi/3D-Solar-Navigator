
#include <iostream>
#include "raster/raster_canvas.h"

int main() {
    // Create a white canvas measuring 400 by 300 pixels.
    RasterCanvas canvas(400, 300);

    // Draw a horizontal black line.
    canvas.drawLine(50, 50, 350, 50);

    // Draw a diagonal red line.
    canvas.drawLine(50, 80, 350, 250, 255, 0, 0);

    // Draw a vertical blue line.
    canvas.drawLine(200, 80, 200, 250, 0, 0, 255);

    // Save the image in the project root directory.
    if (canvas.saveBMP("raster_canvas_test.bmp")) {
        std::cout << "Image saved successfully!\n";
        std::cout << "File: raster_canvas_test.bmp\n";
    } else {
        std::cerr << "Failed to save image.\n";
        return 1;
    }

    return 0;
}
