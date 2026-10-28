
#ifndef MIDPOINT_ELLIPSE_H
#define MIDPOINT_ELLIPSE_H

#include <set>
#include <utility>

inline std::set<std::pair<int, int>> drawMidpointEllipse(
    int centerX,
    int centerY,
    int horizontalRadius,
    int verticalRadius
) {
    std::set<std::pair<int, int>> ellipsePoints;

    if (horizontalRadius < 0 || verticalRadius < 0) {
        return ellipsePoints;
    }

    long long rx2 = 1LL * horizontalRadius * horizontalRadius;
    long long ry2 = 1LL * verticalRadius * verticalRadius;

    long long x = 0;
    long long y = verticalRadius;

    double p1 = ry2 - rx2 * verticalRadius + 0.25 * rx2;

    long long dx = 0;
    long long dy = 2 * rx2 * y;

    auto addPoints = [&](int px, int py) {
        ellipsePoints.insert({centerX + px, centerY + py});
        ellipsePoints.insert({centerX - px, centerY + py});
        ellipsePoints.insert({centerX + px, centerY - py});
        ellipsePoints.insert({centerX - px, centerY - py});
    };

    // Region 1
    while (dx < dy) {
        addPoints(static_cast<int>(x), static_cast<int>(y));

        x++;
        dx += 2 * ry2;

        if (p1 < 0) {
            p1 += dx + ry2;
        } else {
            y--;
            dy -= 2 * rx2;
            p1 += dx - dy + ry2;
        }
    }

    // Region 2
    double p2 =
        ry2 * (x + 0.5) * (x + 0.5)
        + rx2 * (y - 1) * (y - 1)
        - rx2 * ry2;

    // Furthest x drawn on the horizontal axis (y = 0).
    long long lastXOnAxis = 0;

    while (y >= 0) {
        addPoints(static_cast<int>(x), static_cast<int>(y));

        if (y == 0) {
            lastXOnAxis = x;
        }

        y--;
        dy -= 2 * rx2;

        if (p2 > 0) {
            p2 += rx2 - dy;
        } else {
            x++;
            dx += 2 * ry2;
            p2 += dx - dy + rx2;
        }
    }

    // Very flat ellipses (horizontal radius at least 16 times the
    // vertical one, or a vertical radius of 0 or 1) reach y = 0 before
    // x reaches the horizontal radius. Finish the tips along the axis so
    // (centerX +/- horizontalRadius, centerY) is always drawn.
    for (long long tipX = lastXOnAxis + 1; tipX <= horizontalRadius; tipX++) {
        addPoints(static_cast<int>(tipX), 0);
    }

    return ellipsePoints;
}

#endif
