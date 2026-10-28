// Unit tests for polygon representation and the 2D transformations
// (translation, scaling and rotation about a fixed point).

#include <cmath>
#include <vector>

#include "algorithms/polygons/polygon_representation_and_operations.h"
#include "test_framework.h"

namespace {

const double TOLERANCE = 1e-9;

// Signed area by the shoelace formula; positive for counter-clockwise
// vertices in a y-up coordinate system.
double signedArea(const std::vector<Point>& polygon) {
    double sum = 0.0;

    for (std::size_t i = 0; i < polygon.size(); i++) {
        const Point& current = polygon[i];
        const Point& next = polygon[(i + 1) % polygon.size()];
        sum += current.x * next.y - next.x * current.y;
    }

    return sum / 2.0;
}

double distance(const Point& first, const Point& second) {
    return std::hypot(first.x - second.x, first.y - second.y);
}

void checkSamePolygon(const std::vector<Point>& actual,
                      const std::vector<Point>& expected) {
    CHECK_EQ(actual.size(), expected.size());

    for (std::size_t i = 0; i < actual.size() && i < expected.size(); i++) {
        CHECK_NEAR(actual[i].x, expected[i].x, TOLERANCE);
        CHECK_NEAR(actual[i].y, expected[i].y, TOLERANCE);
    }
}

std::vector<Point> sampleHexagon() {
    return {{1, 0}, {4, -1}, {6, 2}, {5, 5}, {2, 6}, {-1, 3}};
}

}  // namespace

TEST_CASE("POL-01 reproduces the hand-worked translate, scale, rotate example") {
    std::vector<Point> triangle = {{2, 2}, {6, 2}, {4, 5}};

    translatePolygon(triangle, 3, 2);
    checkSamePolygon(triangle, {{5, 4}, {9, 4}, {7, 7}});

    scalePolygon(triangle, 2.0, {3, 2});
    checkSamePolygon(triangle, {{7, 6}, {15, 6}, {11, 12}});

    rotatePolygon(triangle, 90, {3, 2});
    checkSamePolygon(triangle, {{-1, 6}, {-1, 14}, {-7, 10}});
}

TEST_CASE("POL-02 translating and translating back restores the polygon") {
    std::vector<Point> polygon = sampleHexagon();

    translatePolygon(polygon, 0, 0);
    checkSamePolygon(polygon, sampleHexagon());

    translatePolygon(polygon, -12.5, 40.25);
    translatePolygon(polygon, 12.5, -40.25);
    checkSamePolygon(polygon, sampleHexagon());
}

TEST_CASE("POL-03 scaling keeps the fixed point and scales distances to it") {
    const Point fixedPoint = {2, 1};
    std::vector<Point> original = sampleHexagon();
    original.push_back(fixedPoint);

    std::vector<Point> scaled = original;
    scalePolygon(scaled, 3.0, fixedPoint);

    CHECK_NEAR(scaled.back().x, fixedPoint.x, TOLERANCE);
    CHECK_NEAR(scaled.back().y, fixedPoint.y, TOLERANCE);

    for (std::size_t i = 0; i < original.size(); i++) {
        CHECK_NEAR(distance(scaled[i], fixedPoint),
                   3.0 * distance(original[i], fixedPoint), TOLERANCE);
    }

    scalePolygon(scaled, 1.0 / 3.0, fixedPoint);
    checkSamePolygon(scaled, original);
}

TEST_CASE("POL-04 a 90 degree rotation turns the +x axis onto the +y axis") {
    std::vector<Point> point = {{1, 0}};

    rotatePolygon(point, 90, {0, 0});
    checkSamePolygon(point, {{0, 1}});

    // About a fixed point other than the origin.
    std::vector<Point> offset = {{5, 3}};
    rotatePolygon(offset, 90, {4, 3});
    checkSamePolygon(offset, {{4, 4}});
}

TEST_CASE("POL-05 rotation preserves side lengths and undoes cleanly") {
    std::vector<Point> polygon = sampleHexagon();
    const Point pivot = {-3, 7};

    rotatePolygon(polygon, 37.5, pivot);

    std::vector<Point> original = sampleHexagon();

    for (std::size_t i = 0; i < polygon.size(); i++) {
        std::size_t next = (i + 1) % polygon.size();
        CHECK_NEAR(distance(polygon[i], polygon[next]),
                   distance(original[i], original[next]), TOLERANCE);
    }

    rotatePolygon(polygon, -37.5, pivot);
    checkSamePolygon(polygon, original);

    rotatePolygon(polygon, 360, pivot);
    checkSamePolygon(polygon, original);
}

TEST_CASE("POL-06 area is kept by translation and rotation, times s^2 by scaling") {
    const double area = signedArea(sampleHexagon());
    CHECK(area > 0);

    std::vector<Point> moved = sampleHexagon();
    translatePolygon(moved, 100, -50);
    CHECK_NEAR(signedArea(moved), area, TOLERANCE);

    std::vector<Point> turned = sampleHexagon();
    rotatePolygon(turned, 123, {10, 10});
    CHECK_NEAR(signedArea(turned), area, 1e-7);

    std::vector<Point> grown = sampleHexagon();
    scalePolygon(grown, 2.5, {0, 0});
    CHECK_NEAR(signedArea(grown), area * 2.5 * 2.5, 1e-7);
}

TEST_CASE("POL-07 transformations keep the number and order of vertices") {
    std::vector<Point> polygon = sampleHexagon();

    translatePolygon(polygon, 1, 1);
    scalePolygon(polygon, -1.0, {0, 0});   // a half turn
    rotatePolygon(polygon, 45, {2, 2});

    CHECK_EQ(polygon.size(), sampleHexagon().size());
    // A half turn plus a rotation keeps the winding direction.
    CHECK(signedArea(polygon) > 0);
}

TEST_CASE("POL-08 an empty polygon is left empty") {
    std::vector<Point> empty;

    translatePolygon(empty, 4, 4);
    scalePolygon(empty, 2, {0, 0});
    rotatePolygon(empty, 30, {0, 0});

    CHECK(empty.empty());
}

int main() {
    return testing::runAllTests();
}
