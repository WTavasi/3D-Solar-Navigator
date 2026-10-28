// Unit tests for Sutherland-Hodgman polygon clipping.

#include <cmath>
#include <sstream>
#include <vector>

#include "algorithms/polygons/polygon_clipping.h"
#include "test_framework.h"

namespace {

const double TOLERANCE = 1e-9;

double signedArea(const std::vector<Point>& polygon) {
    double sum = 0.0;

    for (std::size_t i = 0; i < polygon.size(); i++) {
        const Point& current = polygon[i];
        const Point& next = polygon[(i + 1) % polygon.size()];
        sum += current.x * next.y - next.x * current.y;
    }

    return sum / 2.0;
}

bool samePoint(const Point& first, const Point& second) {
    return std::fabs(first.x - second.x) <= TOLERANCE &&
           std::fabs(first.y - second.y) <= TOLERANCE;
}

// Sutherland-Hodgman can emit the same vertex twice when the polygon
// passes exactly through a window corner. Those zero-length edges do not
// change the shape, so tests compare polygons without them.
std::vector<Point> withoutRepeatedVertices(const std::vector<Point>& polygon) {
    std::vector<Point> result;

    for (const Point& vertex : polygon) {
        if (result.empty() || !samePoint(result.back(), vertex)) {
            result.push_back(vertex);
        }
    }

    while (result.size() > 1 && samePoint(result.front(), result.back())) {
        result.pop_back();
    }

    return result;
}

// True when both polygons have the same vertices in the same cyclic
// order. Which vertex comes first does not change the shape.
bool samePolygon(const std::vector<Point>& first,
                 const std::vector<Point>& second) {
    if (first.size() != second.size()) {
        return false;
    }

    for (std::size_t offset = 0; offset < first.size(); offset++) {
        bool allMatch = true;

        for (std::size_t i = 0; i < first.size() && allMatch; i++) {
            allMatch = samePoint(first[(i + offset) % first.size()],
                                 second[i]);
        }

        if (allMatch) {
            return true;
        }
    }

    return first.empty();
}

void checkSamePolygon(const std::vector<Point>& actual,
                      const std::vector<Point>& expected) {
    bool same = samePolygon(actual, expected);

    if (!same) {
        std::ostringstream message;
        message << "polygons differ\n"
                << "        actual:   " << actual << "\n"
                << "        expected: " << expected;
        testing::reportFailure(__FILE__, __LINE__, message.str());
    }
}

bool insideWindow(const Point& point, double minimumX, double minimumY,
                  double maximumX, double maximumY) {
    return point.x >= minimumX - TOLERANCE && point.x <= maximumX + TOLERANCE &&
           point.y >= minimumY - TOLERANCE && point.y <= maximumY + TOLERANCE;
}

}  // namespace

TEST_CASE("CLP-01 the demo pentagon clips to exactly the window rectangle") {
    // Both roof edges pass exactly through the top corners of the window.
    std::vector<Point> pentagon = {{2, 2}, {8, 2}, {8, 7}, {5, 10}, {2, 7}};

    std::vector<Point> clipped = sutherlandHodgmanClip(pentagon, 3, 3, 7, 8);

    checkSamePolygon(withoutRepeatedVertices(clipped),
                     {{3, 3}, {7, 3}, {7, 8}, {3, 8}});
    CHECK_NEAR(signedArea(clipped), 4.0 * 5.0, TOLERANCE);
}

TEST_CASE("CLP-02 a polygon fully inside the window is returned unchanged") {
    std::vector<Point> inside = {{4, 4}, {6, 4}, {6, 6}, {5, 7}, {4, 6}};

    checkSamePolygon(sutherlandHodgmanClip(inside, 0, 0, 10, 10), inside);
}

TEST_CASE("CLP-03 a polygon fully outside the window is removed") {
    const std::vector<std::vector<Point>> outside = {
        {{-9, 2}, {-5, 2}, {-7, 6}},     // left
        {{15, 2}, {19, 2}, {17, 6}},     // right
        {{2, -9}, {6, -9}, {4, -5}},     // below minimum y
        {{2, 15}, {6, 15}, {4, 19}}      // above maximum y
    };

    for (const std::vector<Point>& polygon : outside) {
        CHECK(sutherlandHodgmanClip(polygon, 0, 0, 10, 10).empty());
    }

    CHECK(sutherlandHodgmanClip({}, 0, 0, 10, 10).empty());
}

