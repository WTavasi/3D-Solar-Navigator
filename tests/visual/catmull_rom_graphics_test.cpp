// Visual test: a Catmull-Rom spline through six control points.
// Output: output/catmull_rom_output.bmp
//
// Grey: straight lines between the control points. White circles: the
// control points. Yellow: the spline, which passes through every one of
// them, including the first and the last.

#include <vector>

#include "algorithms/curves/catmull_rom_spline.h"
#include "visual_output.h"

int main() {
    RasterCanvas canvas = makeVisualCanvas(800, 600);

    const std::vector<Point> points = {
        {80, 350}, {180, 150}, {300, 250}, {420, 100}, {550, 300}, {700, 180}
    };

    drawPolyline(canvas, points, VISUAL_GREY);

    // 100 samples per segment.
    drawPolyline(canvas, generateCatmullRomSpline(points, 100), VISUAL_YELLOW);

    for (const Point& point : points) {
        markPoint(canvas, point, 5, VISUAL_WHITE);
    }

    return saveVisualOutput(canvas, "catmull_rom_output.bmp");
}
