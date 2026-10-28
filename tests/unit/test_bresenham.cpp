// Unit tests for Bresenham's line drawing algorithm.

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <vector>

#include "algorithms/lines/bresenham_line_drawing.h"
#include "algorithms/lines/digital_differential_analyzer.h"
#include "raster/raster_canvas.h"
#include "test_framework.h"

namespace {

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

TEST_CASE("BRE-01 matches the textbook example (20, 10) to (30, 18)") {
    // Slope 0.8: the worked example used in Hearn and Baker.
    std::vector<Pixel> expected = {
        {20, 10}, {21, 11}, {22, 12}, {23, 12}, {24, 13}, {25, 14},
        {26, 15}, {27, 16}, {28, 16}, {29, 17}, {30, 18}
    };

    CHECK_EQ(drawBresenhamLine(20, 10, 30, 18), expected);
}

TEST_CASE("BRE-02 a zero-length line is a single pixel") {
    CHECK_EQ(drawBresenhamLine(-3, 8, -3, 8), (std::vector<Pixel>{{-3, 8}}));
}

TEST_CASE("BRE-03 horizontal, vertical and 45 degree lines are exact") {
    CHECK_EQ(drawBresenhamLine(5, 2, 1, 2),
             (std::vector<Pixel>{{5, 2}, {4, 2}, {3, 2}, {2, 2}, {1, 2}}));

    CHECK_EQ(drawBresenhamLine(0, 0, 0, 3),
             (std::vector<Pixel>{{0, 0}, {0, 1}, {0, 2}, {0, 3}}));

    CHECK_EQ(drawBresenhamLine(2, 2, 5, -1),
             (std::vector<Pixel>{{2, 2}, {3, 1}, {4, 0}, {5, -1}}));
}

TEST_CASE("BRE-04 lines in all eight octants start and end on the endpoints") {
    const int ends[8][2] = {
        {9, 4}, {4, 9}, {-4, 9}, {-9, 4},
        {-9, -4}, {-4, -9}, {4, -9}, {9, -4}
    };

    for (const auto& end : ends) {
        std::vector<Pixel> line = drawBresenhamLine(0, 0, end[0], end[1]);

        CHECK_EQ(line.front(), (Pixel{0, 0}));
        CHECK_EQ(line.back(), (Pixel{end[0], end[1]}));
    }
}

TEST_CASE("BRE-05 one pixel per step along the major axis, with no gaps") {
    for (int x2 = -15; x2 <= 15; x2++) {
        for (int y2 = -15; y2 <= 15; y2++) {
            std::vector<Pixel> line = drawBresenhamLine(0, 0, x2, y2);
            int steps = std::max(std::abs(x2), std::abs(y2));

            CHECK_EQ(static_cast<int>(line.size()), steps + 1);

            for (std::size_t i = 1; i < line.size(); i++) {
                int stepX = std::abs(line[i].x - line[i - 1].x);
                int stepY = std::abs(line[i].y - line[i - 1].y);

                CHECK(stepX <= 1 && stepY <= 1);
                CHECK(stepX + stepY >= 1);
            }
        }
    }
}

TEST_CASE("BRE-06 every pixel is within half a pixel of the true line") {
    double worstError = 0.0;
    const int starts[] = {-10, 0, 7};

    for (int x1 : starts) {
        for (int y1 : starts) {
            for (int x2 = -25; x2 <= 25; x2++) {
                for (int y2 = -25; y2 <= 25; y2++) {
                    if (x1 == x2 && y1 == y2) {
                        continue;
                    }

                    for (const Pixel& pixel :
                         drawBresenhamLine(x1, y1, x2, y2)) {
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

TEST_CASE("BRE-07 agrees with DDA to within one pixel at every step") {
    // The two algorithms may round exact half-pixel ties differently,
    // but they must never be more than one pixel apart.
    for (int x2 = -20; x2 <= 20; x2++) {
        for (int y2 = -20; y2 <= 20; y2++) {
            std::vector<Pixel> bresenham = drawBresenhamLine(0, 0, x2, y2);
            std::vector<Pixel> dda = drawDDA(0, 0, x2, y2);

            CHECK_EQ(bresenham.size(), dda.size());

            if (bresenham.size() != dda.size()) {
                continue;
            }

            for (std::size_t i = 0; i < dda.size(); i++) {
                CHECK(std::abs(bresenham[i].x - dda[i].x) <= 1);
                CHECK(std::abs(bresenham[i].y - dda[i].y) <= 1);
            }
        }
    }
}

TEST_CASE("BRE-08 RasterCanvas::drawLine draws exactly the Bresenham pixels") {
    // The canvas has its own copy of the algorithm, so check they agree.
    const Color ink = {255, 0, 0};
    const int lines[][4] = {
        {5, 5, 45, 20}, {40, 3, 8, 28}, {25, 2, 25, 27}, {2, 15, 47, 15},
        {47, 28, 3, 1}
    };

    for (const auto& line : lines) {
        RasterCanvas canvas(50, 30);
        canvas.drawLine(line[0], line[1], line[2], line[3], ink);

        std::vector<Pixel> expected =
            drawBresenhamLine(line[0], line[1], line[2], line[3]);
        std::vector<Pixel> drawn;

        for (int y = 0; y < canvas.getHeight(); y++) {
            for (int x = 0; x < canvas.getWidth(); x++) {
                if (canvas.getPixel(x, y) == ink) {
                    drawn.push_back({x, y});
                }
            }
        }

        std::sort(expected.begin(), expected.end());
        std::sort(drawn.begin(), drawn.end());

        CHECK_EQ(drawn, expected);
    }
}

int main() {
    return testing::runAllTests();
}