TEST_CASE("CLP-04 a polygon covering the whole window clips to the window") {
    std::vector<Point> large = {{-50, -50}, {50, -50}, {50, 50}, {-50, 50}};

    std::vector<Point> clipped = sutherlandHodgmanClip(large, 2, 3, 8, 9);

    checkSamePolygon(withoutRepeatedVertices(clipped),
                     {{2, 3}, {8, 3}, {8, 9}, {2, 9}});
}

TEST_CASE("CLP-05 cutting off one corner gives the hand-worked pentagon") {
    // Triangle x + y <= 10 clipped to the square [0, 6] x [0, 6] removes
    // the corner triangle (4, 6), (6, 4), (6, 6) of area 2.
    std::vector<Point> triangle = {{0, 0}, {10, 0}, {0, 10}};

    std::vector<Point> clipped = sutherlandHodgmanClip(triangle, 0, 0, 6, 6);

    checkSamePolygon(withoutRepeatedVertices(clipped),
                     {{0, 6}, {0, 0}, {6, 0}, {6, 4}, {4, 6}});
    CHECK_NEAR(signedArea(clipped), 36.0 - 2.0, TOLERANCE);
}

TEST_CASE("CLP-06 clipped vertices stay inside the window and area never grows") {
    const std::vector<std::vector<Point>> polygons = {
        {{100, 250}, {350, 80}, {700, 250}, {500, 520}, {180, 450}},
        {{0, 0}, {800, 100}, {650, 600}, {50, 500}},
        {{400, 100}, {500, 250}, {460, 420}, {340, 420}, {300, 250}},
        {{150, 300}, {400, 120}, {650, 300}, {400, 480}}
    };
    const double windowArea = (600.0 - 200.0) * (450.0 - 150.0);

    for (const std::vector<Point>& polygon : polygons) {
        std::vector<Point> clipped =
            sutherlandHodgmanClip(polygon, 200, 150, 600, 450);

        for (const Point& vertex : clipped) {
            CHECK(insideWindow(vertex, 200, 150, 600, 450));
        }

        double clippedArea = std::fabs(signedArea(clipped));
        CHECK(clippedArea <= std::fabs(signedArea(polygon)) + TOLERANCE);
        CHECK(clippedArea <= windowArea + TOLERANCE);
    }
}

TEST_CASE("CLP-07 vertices exactly on the window edge count as inside") {
    std::vector<Point> window = {{1, 1}, {5, 1}, {5, 4}, {1, 4}};

    checkSamePolygon(sutherlandHodgmanClip(window, 1, 1, 5, 4), window);
}

TEST_CASE("CLP-08 clipping keeps the winding direction of the input") {
    std::vector<Point> counterClockwise = {{-2, -2}, {12, 3}, {4, 14}};
    std::vector<Point> clockwise(counterClockwise.rbegin(),
                                 counterClockwise.rend());

    CHECK(signedArea(sutherlandHodgmanClip(counterClockwise, 0, 0, 10, 10)) > 0);
    CHECK(signedArea(sutherlandHodgmanClip(clockwise, 0, 0, 10, 10)) < 0);
}

TEST_CASE("CLP-09 known limitation: a concave polygon split in two stays one outline") {
    // A U shape whose two arms are cut apart by the window. The true
    // result is two 3 x 4 rectangles (area 24). Sutherland-Hodgman
    // returns one polygon joined by edges along y = 4. The area is still
    // right, but drawing its outline would show an extra line.
    std::vector<Point> uShape = {
        {0, 0}, {9, 0}, {9, 9}, {6, 9}, {6, 3}, {3, 3}, {3, 9}, {0, 9}
    };

    std::vector<Point> clipped = sutherlandHodgmanClip(uShape, -1, 4, 10, 8);

    CHECK_EQ(clipped.size(), static_cast<std::size_t>(8));
    CHECK_NEAR(std::fabs(signedArea(clipped)), 24.0, TOLERANCE);

    int verticesOnCutLine = 0;
    for (const Point& vertex : clipped) {
        if (std::fabs(vertex.y - 4.0) <= TOLERANCE) {
            verticesOnCutLine++;
        }
    }
    CHECK_EQ(verticesOnCutLine, 4);
}

int main() {
    return testing::runAllTests();
}
