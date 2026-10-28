// Visual test: Sutherland-Hodgman clipping against a rectangular window.
// Output: output/polygon_clipping_output.bmp
//
// White: the clipping window. Red: the original polygon.
// Green (filled): the part of the polygon inside the window.

#include <vector>

#include "algorithms/polygons/polygon_clipping.h"
#include "algorithms/polygons/polygon_filling.h"
#include "visual_output.h"

int main() {
    RasterCanvas canvas = makeVisualCanvas(800, 600);

    const double minimumX = 200;
    const double minimumY = 150;
    const double maximumX = 600;
    const double maximumY = 450;

    const std::vector<Point> original = {
        {100, 250}, {350, 80}, {700, 250}, {500, 520}, {180, 450}
    };

    std::vector<Point> clipped = sutherlandHodgmanClip(
        original, minimumX, minimumY, maximumX, maximumY);

    // Fill the clipped polygon with the project's scan-line fill.
    for (const FillSpan& span : computeFillSpans(clipped)) {
        canvas.drawLine(span.startX, span.y, span.endX, span.y,
                        VISUAL_DARK_GREEN);
    }

    drawPolygonOutline(canvas, original, VISUAL_RED);
    drawPolygonOutline(canvas,
                       {{minimumX, minimumY}, {maximumX, minimumY},
                        {maximumX, maximumY}, {minimumX, maximumY}},
                       VISUAL_WHITE);
    drawPolygonOutline(canvas, clipped, VISUAL_GREEN);

    return saveVisualOutput(canvas, "polygon_clipping_output.bmp");
}
