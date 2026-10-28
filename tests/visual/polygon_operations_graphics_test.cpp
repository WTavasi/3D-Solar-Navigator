// Visual test: one polygon translated, scaled and rotated.
// Output: output/polygon_operations_output.bmp

#include <vector>

#include "algorithms/polygons/polygon_representation_and_operations.h"
#include "visual_output.h"

namespace {

Point centroid(const std::vector<Point>& polygon) {
    Point sum = {0, 0};

    for (const Point& vertex : polygon) {
        sum.x += vertex.x;
        sum.y += vertex.y;
    }

    return {sum.x / polygon.size(), sum.y / polygon.size()};
}

}  // namespace

int main() {
    RasterCanvas canvas = makeVisualCanvas(800, 600);

    const std::vector<Point> original = {
        {100, 150}, {220, 100}, {280, 220}, {170, 280}
    };

    // Red: the original polygon.
    drawPolygonOutline(canvas, original, VISUAL_RED);

    // Green: translated 400 pixels to the right.
    std::vector<Point> translated = original;
    translatePolygon(translated, 400, 0);
    drawPolygonOutline(canvas, translated, VISUAL_GREEN);

    // Blue: moved down, then scaled by 1.4 about its own centre (marked).
    std::vector<Point> scaled = original;
    translatePolygon(scaled, 0, 270);
    const Point scaleCentre = centroid(scaled);
    scalePolygon(scaled, 1.4, scaleCentre);
    drawPolygonOutline(canvas, scaled, VISUAL_BLUE);
    markPoint(canvas, scaleCentre, 3, VISUAL_WHITE);

    // Yellow: moved down and right, then rotated 45 degrees about its
    // centre (marked). The dim outline shows it before rotation.
    std::vector<Point> rotated = original;
    translatePolygon(rotated, 400, 270);
    drawPolygonOutline(canvas, rotated, VISUAL_GREY);
    const Point rotationCentre = centroid(rotated);
    rotatePolygon(rotated, 45, rotationCentre);
    drawPolygonOutline(canvas, rotated, VISUAL_YELLOW);
    markPoint(canvas, rotationCentre, 3, VISUAL_WHITE);

    return saveVisualOutput(canvas, "polygon_operations_output.bmp");
}
