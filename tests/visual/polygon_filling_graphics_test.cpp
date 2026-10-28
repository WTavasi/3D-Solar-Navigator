#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include "raster/raster_canvas.h"

using Point = std::pair<int, int>;
using Polygon = std::vector<Point>;

void fillPolygon(
    RasterCanvas& canvas,
    const Polygon& polygon,
    unsigned char r,
    unsigned char g,
    unsigned char b
) {
    if (polygon.size() < 3) return;

    int minY = polygon[0].second;
    int maxY = polygon[0].second;

    for (const auto& p : polygon) {
        minY = std::min(minY, p.second);
        maxY = std::max(maxY, p.second);
    }

    for (int y = minY; y <= maxY; y++) {
        std::vector<double> intersections;

        for (size_t i = 0; i < polygon.size(); i++) {
            Point a = polygon[i];
            Point b = polygon[(i + 1) % polygon.size()];

            // Ignore horizontal edges.
            if (a.second == b.second) continue;

            // Include the lower endpoint, exclude the upper endpoint.
            int lowY = std::min(a.second, b.second);
            int highY = std::max(a.second, b.second);

            if (y < lowY || y >= highY) continue;

            double x = a.first +
                static_cast<double>(y - a.second) *
                (b.first - a.first) /
                (b.second - a.second);

            intersections.push_back(x);
        }

        std::sort(intersections.begin(), intersections.end());

        for (size_t i = 0; i + 1 < intersections.size(); i += 2) {
            int xStart = static_cast<int>(
                std::ceil(intersections[i])
            );

            int xEnd = static_cast<int>(
                std::floor(intersections[i + 1])
            );

            for (int x = xStart; x <= xEnd; x++) {
                canvas.setPixel(x, y, r, g, b);
            }
        }
    }
}

void drawOutline(RasterCanvas& canvas, const Polygon& polygon) {
    for (size_t i = 0; i < polygon.size(); i++) {
        const auto& a = polygon[i];
        const auto& b = polygon[(i + 1) % polygon.size()];

        canvas.drawLine(
            a.first, a.second,
            b.first, b.second,
            255, 255, 255
        );
    }
}

int main() {
    RasterCanvas canvas(800, 600);

    Polygon spacecraft = {
        {400, 100},
        {500, 250},
        {460, 420},
        {340, 420},
        {300, 250}
    };

    fillPolygon(canvas, spacecraft, 0, 150, 255);
    drawOutline(canvas, spacecraft);

    canvas.saveBMP("output/polygon_filling_output.bmp");

    std::cout << "Polygon filling test completed.\n";
    return 0;
}

