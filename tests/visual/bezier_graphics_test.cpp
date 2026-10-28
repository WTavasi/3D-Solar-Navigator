// Visual test: a cubic Bezier curve and its control polygon.
// Output: output/bezier_output.bmp
//
// Grey: the control polygon. White circles: the four control points.
// Cyan: the curve, which starts and ends on the outer control points
// and is pulled towards, but does not pass through, the inner two.

#include <vector>

#include "algorithms/curves/bezier_curve.h"
#include "visual_output.h"

int main() {
    RasterCanvas canvas = makeVisualCanvas(800, 600);

    const Point start = {100, 450};
    const Point firstControl = {200, 100};
    const Point secondControl = {600, 100};
    const Point end = {700, 450};

    drawPolyline(canvas, {start, firstControl, secondControl, end}, VISUAL_GREY);

    std::vector<Point> curve = generateBezierCurve(
        start, firstControl, secondControl, end, 200);
    drawPolyline(canvas, curve, VISUAL_CYAN);

    for (const Point& point : {start, firstControl, secondControl, end}) {
        markPoint(canvas, point, 5, VISUAL_WHITE);
    }

    return saveVisualOutput(canvas, "bezier_output.bmp");
}
