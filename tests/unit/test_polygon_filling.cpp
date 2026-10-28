// Unit tests for scan-line polygon filling.

#include <algorithm>
#include <cmath>
#include <vector>

#include "algorithms/polygons/polygon_filling.h"
#include "test_framework.h"

// Declared at global scope (next to FillSpan) so that templates such as
// std::equal and the test printers can find them.
std::ostream& operator<<(std::ostream& stream, const FillSpan& span) {
    return stream << "y=" << span.y << " x=" << span.startX << ".."
                  << span.endX;
}

bool operator==(const FillSpan& first, const FillSpan& second) {
    return first.y == second.y && first.startX == second.startX &&
           first.endX == second.endX;
}

namespace {

const double TOLERANCE = 1e-9;

double polygonArea(const std::vector<Point>& polygon) {
    double sum = 0.0;

    for (std::size_t i = 0; i < polygon.size(); i++) {
        const Point& current = polygon[i];
        const Point& next = polygon[(i + 1) % polygon.size()];
        sum += current.x * next.y - next.x * current.y;
    }

    return std::fabs(sum) / 2.0;
}

double polygonPerimeter(const std::vector<Point>& polygon) {
    double total = 0.0;

    for (std::size_t i = 0; i < polygon.size(); i++) {
        const Point& current = polygon[i];
        const Point& next = polygon[(i + 1) % polygon.size()];
        total += std::hypot(next.x - current.x, next.y - current.y);
    }

    return total;
}

bool onSegment(double x, double y, const Point& a, const Point& b) {
    double cross = (b.x - a.x) * (y - a.y) - (b.y - a.y) * (x - a.x);

    return std::fabs(cross) <= TOLERANCE &&
           x >= std::min(a.x, b.x) - TOLERANCE &&
           x <= std::max(a.x, b.x) + TOLERANCE &&
           y >= std::min(a.y, b.y) - TOLERANCE &&
           y <= std::max(a.y, b.y) + TOLERANCE;
}

// Even-odd ray casting; points on an edge count as inside.
bool insideOrOnPolygon(double x, double y, const std::vector<Point>& polygon) {
    bool inside = false;

    for (std::size_t i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
        const Point& a = polygon[i];
        const Point& b = polygon[j];

        if (onSegment(x, y, a, b)) {
            return true;
        }

        if ((a.y > y) != (b.y > y)) {
            double crossingX = a.x + (y - a.y) * (b.x - a.x) / (b.y - a.y);

            if (x < crossingX) {
                inside = !inside;
            }
        }
    }

    return inside;
}

int countPixels(const std::vector<FillSpan>& spans) {
    int count = 0;

    for (const FillSpan& span : spans) {
        count += span.endX - span.startX + 1;
    }

    return count;
}

std::vector<std::vector<Point>> testPolygons() {
    return {
        // The spacecraft shape from the graphics test.
        {{400, 100}, {500, 250}, {460, 420}, {340, 420}, {300, 250}},
        // Concave: a notch cut into the top edge.
        {{0, 0}, {40, 0}, {40, 40}, {20, 16}, {0, 40}},
        // Concave star-like shape.
        {{50, 0}, {62, 35}, {100, 38}, {70, 60}, {80, 98},
         {50, 75}, {20, 98}, {30, 60}, {0, 38}, {38, 35}},
        // Non-integer vertices.
        {{1.5, 2.25}, {30.75, 4.5}, {25.2, 28.9}, {3.3, 20.1}}
    };
}

}  // namespace

TEST_CASE("FIL-01 the demo pentagon produces the hand-worked spans") {
    std::vector<Point> pentagon = {{2, 2}, {8, 2}, {8, 7}, {5, 10}, {2, 7}};

    std::vector<FillSpan> expected = {
        {2, 2, 8}, {3, 2, 8}, {4, 2, 8}, {5, 2, 8}, {6, 2, 8},
        {7, 2, 8}, {8, 3, 7}, {9, 4, 6}
    };

    CHECK_EQ(computeFillSpans(pentagon), expected);
}

