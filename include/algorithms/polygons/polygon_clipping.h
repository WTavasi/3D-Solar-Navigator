#ifndef ALGORITHMS_POLYGONS_POLYGON_CLIPPING_H
#define ALGORITHMS_POLYGONS_POLYGON_CLIPPING_H

#include <vector>

#include "algorithms/geometry.h"

// Sutherland-Hodgman polygon clipping against an axis-aligned window.
// The polygon is clipped against one window edge at a time.
//
// The boundary names follow maths (y-up) coordinates: BOTTOM is the
// minimum y and TOP is the maximum y. On a y-down screen, BOTTOM is
// therefore the top edge of the window.

enum class ClippingBoundary {
    LEFT,
    RIGHT,
    BOTTOM,
    TOP
};

inline bool isInside(Point point, ClippingBoundary boundary,
                     double boundaryValue) {
    switch (boundary) {
        case ClippingBoundary::LEFT:
            return point.x >= boundaryValue;

        case ClippingBoundary::RIGHT:
            return point.x <= boundaryValue;

        case ClippingBoundary::BOTTOM:
            return point.y >= boundaryValue;

        case ClippingBoundary::TOP:
            return point.y <= boundaryValue;
    }

    return false;
}

inline Point findIntersection(Point startingPoint, Point endingPoint,
                              ClippingBoundary boundary,
                              double boundaryValue) {
    double differenceX = endingPoint.x - startingPoint.x;
    double differenceY = endingPoint.y - startingPoint.y;

    if (boundary == ClippingBoundary::LEFT ||
        boundary == ClippingBoundary::RIGHT) {
        double parameter =
            (boundaryValue - startingPoint.x) / differenceX;

        return {
            boundaryValue,
            startingPoint.y + parameter * differenceY
        };
    }

    double parameter =
        (boundaryValue - startingPoint.y) / differenceY;

    return {
        startingPoint.x + parameter * differenceX,
        boundaryValue
    };
}

inline std::vector<Point> clipAgainstBoundary(
    const std::vector<Point>& polygon,
    ClippingBoundary boundary,
    double boundaryValue
) {
    std::vector<Point> clippedPolygon;

    if (polygon.empty()) {
        return clippedPolygon;
    }

    Point previousPoint = polygon.back();
    bool previousInside =
        isInside(previousPoint, boundary, boundaryValue);

    for (const Point& currentPoint : polygon) {
        bool currentInside =
            isInside(currentPoint, boundary, boundaryValue);

        if (currentInside) {
            if (!previousInside) {
                clippedPolygon.push_back(
                    findIntersection(previousPoint, currentPoint,
                                     boundary, boundaryValue)
                );
            }

            clippedPolygon.push_back(currentPoint);
        } else if (previousInside) {
            clippedPolygon.push_back(
                findIntersection(previousPoint, currentPoint,
                                 boundary, boundaryValue)
            );
        }

        previousPoint = currentPoint;
        previousInside = currentInside;
    }

    return clippedPolygon;
}

inline std::vector<Point> sutherlandHodgmanClip(
    std::vector<Point> polygon,
    double minimumX,
    double minimumY,
    double maximumX,
    double maximumY
) {
    polygon = clipAgainstBoundary(
        polygon, ClippingBoundary::LEFT, minimumX
    );

    polygon = clipAgainstBoundary(
        polygon, ClippingBoundary::RIGHT, maximumX
    );

    polygon = clipAgainstBoundary(
        polygon, ClippingBoundary::BOTTOM, minimumY
    );

    polygon = clipAgainstBoundary(
        polygon, ClippingBoundary::TOP, maximumY
    );

    return polygon;
}

#endif
