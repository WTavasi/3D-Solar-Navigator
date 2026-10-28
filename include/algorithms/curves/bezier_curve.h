#ifndef ALGORITHMS_CURVES_BEZIER_CURVE_H
#define ALGORITHMS_CURVES_BEZIER_CURVE_H

#include <vector>

#include "algorithms/geometry.h"

// Cubic Bezier curve from a start point, two control points and an end
// point, evaluated with the Bernstein polynomials
//   B(t) = (1-t)^3 P0 + 3(1-t)^2 t P1 + 3(1-t) t^2 P2 + t^3 P3.

inline Point calculateBezierPoint(
    double parameter,
    Point startingPoint,
    Point firstControlPoint,
    Point secondControlPoint,
    Point endingPoint
) {
    double remainingParameter = 1.0 - parameter;

    double startingWeight =
        remainingParameter * remainingParameter * remainingParameter;

    double firstControlWeight =
        3 * remainingParameter * remainingParameter * parameter;

    double secondControlWeight =
        3 * remainingParameter * parameter * parameter;

    double endingWeight =
        parameter * parameter * parameter;

    Point curvePoint;

    curvePoint.x =
        startingWeight * startingPoint.x
        + firstControlWeight * firstControlPoint.x
        + secondControlWeight * secondControlPoint.x
        + endingWeight * endingPoint.x;

    curvePoint.y =
        startingWeight * startingPoint.y
        + firstControlWeight * firstControlPoint.y
        + secondControlWeight * secondControlPoint.y
        + endingWeight * endingPoint.y;

    return curvePoint;
}

inline std::vector<Point> generateBezierCurve(
    Point startingPoint,
    Point firstControlPoint,
    Point secondControlPoint,
    Point endingPoint,
    int numberOfSteps
) {
    std::vector<Point> curvePoints;

    if (numberOfSteps < 1) {
        numberOfSteps = 1;
    }

    for (int step = 0; step <= numberOfSteps; step++) {
        double parameter =
            static_cast<double>(step) / numberOfSteps;

        Point curvePoint = calculateBezierPoint(
            parameter,
            startingPoint,
            firstControlPoint,
            secondControlPoint,
            endingPoint
        );

        curvePoints.push_back(curvePoint);
    }

    return curvePoints;
}

#endif
