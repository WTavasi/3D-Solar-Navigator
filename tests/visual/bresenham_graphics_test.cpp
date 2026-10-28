// Visual test: Bresenham lines with different slopes and directions.
// Output: output/bresenham_output.bmp

#include "algorithms/lines/bresenham_line_drawing.h"
#include "visual_output.h"

int main() {
    RasterCanvas canvas = makeVisualCanvas(800, 600);

    drawPixels(canvas, drawBresenhamLine(100, 100, 700, 100), VISUAL_RED);
    drawPixels(canvas, drawBresenhamLine(100, 150, 700, 450), VISUAL_GREEN);
    drawPixels(canvas, drawBresenhamLine(100, 500, 700, 200), VISUAL_BLUE);
    drawPixels(canvas, drawBresenhamLine(400, 80, 400, 520), VISUAL_ORANGE);
    drawPixels(canvas, drawBresenhamLine(150, 500, 650, 500), VISUAL_MAGENTA);

    // Steep lines drawn right to left.
    drawPixels(canvas, drawBresenhamLine(650, 560, 560, 60), VISUAL_CYAN);
    drawPixels(canvas, drawBresenhamLine(240, 60, 150, 560), VISUAL_YELLOW);

    return saveVisualOutput(canvas, "bresenham_output.bmp");
}
