// Visual test: scan-line filling of a convex and a concave polygon.
// Output: output/polygon_filling_output.bmp

#include <vector>

#include "algorithms/polygons/polygon_filling.h"
#include "visual_output.h"

namespace {

void fillAndOutline(RasterCanvas& canvas, const std::vector<Point>& polygon,
                    const Color& fill) {
    for (const FillSpan& span : computeFillSpans(polygon)) {
        canvas.drawLine(span.startX, span.y, span.endX, span.y, fill);
    }

    drawPolygonOutline(canvas, polygon, VISUAL_WHITE);
}

}  // namespace

int main() {
    RasterCanvas canvas = makeVisualCanvas(800, 600);

    // Convex: the spacecraft shape. Every scan line has one span.
    fillAndOutline(canvas,
                   {{220, 100}, {320, 250}, {280, 420}, {160, 420}, {120, 250}},
                   VISUAL_BLUE);

    // Concave: a star. Scan lines through the points have two or three
    // spans, and spans meeting at an inner corner are joined.
    fillAndOutline(canvas,
                   {{580, 80}, {616, 185}, {730, 194}, {640, 260}, {670, 374},
                    {580, 305}, {490, 374}, {520, 260}, {430, 194}, {544, 185}},
                   VISUAL_ORANGE);

    // Concave: a notch cut into the top edge.
    fillAndOutline(canvas,
                   {{430, 560}, {730, 560}, {730, 420}, {580, 500}, {430, 420}},
                   VISUAL_MAGENTA);

    return saveVisualOutput(canvas, "polygon_filling_output.bmp");
}
