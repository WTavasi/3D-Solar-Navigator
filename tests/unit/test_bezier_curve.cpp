// Unit tests for the cubic Bezier curve.

#include <cmath>
#include <vector>

#include "algorithms/curves/bezier_curve.h"
#include "test_framework.h"

namespace {

const double TOLERANCE = 1e-9;

// Control points from the demo program: a symmetric arch.
const Point P0 = {0, 0};
const Point P1 = {2, 8};
const Point P2 = {8, 8};
const Point P3 = {10, 0};

Point lerp(const Point& from, const Point& to, double t) {
    return {from.x + (to.x - from.x) * t, from.y + (to.y - from.y) * t};
}

// de Casteljau's algorithm: repeated linear interpolation. It is a
// different method from the Bernstein form, so it makes a good
// independent reference.
Point deCasteljau(const Point& a, const Point& b, const Point& c,
                  const Point& d, double t) {
    Point ab = lerp(a, b, t);
    Point bc = lerp(b, c, t);
    Point cd = lerp(c, d, t);
    Point abc = lerp(ab, bc, t);
    Point bcd = lerp(bc, cd, t);
    return lerp(abc, bcd, t);
}

// Cross product sign test for a convex polygon given counter-clockwise.
bool insideConvexPolygon(const Point& point, const std::vector<Point>& polygon) {
    for (std::size_t i = 0; i < polygon.size(); i++) {
        const Point& a = polygon[i];
        const Point& b = polygon[(i + 1) % polygon.size()];
        double cross = (b.x - a.x) * (point.y - a.y) -
                       (b.y - a.y) * (point.x - a.x);

        if (cross < -TOLERANCE) {
            return false;
        }
    }

    return true;
}

}  // namespace

TEST_CASE("BEZ-01 the curve starts at P0 and ends at P3") {
    Point start = calculateBezierPoint(0.0, P0, P1, P2, P3);
    Point end = calculateBezierPoint(1.0, P0, P1, P2, P3);

    CHECK_NEAR(start.x, P0.x, TOLERANCE);
    CHECK_NEAR(start.y, P0.y, TOLERANCE);
    CHECK_NEAR(end.x, P3.x, TOLERANCE);
    CHECK_NEAR(end.y, P3.y, TOLERANCE);
}

TEST_CASE("BEZ-02 the midpoint matches the hand-worked value (5, 6)") {
    // B(0.5) = (P0 + 3 P1 + 3 P2 + P3) / 8
    Point middle = calculateBezierPoint(0.5, P0, P1, P2, P3);

    CHECK_NEAR(middle.x, (0 + 6 + 24 + 10) / 8.0, TOLERANCE);
    CHECK_NEAR(middle.y, (0 + 24 + 24 + 0) / 8.0, TOLERANCE);
}

TEST_CASE("BEZ-03 agrees with de Casteljau's algorithm everywhere") {
    const Point a = {100, 450};
    const Point b = {200, 100};
    const Point c = {600, 100};
    const Point d = {700, 450};

    for (int step = 0; step <= 100; step++) {
        double t = step / 100.0;
        Point bernstein = calculateBezierPoint(t, a, b, c, d);
        Point reference = deCasteljau(a, b, c, d, t);

        CHECK_NEAR(bernstein.x, reference.x, 1e-9);
        CHECK_NEAR(bernstein.y, reference.y, 1e-9);
    }
}

TEST_CASE("BEZ-04 moving the control points moves the curve by the same amount") {
    // True because the four Bernstein weights always add up to 1.
    const double moveX = -37.5;
    const double moveY = 120.25;

    for (int step = 0; step <= 20; step++) {
        double t = step / 20.0;
        Point original = calculateBezierPoint(t, P0, P1, P2, P3);
        Point moved = calculateBezierPoint(
            t,
            {P0.x + moveX, P0.y + moveY}, {P1.x + moveX, P1.y + moveY},
            {P2.x + moveX, P2.y + moveY}, {P3.x + moveX, P3.y + moveY});

        CHECK_NEAR(moved.x, original.x + moveX, TOLERANCE);
        CHECK_NEAR(moved.y, original.y + moveY, TOLERANCE);
    }
}

TEST_CASE("BEZ-05 the curve stays inside the convex hull of its control points") {
    // The demo control polygon is convex: list it counter-clockwise.
    const std::vector<Point> hull = {P0, P3, P2, P1};

    for (int step = 0; step <= 200; step++) {
        Point point = calculateBezierPoint(step / 200.0, P0, P1, P2, P3);
        CHECK(insideConvexPolygon(point, hull));
    }
}

TEST_CASE("BEZ-06 end tangents point along the first and last control legs") {
    // B'(0) = 3 (P1 - P0) and B'(1) = 3 (P3 - P2).
    const double h = 1e-6;

    Point start = calculateBezierPoint(0.0, P0, P1, P2, P3);
    Point justAfter = calculateBezierPoint(h, P0, P1, P2, P3);
    CHECK_NEAR((justAfter.x - start.x) / h, 3 * (P1.x - P0.x), 1e-3);
    CHECK_NEAR((justAfter.y - start.y) / h, 3 * (P1.y - P0.y), 1e-3);

    Point end = calculateBezierPoint(1.0, P0, P1, P2, P3);
    Point justBefore = calculateBezierPoint(1.0 - h, P0, P1, P2, P3);
    CHECK_NEAR((end.x - justBefore.x) / h, 3 * (P3.x - P2.x), 1e-3);
    CHECK_NEAR((end.y - justBefore.y) / h, 3 * (P3.y - P2.y), 1e-3);
}

TEST_CASE("BEZ-07 symmetric control points give a symmetric curve") {
    // The demo points are mirror images about x = 5.
    for (int step = 0; step <= 50; step++) {
        double t = step / 50.0;
        Point left = calculateBezierPoint(t, P0, P1, P2, P3);
        Point right = calculateBezierPoint(1.0 - t, P0, P1, P2, P3);

        CHECK_NEAR(left.x + right.x, 10.0, TOLERANCE);
        CHECK_NEAR(left.y, right.y, TOLERANCE);
    }
}

TEST_CASE("BEZ-08 evenly spaced collinear control points give a straight, even line") {
    for (int step = 0; step <= 10; step++) {
        double t = step / 10.0;
        Point point = calculateBezierPoint(t, {0, 0}, {1, 2}, {2, 4}, {3, 6});

        CHECK_NEAR(point.x, 3 * t, TOLERANCE);
        CHECK_NEAR(point.y, 6 * t, TOLERANCE);
    }
}

TEST_CASE("BEZ-09 generateBezierCurve returns steps + 1 points from P0 to P3") {
    std::vector<Point> curve = generateBezierCurve(P0, P1, P2, P3, 20);

    CHECK_EQ(curve.size(), static_cast<std::size_t>(21));
    CHECK_NEAR(curve.front().x, P0.x, TOLERANCE);
    CHECK_NEAR(curve.back().x, P3.x, TOLERANCE);
    CHECK_NEAR(curve[10].x, 5.0, TOLERANCE);
    CHECK_NEAR(curve[10].y, 6.0, TOLERANCE);

    // Fewer than one step is treated as one step.
    CHECK_EQ(generateBezierCurve(P0, P1, P2, P3, 0).size(),
             static_cast<std::size_t>(2));
}

int main() {
    return testing::runAllTests();
}
