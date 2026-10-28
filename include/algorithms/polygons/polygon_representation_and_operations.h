#ifndef ALGORITHMS_POLYGONS_POLYGON_REPRESENTATION_AND_OPERATIONS_H
#define ALGORITHMS_POLYGONS_POLYGON_REPRESENTATION_AND_OPERATIONS_H

#include <cmath>
#include <vector>

#include "algorithms/geometry.h"

// A polygon is represented by its vertices in order; the last vertex
// connects back to the first. The operations below change it in place.

inline void translatePolygon(
    std::vector<Point>& vertices,
    double horizontalMovement,
    double verticalMovement
) {
    for (Point& vertex : vertices) {
        vertex.x += horizontalMovement;
        vertex.y += verticalMovement;
    }
}

// Scales every vertex away from (or towards) a fixed point.
inline void scalePolygon(
    std::vector<Point>& vertices,
    double scaleFactor,
    Point fixedPoint
) {
    for (Point& vertex : vertices) {
        vertex.x = fixedPoint.x
            + (vertex.x - fixedPoint.x) * scaleFactor;

        vertex.y = fixedPoint.y
            + (vertex.y - fixedPoint.y) * scaleFactor;
    }
}

// Rotates every vertex about a fixed point. Positive angles turn from
// the +x axis towards the +y axis.
inline void rotatePolygon(
    std::vector<Point>& vertices,
    double angleInDegrees,
    Point fixedPoint
) {
    const double pi = std::acos(-1.0);
    double angleInRadians = angleInDegrees * pi / 180.0;

    double cosineValue = std::cos(angleInRadians);
    double sineValue = std::sin(angleInRadians);

    for (Point& vertex : vertices) {
        double relativeX = vertex.x - fixedPoint.x;
        double relativeY = vertex.y - fixedPoint.y;

        double rotatedX =
            relativeX * cosineValue - relativeY * sineValue;

        double rotatedY =
            relativeX * sineValue + relativeY * cosineValue;

        vertex.x = fixedPoint.x + rotatedX;
        vertex.y = fixedPoint.y + rotatedY;
    }
}

#endif
