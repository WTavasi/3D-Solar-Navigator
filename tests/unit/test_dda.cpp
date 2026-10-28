// Unit tests for the Digital Differential Analyzer (DDA) line algorithm.

#include <cmath>
#include <cstdlib>
#include <vector>

#include "algorithms/lines/digital_differential_analyzer.h"
#include "test_framework.h"

namespace {

// Distance, along the minor axis, between a pixel and the ideal line.
double minorAxisError(const Pixel& pixel, int x1, int y1, int x2, int y2) {
    int dx = x2 - x1;
    int dy = y2 - y1;

    if (std::abs(dx) >= std::abs(dy)) {
        double idealY = y1 + static_cast<double>(pixel.x - x1) * dy / dx;
        return std::fabs(pixel.y - idealY);
    }

    double idealX = x1 + static_cast<double>(pixel.y - y1) * dx / dy;
    return std::fabs(pixel.x - idealX);
}

}  // namespace

TEST_CASE("DDA-01 matches the hand-worked example (2, 3) to (10, 7)") {
    // steps = 8, x increment = 1, y increment = 0.5; halves round up.
    std::vector<Pixel> expected = {
        {2, 3}, {3, 4}, {4, 4}, {5, 5}, {6, 5},
        {7, 6}, {8, 6}, {9, 7}, {10, 7}
    };

    CHECK_EQ(drawDDA(2, 3, 10, 7), expected);
}

TEST_CASE("DDA-02 a zero-length line is a single pixel") {
    CHECK_EQ(drawDDA(4, 9, 4, 9), (std::vector<Pixel>{{4, 9}}));
}

TEST_CASE("DDA-03 horizontal, vertical and 45 degree lines are exact") {
    CHECK_EQ(drawDDA(1, 5, 5, 5),
             (std::vector<Pixel>{{1, 5}, {2, 5}, {3, 5}, {4, 5}, {5, 5}}));

    CHECK_EQ(drawDDA(3, 4, 3, 0),
             (std::vector<Pixel>{{3, 4}, {3, 3}, {3, 2}, {3, 1}, {3, 0}}));

    CHECK_EQ(drawDDA(0, 0, -3, 3),
             (std::vector<Pixel>{{0, 0}, {-1, 1}, {-2, 2}, {-3, 3}}));
}

TEST_CASE("DDA-04 lines in all eight octants start and end on the endpoints") {
    const int ends[8][2] = {
        {9, 4}, {4, 9}, {-4, 9}, {-9, 4},
        {-9, -4}, {-4, -9}, {4, -9}, {9, -4}
    };

    for (const auto& end : ends) {
        std::vector<Pixel> line = drawDDA(0, 0, end[0], end[1]);

        CHECK_EQ(line.front(), (Pixel{0, 0}));
        CHECK_EQ(line.back(), (Pixel{end[0], end[1]}));
    }
}

TEST_CASE("DDA-05 one pixel per step along the major axis, with no gaps") {
    for (int x2 = -15; x2 <= 15; x2++) {
        for (int y2 = -15; y2 <= 15; y2++) {
            std::vector<Pixel> line = drawDDA(0, 0, x2, y2);
            int steps = std::max(std::abs(x2), std::abs(y2));

            CHECK_EQ(static_cast<int>(line.size()), steps + 1);

            for (std::size_t i = 1; i < line.size(); i++) {
                int stepX = std::abs(line[i].x - line[i - 1].x);
                int stepY = std::abs(line[i].y - line[i - 1].y);

                // Neighbouring pixels touch (8-connected) and never repeat.
                CHECK(stepX <= 1 && stepY <= 1);
                CHECK(stepX + stepY >= 1);
            }
        }
    }
}

TEST_CASE("DDA-06 every pixel is within half a pixel of the true line") {
    double worstError = 0.0;

    const int starts[] = {-10, 0, 7};

    for (int x1 : starts) {
        for (int y1 : starts) {
            for (int x2 = -25; x2 <= 25; x2++) {
                for (int y2 = -25; y2 <= 25; y2++) {
                    if (x1 == x2 && y1 == y2) {
                        continue;
                    }

                    for (const Pixel& pixel : drawDDA(x1, y1, x2, y2)) {
                        worstError = std::max(
                            worstError,
                            minorAxisError(pixel, x1, y1, x2, y2));
                    }
                }
            }
        }
    }

    CHECK(worstError <= 0.5 + 1e-9);
}

TEST_CASE("DDA-07 works with negative and off-screen coordinates") {
    std::vector<Pixel> line = drawDDA(-6, -2, -2, -4);

    CHECK_EQ(line.size(), static_cast<std::size_t>(5));
    CHECK_EQ(line.front(), (Pixel{-6, -2}));
    CHECK_EQ(line.back(), (Pixel{-2, -4}));
}

int main() {
    return testing::runAllTests();
}
