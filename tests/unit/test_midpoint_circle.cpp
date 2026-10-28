// Unit tests for the midpoint circle drawing algorithm.

#include <algorithm>
#include <cmath>
#include <set>
#include <utility>
#include <vector>

#include "algorithms/shapes/midpoint_circle.h"
#include "test_framework.h"

namespace {

using PixelSet = std::set<std::pair<int, int>>;

int countNeighbours(const PixelSet& pixels, const std::pair<int, int>& pixel) {
    int count = 0;

    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if ((dx != 0 || dy != 0) &&
                pixels.count({pixel.first + dx, pixel.second + dy}) > 0) {
                count++;
            }
        }
    }

    return count;
}

}  // namespace

TEST_CASE("CIR-01 matches the textbook first octant for radius 10") {
    // Hearn and Baker's worked example: p0 = 1 - r = -9.
    std::vector<std::pair<int, int>> expected = {
        {0, 10}, {1, 10}, {2, 10}, {3, 10}, {4, 9}, {5, 9}, {6, 8}, {7, 7}
    };

    std::vector<std::pair<int, int>> firstOctant;
    for (const auto& pixel : drawMidpointCircle(0, 0, 10)) {
        if (pixel.first >= 0 && pixel.second >= pixel.first) {
            firstOctant.push_back(pixel);
        }
    }

    CHECK_EQ(firstOctant, expected);
}

TEST_CASE("CIR-02 the circle has eight-way symmetry") {
    for (int radius : {1, 5, 17, 64}) {
        PixelSet circle = drawMidpointCircle(0, 0, radius);

        for (const auto& pixel : circle) {
            int x = pixel.first;
            int y = pixel.second;

            CHECK(circle.count({-x, y}) && circle.count({x, -y}) &&
                  circle.count({-x, -y}) && circle.count({y, x}) &&
                  circle.count({-y, x}) && circle.count({y, -x}) &&
                  circle.count({-y, -x}));
        }
    }
}

TEST_CASE("CIR-03 every pixel is within half a pixel of the true radius") {
    double worstError = 0.0;

    for (int radius = 1; radius <= 300; radius++) {
        for (const auto& pixel : drawMidpointCircle(0, 0, radius)) {
            double distance = std::hypot(pixel.first, pixel.second);
            worstError = std::max(worstError, std::fabs(distance - radius));
        }
    }

    CHECK(worstError < 0.5);
}

TEST_CASE("CIR-04 the outline is closed with no gaps") {
    for (int radius = 1; radius <= 120; radius++) {
        PixelSet circle = drawMidpointCircle(0, 0, radius);
        int pixelsWithFewNeighbours = 0;

        for (const auto& pixel : circle) {
            if (countNeighbours(circle, pixel) < 2) {
                pixelsWithFewNeighbours++;
            }
        }

        CHECK_EQ(pixelsWithFewNeighbours, 0);
    }
}

TEST_CASE("CIR-05 the four axis points are always drawn") {
    for (int radius = 1; radius <= 200; radius++) {
        PixelSet circle = drawMidpointCircle(50, -20, radius);

        CHECK(circle.count({50 + radius, -20}) && circle.count({50 - radius, -20}) &&
              circle.count({50, -20 + radius}) && circle.count({50, -20 - radius}));
    }
}

TEST_CASE("CIR-06 radius 0 is the centre point and a negative radius is empty") {
    CHECK_EQ(drawMidpointCircle(7, 3, 0), (PixelSet{{7, 3}}));
    CHECK(drawMidpointCircle(7, 3, -4).empty());
}

TEST_CASE("CIR-07 moving the centre moves every pixel by the same amount") {
    PixelSet atOrigin = drawMidpointCircle(0, 0, 23);
    PixelSet moved = drawMidpointCircle(600, 400, 23);

    PixelSet expected;
    for (const auto& pixel : atOrigin) {
        expected.insert({pixel.first + 600, pixel.second + 400});
    }

    CHECK_EQ(moved, expected);
}

int main() {
    return testing::runAllTests();
}