TEST_CASE("FIL-02 a rectangle fills its bottom row but not its top row") {
    // Lower endpoints are included and upper endpoints excluded, so a
    // 6 x 5 rectangle fills rows 2 to 6. Both x ends are included.
    std::vector<Point> rectangle = {{2, 2}, {8, 2}, {8, 7}, {2, 7}};

    std::vector<FillSpan> expected = {
        {2, 2, 8}, {3, 2, 8}, {4, 2, 8}, {5, 2, 8}, {6, 2, 8}
    };

    CHECK_EQ(computeFillSpans(rectangle), expected);
}

TEST_CASE("FIL-03 a concave notch splits a scan line into two spans") {
    std::vector<Point> notched = {{0, 0}, {10, 0}, {10, 10}, {5, 4}, {0, 10}};

    std::vector<FillSpan> rowSeven;
    for (const FillSpan& span : computeFillSpans(notched)) {
        if (span.y == 7) {
            rowSeven.push_back(span);
        }
    }

    // Edges cross y = 7 at x = 0, 2.5, 7.5 and 10.
    CHECK_EQ(rowSeven, (std::vector<FillSpan>{{7, 0, 2}, {7, 8, 10}}));
}

TEST_CASE("FIL-04 every filled pixel lies inside or on the polygon") {
    for (const std::vector<Point>& polygon : testPolygons()) {
        int outside = 0;

        for (const FillSpan& span : computeFillSpans(polygon)) {
            for (int x = span.startX; x <= span.endX; x++) {
                if (!insideOrOnPolygon(x, span.y, polygon)) {
                    outside++;
                }
            }
        }

        CHECK_EQ(outside, 0);
    }
}

TEST_CASE("FIL-05 spans on a scan line are ordered and never overlap") {
    for (const std::vector<Point>& polygon : testPolygons()) {
        std::vector<FillSpan> spans = computeFillSpans(polygon);

        for (std::size_t i = 0; i < spans.size(); i++) {
            CHECK(spans[i].startX <= spans[i].endX);

            if (i > 0 && spans[i].y == spans[i - 1].y) {
                CHECK(spans[i].startX > spans[i - 1].endX);
            }

            if (i > 0) {
                CHECK(spans[i].y >= spans[i - 1].y);
            }
        }
    }
}

TEST_CASE("FIL-06 the number of filled pixels is close to the polygon area") {
    // Rounding at the edges can add or remove at most about one pixel
    // per unit of boundary length.
    for (const std::vector<Point>& polygon : testPolygons()) {
        double area = polygonArea(polygon);
        int filled = countPixels(computeFillSpans(polygon));

        CHECK(std::fabs(filled - area) <= polygonPerimeter(polygon));
    }
}

TEST_CASE("FIL-07 moving the polygon by whole pixels moves the spans with it") {
    std::vector<Point> shape = testPolygons()[2];
    std::vector<Point> moved = shape;

    for (Point& vertex : moved) {
        vertex.x += 13;
        vertex.y -= 7;
    }

    std::vector<FillSpan> expected = computeFillSpans(shape);
    for (FillSpan& span : expected) {
        span.y -= 7;
        span.startX += 13;
        span.endX += 13;
    }

    CHECK_EQ(computeFillSpans(moved), expected);
}

TEST_CASE("FIL-08 fewer than three vertices fills nothing") {
    CHECK(computeFillSpans({}).empty());
    CHECK(computeFillSpans({{1, 1}}).empty());
    CHECK(computeFillSpans({{1, 1}, {9, 9}}).empty());
}

TEST_CASE("FIL-09 vertex order (clockwise or not) gives the same fill") {
    std::vector<Point> shape = testPolygons()[0];
    std::vector<Point> reversed(shape.rbegin(), shape.rend());

    CHECK_EQ(computeFillSpans(reversed), computeFillSpans(shape));
}

TEST_CASE("FIL-10 spans that meet at a concave vertex are joined") {
    // Regression test: on the scan line through the notch vertex (20, 16)
    // the fill used to return 0..20 and 20..40, filling x = 20 twice.
    std::vector<Point> notched = {{0, 0}, {40, 0}, {40, 40}, {20, 16}, {0, 40}};

    std::vector<FillSpan> rowSixteen;
    for (const FillSpan& span : computeFillSpans(notched)) {
        if (span.y == 16) {
            rowSixteen.push_back(span);
        }
    }

    CHECK_EQ(rowSixteen, (std::vector<FillSpan>{{16, 0, 40}}));
}

int main() {
    return testing::runAllTests();
}
