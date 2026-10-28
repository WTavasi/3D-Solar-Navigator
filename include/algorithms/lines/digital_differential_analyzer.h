#ifndef ALGORITHMS_LINES_DIGITAL_DIFFERENTIAL_ANALYZER_H
#define ALGORITHMS_LINES_DIGITAL_DIFFERENTIAL_ANALYZER_H

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <vector>

#include "algorithms/geometry.h"

// Digital Differential Analyzer (DDA) line drawing.
// Returns every pixel from (x1, y1) to (x2, y2), both endpoints included.
inline std::vector<Pixel> drawDDA(int x1, int y1, int x2, int y2) {
    std::vector<Pixel> points;

    int dx = x2 - x1;
    int dy = y2 - y1;

    int steps = std::max(std::abs(dx), std::abs(dy));

    // Handle a line whose starting and ending points are identical.
    if (steps == 0) {
        points.push_back({x1, y1});
        return points;
    }

    double xIncrement = static_cast<double>(dx) / steps;
    double yIncrement = static_cast<double>(dy) / steps;

    double x = x1;
    double y = y1;

    for (int i = 0; i <= steps; i++) {
        points.push_back({
            static_cast<int>(std::round(x)),
            static_cast<int>(std::round(y))
        });

        x += xIncrement;
        y += yIncrement;
    }

    return points;
}

#endif
