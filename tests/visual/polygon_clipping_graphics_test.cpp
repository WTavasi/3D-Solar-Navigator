#include <iostream>
#include <vector>
#include <utility>
#include "raster/raster_canvas.h"

using Point = std::pair<double, double>;
using Polygon = std::vector<Point>;

enum class Edge {
    Left,
    Right,
    Top,
    Bottom
};

bool inside(const Point& p, Edge edge,
            double xmin, double ymin,
            double xmax, double ymax) {
    switch (edge) {
        case Edge::Left:   return p.first >= xmin;
        case Edge::Right:  return p.first <= xmax;
        case Edge::Top:    return p.second >= ymin;
        case Edge::Bottom: return p.second <= ymax;
    }

    return false;
}

Point intersection(const Point& a, const Point& b, Edge edge,
                   double xmin, double ymin,
                   double xmax, double ymax) {
    double dx = b.first - a.first;
    double dy = b.second - a.second;

    if (edge == Edge::Left || edge == Edge::Right) {
        double x = (edge == Edge::Left) ? xmin : xmax;

        if (dx == 0) return {x, a.second};

        double t = (x - a.first) / dx;
        return {x, a.second + t * dy};
    }

    double y = (edge == Edge::Top) ? ymin : ymax;

    if (dy == 0) return {a.first, y};

    double t = (y - a.second) / dy;
    return {a.first + t * dx, y};
}

Polygon clipAgainstEdge(
    const Polygon& input,
    Edge edge,
    double xmin, double ymin,
    double xmax, double ymax
) {
    Polygon output;

    if (input.empty()) return output;

    Point previous = input.back();
    bool previousInside = inside(
        previous, edge, xmin, ymin, xmax, ymax
    );

    for (const Point& current : input) {
        bool currentInside = inside(
            current, edge, xmin, ymin, xmax, ymax
        );

        if (currentInside) {
            if (!previousInside) {
                output.push_back(intersection(
                    previous, current, edge,
                    xmin, ymin, xmax, ymax
                ));
            }

            output.push_back(current);
        } else if (previousInside) {
            output.push_back(intersection(
                previous, current, edge,
                xmin, ymin, xmax, ymax
            ));
        }

        previous = current;
        previousInside = currentInside;
    }

    return output;
}

Polygon clipPolygon(
    Polygon polygon,
    double xmin, double ymin,
    double xmax, double ymax
) {
    polygon = clipAgainstEdge(
        polygon, Edge::Left, xmin, ymin, xmax, ymax
    );

    polygon = clipAgainstEdge(
        polygon, Edge::Right, xmin, ymin, xmax, ymax
    );

    polygon = clipAgainstEdge(
        polygon, Edge::Top, xmin, ymin, xmax, ymax
    );

    polygon = clipAgainstEdge(
        polygon, Edge::Bottom, xmin, ymin, xmax, ymax
    );

    return polygon;
}

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
            static_cast<int>(polygon[i].first),
            static_cast<int>(polygon[i].second),
            static_cast<int>(polygon[next].first),
            static_cast<int>(polygon[next].second),
            r, g, b
        );
    }
}

int main() {
    RasterCanvas canvas(800, 600);

    double xmin = 200;
    double ymin = 150;
    double xmax = 600;
    double ymax = 450;

    // Draw the rectangular clipping window.
    canvas.drawLine(200, 150, 600, 150, 255, 255, 255);
    canvas.drawLine(600, 150, 600, 450, 255, 255, 255);
    canvas.drawLine(600, 450, 200, 450, 255, 255, 255);
    canvas.drawLine(200, 450, 200, 150, 255, 255, 255);

    Polygon original = {
        {100, 250},
        {350, 80},
        {700, 250},
        {500, 520},
        {180, 450}
    };

    Polygon clipped = clipPolygon(
        original, xmin, ymin, xmax, ymax
    );

    drawPolygon(canvas, original, 255, 0, 0);
    drawPolygon(canvas, clipped, 0, 255, 0);

    canvas.saveBMP("output/polygon_clipping_output.bmp");

    std::cout << "Polygon clipping test completed.\n";
    return 0;
}

