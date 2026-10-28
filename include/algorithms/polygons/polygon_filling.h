#ifndef ALGORITHMS_POLYGONS_POLYGON_FILLING_H
#define ALGORITHMS_POLYGONS_POLYGON_FILLING_H

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

#include "algorithms/geometry.h"

// Scan-line polygon filling.
//
// Each non-horizontal edge is stored with its lower y, upper y, the x
// at the lower end and its inverse slope (change in x per unit of y).
// A scan line at y crosses an edge when lowerY <= y < upperY, so a
// vertex shared by two edges is only counted once.

struct Edge {
    double minimumY;
    double maximumY;
    double currentX;
    double inverseSlope;
};

// One run of filled pixels on scan line y, from startX to endX inclusive.
struct FillSpan {
    int y;
    int startX;
    int endX;
};

inline std::vector<Edge> buildEdgeTable(const std::vector<Point>& polygon) {
    std::vector<Edge> edges;

    for (std::size_t index = 0; index < polygon.size(); index++) {
        Point startingPoint = polygon[index];
        Point endingPoint = polygon[(index + 1) % polygon.size()];

        // Horizontal edges do not contribute scan-line intersections.
        if (startingPoint.y == endingPoint.y) {
            continue;
        }

        if (startingPoint.y > endingPoint.y) {
            std::swap(startingPoint, endingPoint);
        }

        double inverseSlope =
            (endingPoint.x - startingPoint.x) /
            (endingPoint.y - startingPoint.y);

        edges.push_back({
            startingPoint.y,
            endingPoint.y,
            startingPoint.x,
            inverseSlope
        });
    }

    return edges;
}

// The scan lines to process are minimumY <= y < maximumY.
inline void findScanlineRange(const std::vector<Point>& polygon,
                              int& minimumY, int& maximumY) {
    minimumY = 0;
    maximumY = 0;

    if (polygon.empty()) {
        return;
    }

    minimumY = static_cast<int>(std::ceil(polygon[0].y));
    maximumY = static_cast<int>(std::ceil(polygon[0].y));

    for (const Point& vertex : polygon) {
        minimumY = std::min(minimumY, static_cast<int>(std::ceil(vertex.y)));
        maximumY = std::max(maximumY, static_cast<int>(std::ceil(vertex.y)));
    }
}

// The x positions where scan line y crosses the polygon, sorted.
inline std::vector<double> findScanlineIntersections(
    const std::vector<Edge>& edges,
    int scanY
) {
    std::vector<double> intersections;

    for (const Edge& edge : edges) {
        if (scanY >= edge.minimumY &&
            scanY < edge.maximumY) {
            double intersectionX =
                edge.currentX +
                (scanY - edge.minimumY) * edge.inverseSlope;

            intersections.push_back(intersectionX);
        }
    }

    std::sort(intersections.begin(), intersections.end());

    return intersections;
}

// All filled spans of the polygon, scan line by scan line. Pixels are
// filled between pairs of intersections, from ceil(left) to floor(right).
inline std::vector<FillSpan> computeFillSpans(const std::vector<Point>& polygon) {
    std::vector<FillSpan> spans;

    if (polygon.size() < 3) {
        return spans;
    }

    std::vector<Edge> edges = buildEdgeTable(polygon);

    int minimumY = 0;
    int maximumY = 0;
    findScanlineRange(polygon, minimumY, maximumY);

    for (int scanY = minimumY; scanY < maximumY; scanY++) {
        std::vector<double> intersections =
            findScanlineIntersections(edges, scanY);

        for (std::size_t index = 0;
             index + 1 < intersections.size();
             index += 2) {

            int startingX =
                static_cast<int>(std::ceil(intersections[index]));

            int endingX =
                static_cast<int>(std::floor(intersections[index + 1]));

            // A pair closer together than one pixel fills nothing.
            if (startingX <= endingX) {
                spans.push_back({scanY, startingX, endingX});
            }
        }
    }

    return spans;
}

#endif
