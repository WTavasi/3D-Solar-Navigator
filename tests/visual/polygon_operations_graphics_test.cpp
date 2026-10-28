#include <iostream>
#include <vector>
#include <utility>
#include "raster/raster_canvas.h"

using Point = std::pair<int, int>;
using Polygon = std::vector<Point>;

void drawPolygon(
    RasterCanvas& canvas,
    const Polygon& polygon,
    unsigned char r,
    unsigned char g,
    unsigned char b
) {
    if (polygon.size() < 2) return;

    for (size_t i = 0; i < polygon.size(); i++) {
        size_t next = (i + 1) % polygon.size();

        canvas.drawLine(
            polygon[i].first,
            polygon[i].second,
            polygon[next].first,
            polygon[next].second,
            r, g, b
        );
    }
}

Polygon translatePolygon(const Polygon& polygon, int dx, int dy) {
    Polygon result;

    for (const auto& point : polygon) {
        result.push_back({
            point.first + dx,
            point.second + dy
        });
    }

    return result;
}

int main() {
    RasterCanvas canvas(800, 600);

    Polygon original = {
        {100, 150},
        {220, 100},
        {280, 220},
        {170, 280}
    };

    Polygon translated = translatePolygon(original, 300, 150);

    drawPolygon(canvas, original, 255, 0, 0);
    drawPolygon(canvas, translated, 0, 255, 0);

    canvas.saveBMP("output/polygon_operations_output.bmp");

    std::cout << "Polygon operations test completed.\n";
    return 0;
}

