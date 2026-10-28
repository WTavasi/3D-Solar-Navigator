// Unit tests for the uniform Catmull-Rom spline.

#include <cmath>
#include <vector>

#include "algorithms/curves/catmull_rom_spline.h"
#include "test_framework.h"

namespace {

const double TOLERANCE = 1e-9;

// Control points from the demo program.
const std::vector<Point> DEMO_POINTS = {
    {0, 0}, {2, 5}, {5, 3}, {8, 7}, {10, 2}
};

// Derivative of one segment at parameter t, by central differences.
Point segmentTangent(const Point& p0, const Point& p1, const Point& p2,
                     const Point& p3, double t) {
    const double h = 1e-6;
    Point after = calculateCatmullRomPoint(p0, p1, p2, p3, t + h);
    Point before = calculateCatmullRomPoint(p0, p1, p2, p3, t - h);

    return {(after.x - before.x) / (2 * h), (after.y - before.y) / (2 * h)};
}

}  // namespace

TEST_CASE("CRS-01 the spline passes through every control point") {
    const int steps = 10;
    std::vector<Point> curve = generateCatmullRomSpline(DEMO_POINTS, steps);

    for (std::size_t k = 0; k < DEMO_POINTS.size(); k++) {
        const Point& onCurve = curve[k * steps];

        CHECK_NEAR(onCurve.x, DEMO_POINTS[k].x, TOLERANCE);
        CHECK_NEAR(onCurve.y, DEMO_POINTS[k].y, TOLERANCE);
    }
}

TEST_CASE("CRS-02 a segment runs from its second point to its third") {
    const Point p0 = {80, 350};
    const Point p1 = {180, 150};
    const Point p2 = {300, 250};
    const Point p3 = {420, 100};

    Point start = calculateCatmullRomPoint(p0, p1, p2, p3, 0.0);
    Point end = calculateCatmullRomPoint(p0, p1, p2, p3, 1.0);

    CHECK_NEAR(start.x, p1.x, TOLERANCE);
    CHECK_NEAR(start.y, p1.y, TOLERANCE);
    CHECK_NEAR(end.x, p2.x, TOLERANCE);
    CHECK_NEAR(end.y, p2.y, TOLERANCE);
}

TEST_CASE("CRS-03 the midpoint matches the hand-worked value (3.4375, 4.0625)") {
    // C(0.5) = (-P0 + 9 P1 + 9 P2 - P3) / 16
    Point middle = calculateCatmullRomPoint(DEMO_POINTS[0], DEMO_POINTS[1],
                                            DEMO_POINTS[2], DEMO_POINTS[3],
                                            0.5);

    CHECK_NEAR(middle.x, 55.0 / 16.0, TOLERANCE);
    CHECK_NEAR(middle.y, 65.0 / 16.0, TOLERANCE);
}

TEST_CASE("CRS-04 the tangent at a control point is half the chord around it") {
    // At P1 the tangent is (P2 - P0) / 2.
    const Point& p0 = DEMO_POINTS[0];
    const Point& p1 = DEMO_POINTS[1];
    const Point& p2 = DEMO_POINTS[2];
    const Point& p3 = DEMO_POINTS[3];

    Point tangent = segmentTangent(p0, p1, p2, p3, 0.0);

    CHECK_NEAR(tangent.x, (p2.x - p0.x) / 2.0, 1e-5);
    CHECK_NEAR(tangent.y, (p2.y - p0.y) / 2.0, 1e-5);
}

TEST_CASE("CRS-05 neighbouring segments join smoothly (same tangent)") {
    const std::vector<Point> points = {
        {80, 350}, {180, 150}, {300, 250}, {420, 100}, {550, 300}, {700, 180}
    };

    for (std::size_t i = 0; i + 4 < points.size(); i++) {
        Point endOfSegment = segmentTangent(points[i], points[i + 1],
                                            points[i + 2], points[i + 3], 1.0);
        Point startOfNext = segmentTangent(points[i + 1], points[i + 2],
                                           points[i + 3], points[i + 4], 0.0);

        CHECK_NEAR(endOfSegment.x, startOfNext.x, 1e-4);
        CHECK_NEAR(endOfSegment.y, startOfNext.y, 1e-4);
    }
}

TEST_CASE("CRS-06 evenly spaced points on a line give a straight, even curve") {
    const std::vector<Point> line = {{0, 0}, {1, 2}, {2, 4}, {3, 6}};

    for (int step = 0; step <= 10; step++) {
        double t = step / 10.0;
        Point point = calculateCatmullRomPoint(line[0], line[1], line[2],
                                               line[3], t);

        CHECK_NEAR(point.x, 1 + t, TOLERANCE);
        CHECK_NEAR(point.y, 2 + 2 * t, TOLERANCE);
    }
}

TEST_CASE("CRS-07 the number of generated points is (n - 1) * steps + 1") {
    CHECK_EQ(generateCatmullRomSpline(DEMO_POINTS, 10).size(),
             static_cast<std::size_t>(4 * 10 + 1));
    CHECK_EQ(generateCatmullRomSpline(DEMO_POINTS, 100).size(),
             static_cast<std::size_t>(4 * 100 + 1));

    // Fewer than one step is treated as one step.
    CHECK_EQ(generateCatmullRomSpline(DEMO_POINTS, 0).size(),
             DEMO_POINTS.size());
}

TEST_CASE("CRS-08 fewer than two points give no curve; two give a straight segment") {
    CHECK(generateCatmullRomSpline({}, 10).empty());
    CHECK(generateCatmullRomSpline({{4, 4}}, 10).empty());

    std::vector<Point> segment = generateCatmullRomSpline({{0, 0}, {10, 5}}, 8);
    double previousX = -1.0;

    for (const Point& point : segment) {
        // Every point lies on the line y = x / 2, moving steadily forward.
        CHECK_NEAR(point.y, point.x / 2.0, TOLERANCE);
        CHECK(point.x >= previousX);
        previousX = point.x;
    }

    CHECK_NEAR(segment.back().x, 10.0, TOLERANCE);
}

TEST_CASE("CRS-09 moving the control points moves the curve by the same amount") {
    std::vector<Point> moved = DEMO_POINTS;
    for (Point& point : moved) {
        point.x += 250;
        point.y -= 75;
    }

    std::vector<Point> original = generateCatmullRomSpline(DEMO_POINTS, 12);
    std::vector<Point> shifted = generateCatmullRomSpline(moved, 12);

    CHECK_EQ(shifted.size(), original.size());

    for (std::size_t i = 0; i < original.size() && i < shifted.size(); i++) {
        CHECK_NEAR(shifted[i].x, original[i].x + 250, 1e-9);
        CHECK_NEAR(shifted[i].y, original[i].y - 75, 1e-9);
    }
}

int main() {
    return testing::runAllTests();
}
