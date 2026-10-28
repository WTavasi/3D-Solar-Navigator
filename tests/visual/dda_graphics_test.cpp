#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include "raster/raster_canvas.h"

struct Point {
    int x;
    int y;
};

// DDA algorithm: calculate the points on the line.
std::vector<Point> calculateDDAPoints(
    int x1, int y1, int x2, int y2
) {
    std::vector<Point> points;

    int differenceX = x2 - x1;
    int differenceY = y2 - y1;

    int steps = std::max(
        std::abs(differenceX),
        std::abs(differenceY)
    );

    if (steps == 0) {
        points.push_back({x1, y1});
        return points;
    }

    double incrementX =
        static_cast<double>(differenceX) / steps;
    double incrementY =
        static_cast<double>(differenceY) / steps;

    double x = x1;
    double y = y1;

    for (int step = 0; step <= steps; step++) {
        points.push_back({
            static_cast<int>(std::round(x)),
            static_cast<int>(std::round(y))
        });

        x += incrementX;
        y += incrementY;
    }

    return points;
}



int main() {
    RasterCanvas canvas(500, 400);

    // Center point for all lines.
    int centerX = 250;
    int centerY = 200;

    // Draw lines in different directions.
    auto line1 = calculateDDAPoints(0, 0, 150, 0);
    auto line2 = calculateDDAPoints(0, 0, 100, 100);
    auto line3 = calculateDDAPoints(0, 0, 0, 150);
    auto line4 = calculateDDAPoints(0, 0, -100, 100);

    // Plot each line around the center of the image.
    for (const Point& p : line1) {
        canvas.setPixel(centerX + p.x, centerY - p.y, 0, 0, 0);
    }

    for (const Point& p : line2) {
        canvas.setPixel(centerX + p.x, centerY - p.y, 255, 0, 0);
    }

    for (const Point& p : line3) {
        canvas.setPixel(centerX + p.x, centerY - p.y, 0, 0, 255);
    }

    for (const Point& p : line4) {
        canvas.setPixel(centerX + p.x, centerY - p.y, 0, 150, 0);
    }

    if (canvas.saveBMP("output/dda_output.bmp")) {
        std::cout << "DDA image saved: dda_output.bmp\n";
    } else {
        std::cerr << "Could not save DDA image.\n";
        return 1;
    }

    return 0;
}

