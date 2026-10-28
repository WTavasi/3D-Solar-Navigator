#ifndef ALGORITHMS_LINES_BRESENHAM_LINE_DRAWING_H
#define ALGORITHMS_LINES_BRESENHAM_LINE_DRAWING_H

#include <cstdlib>
#include <vector>

#include "algorithms/geometry.h"

// Bresenham's line drawing algorithm, using integer arithmetic only.
// Returns every pixel from the starting point to the ending point,
// both endpoints included.
inline std::vector<Pixel> drawBresenhamLine(
    int startingX,
    int startingY,
    int endingX,
    int endingY
) {
    std::vector<Pixel> points;

    int differenceX = std::abs(endingX - startingX);
    int differenceY = std::abs(endingY - startingY);

    int horizontalDirection =
        (startingX < endingX) ? 1 : -1;

    int verticalDirection =
        (startingY < endingY) ? 1 : -1;

    int decisionParameter = differenceX - differenceY;

    while (true) {
        points.push_back({startingX, startingY});

        if (startingX == endingX && startingY == endingY) {
            break;
        }

        int doubledDecisionParameter = 2 * decisionParameter;

        if (doubledDecisionParameter > -differenceY) {
            decisionParameter -= differenceY;
            startingX += horizontalDirection;
        }

        if (doubledDecisionParameter < differenceX) {
            decisionParameter += differenceX;
            startingY += verticalDirection;
        }
    }

    return points;
}

#endif
