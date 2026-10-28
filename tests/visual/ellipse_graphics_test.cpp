// Visual test: midpoint ellipses, including a very flat one.
// Output: output/ellipse_graphics_test.bmp

#include "algorithms/shapes/midpoint_ellipse.h"
#include "visual_output.h"

namespace {

void drawEllipse(RasterCanvas& canvas, int radiusX, int radiusY,
                 const Color& color) {
    for (const auto& point : drawMidpointEllipse(250, 250, radiusX, radiusY)) {
        canvas.setPixel(point.first, point.second, color);
    }
}

}  // namespace

int main() {
    RasterCanvas canvas = makeVisualCanvas(500, 500);

    drawEllipse(canvas, 150, 80, VISUAL_RED);
    drawEllipse(canvas, 110, 55, VISUAL_GREEN);
    drawEllipse(canvas, 70, 30, VISUAL_BLUE);
    drawEllipse(canvas, 40, 200, VISUAL_ORANGE);   // taller than wide

    // Very flat: horizontal radius more than 16 times the vertical one.
    // The tips must reach x = 250 +/- 220 (white ticks just below them).
    drawEllipse(canvas, 220, 6, VISUAL_CYAN);
    canvas.drawLine(30, 255, 30, 263, VISUAL_WHITE);
    canvas.drawLine(470, 255, 470, 263, VISUAL_WHITE);

    return saveVisualOutput(canvas, "ellipse_graphics_test.bmp");
}
