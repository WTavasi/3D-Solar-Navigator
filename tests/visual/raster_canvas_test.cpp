// Visual test: the raster canvas draws lines and saves a BMP.
// Output: output/raster_canvas_test.bmp

#include "raster/raster_canvas.h"
#include "visual_output.h"

int main() {
    RasterCanvas canvas = makeVisualCanvas(400, 300);

    // A horizontal white line, a diagonal red line and a vertical blue line.
    canvas.drawLine(50, 50, 350, 50, VISUAL_WHITE);
    canvas.drawLine(50, 80, 350, 250, VISUAL_RED);
    canvas.drawLine(200, 80, 200, 250, VISUAL_BLUE);

    return saveVisualOutput(canvas, "raster_canvas_test.bmp");
}
