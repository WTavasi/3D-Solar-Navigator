// Unit tests for the midpoint ellipse drawing algorithm.

#include <algorithm>
#include <cmath>
#include <set>
#include <utility>
#include <vector>

#include "algorithms/shapes/midpoint_circle.h"
#include "algorithms/shapes/midpoint_ellipse.h"
#include "test_framework.h"

namespace {

using PixelSet = std::set<std::pair<int, int>>;

// True when every pixel can reach every other pixel through
// 8-connected neighbours, so the outline has no gaps.
bool isConnected(const PixelSet& pixels) {
    if (pixels.empty()) {
        return true;
    }

    PixelSet visited = {*pixels.begin()};
    std::vector<std::pair<int, int>> toVisit = {*pixels.begin()};

    while (!toVisit.empty()) {
        std::pair<int, int> pixel = toVisit.back();
        toVisit.pop_back();

        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                std::pair<int, int> neighbour = {pixel.first + dx,
                                                 pixel.second + dy};

                if (pixels.count(neighbour) && !visited.count(neighbour)) {
                    visited.insert(neighbour);
                    toVisit.push_back(neighbour);
                }
            }
        }
    }

    return visited.size() == pixels.size();
}

// Shortest distance from (x, y) to the ellipse, found by sampling the
// curve densely. Slow but independent of the algorithm under test.
double distanceToEllipse(double x, double y, int rx, int ry) {
    const double pi = std::acos(-1.0);
    const int samples = 4000;
    double best = 1e18;

    for (int i = 0; i < samples; i++) {
        double angle = 2.0 * pi * i / samples;
        best = std::min(best, std::hypot(x - rx * std::cos(angle),
                                         y - ry * std::sin(angle)));
    }

    return best;
}

}  // namespace

TEST_CASE("ELL-01 matches the textbook first quadrant for rx = 8, ry = 6") {
    // Hearn and Baker's worked example: region 1 then region 2.
    PixelSet expected = {
        {0, 6}, {1, 6}, {2, 6}, {3, 6}, {4, 5}, {5, 5}, {6, 4}, {7, 3},
        {8, 2}, {8, 1}, {8, 0}
    };

    PixelSet firstQuadrant;
    for (const auto& pixel : drawMidpointEllipse(0, 0, 8, 6)) {
        if (pixel.first >= 0 && pixel.second >= 0) {
            firstQuadrant.insert(pixel);
        }
    }

    CHECK_EQ(firstQuadrant, expected);
}

TEST_CASE("ELL-02 the ellipse has four-way symmetry") {
    for (const auto& radii : {std::make_pair(8, 6), std::make_pair(150, 80),
                              std::make_pair(9, 40)}) {
        PixelSet ellipse = drawMidpointEllipse(0, 0, radii.first, radii.second);

        for (const auto& pixel : ellipse) {
            int x = pixel.first;
            int y = pixel.second;

            CHECK(ellipse.count({-x, y}) && ellipse.count({x, -y}) &&
                  ellipse.count({-x, -y}));
        }
    }
}

TEST_CASE("ELL-03 the four axis points are always drawn, even when very flat") {
    // Regression test: flat ellipses (rx >= 16 * ry) used to stop one
    // pixel short of (cx +/- rx, cy).
    int missing = 0;

    for (int rx = 0; rx <= 120; rx++) {
        for (int ry = 0; ry <= 120; ry++) {
            PixelSet ellipse = drawMidpointEllipse(0, 0, rx, ry);

            if (!ellipse.count({rx, 0}) || !ellipse.count({-rx, 0}) ||
                !ellipse.count({0, ry}) || !ellipse.count({0, -ry})) {
                missing++;
            }
        }
    }

    CHECK_EQ(missing, 0);
}

TEST_CASE("ELL-04 every pixel is within one pixel of the true ellipse") {
    double worstDistance = 0.0;
    const int radii[] = {1, 2, 5, 12, 33, 60};

    for (int rx : radii) {
        for (int ry : radii) {
            for (const auto& pixel : drawMidpointEllipse(0, 0, rx, ry)) {
                if (pixel.first >= 0 && pixel.second >= 0) {
                    worstDistance = std::max(
                        worstDistance,
                        distanceToEllipse(pixel.first, pixel.second, rx, ry));
                }
            }
        }
    }

    CHECK(worstDistance < 1.0);
}

TEST_CASE("ELL-05 the outline is one connected piece with no gaps") {
    for (int rx = 0; rx <= 90; rx += 3) {
        for (int ry = 0; ry <= 90; ry += 5) {
            CHECK(isConnected(drawMidpointEllipse(0, 0, rx, ry)));
        }
    }
}

TEST_CASE("ELL-06 equal radii draw exactly the midpoint circle") {
    for (int radius = 0; radius <= 150; radius++) {
        CHECK_EQ(drawMidpointEllipse(4, -9, radius, radius),
                 drawMidpointCircle(4, -9, radius));
    }
}

TEST_CASE("ELL-07 zero radii draw a point or a straight segment") {
    CHECK_EQ(drawMidpointEllipse(3, 3, 0, 0), (PixelSet{{3, 3}}));

    CHECK_EQ(drawMidpointEllipse(0, 0, 0, 2),
             (PixelSet{{0, -2}, {0, -1}, {0, 0}, {0, 1}, {0, 2}}));

    // Regression test: this used to return only the centre pixel.
    CHECK_EQ(drawMidpointEllipse(0, 0, 3, 0),
             (PixelSet{{-3, 0}, {-2, 0}, {-1, 0}, {0, 0}, {1, 0}, {2, 0},
                       {3, 0}}));

    CHECK(drawMidpointEllipse(0, 0, -1, 5).empty());
    CHECK(drawMidpointEllipse(0, 0, 5, -1).empty());
}

TEST_CASE("ELL-08 moving the centre moves every pixel by the same amount") {
    PixelSet atOrigin = drawMidpointEllipse(0, 0, 410, 265);
    PixelSet moved = drawMidpointEllipse(600, 400, 410, 265);

    PixelSet expected;
    for (const auto& pixel : atOrigin) {
        expected.insert({pixel.first + 600, pixel.second + 400});
    }

    CHECK_EQ(moved, expected);
}

int main() {
    return testing::runAllTests();
}
