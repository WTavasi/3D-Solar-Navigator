// Visual test: concentric midpoint circles.
// Output: output/circle_graphics_test.bmp

#include "algorithms/shapes/midpoint_circle.h"
#include "visual_output.h"

namespace {

void drawCircle(RasterCanvas& canvas, int radius, const Color& color) {
    for (const auto& point : drawMidpointCircle(250, 250, radius)) {
        canvas.setPixel(point.first, point.second, color);
    }
}

}  // namespace

int main() {
    RasterCanvas canvas = makeVisualCanvas(500, 500);

    drawCircle(canvas, 200, VISUAL_WHITE);
    drawCircle(canvas, 140, VISUAL_RED);
    drawCircle(canvas, 100, VISUAL_GREEN);
    drawCircle(canvas, 60, VISUAL_BLUE);
    drawCircle(canvas, 25, VISUAL_YELLOW);
    drawCircle(canvas, 0, VISUAL_WHITE);   // radius 0: the centre pixel

    return saveVisualOutput(canvas, "circle_graphics_test.bmp");
}
