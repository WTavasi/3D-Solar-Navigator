// Visual test: DDA lines from the centre in every direction.
// Output: output/dda_output.bmp

#include <vector>

#include "algorithms/lines/digital_differential_analyzer.h"
#include "visual_output.h"

int main() {
    RasterCanvas canvas = makeVisualCanvas(500, 400);

    const int centerX = 250;
    const int centerY = 200;

    // Rays along both axes and through all eight octants, so every kind
    // of slope and direction is covered.
    struct Ray {
        int dx;
        int dy;
        Color color;
    };

    const std::vector<Ray> rays = {
        {180, 0, VISUAL_WHITE},    {170, -70, VISUAL_RED},
        {70, -170, VISUAL_ORANGE}, {0, -180, VISUAL_YELLOW},
        {-70, -170, VISUAL_GREEN}, {-170, -70, VISUAL_CYAN},
        {-180, 0, VISUAL_BLUE},    {-170, 70, VISUAL_MAGENTA},
        {-70, 170, VISUAL_RED},    {0, 180, VISUAL_ORANGE},
        {70, 170, VISUAL_YELLOW},  {170, 70, VISUAL_GREEN}
    };

    for (const Ray& ray : rays) {
        drawPixels(canvas,
                   drawDDA(centerX, centerY, centerX + ray.dx, centerY + ray.dy),
                   ray.color);
    }

    return saveVisualOutput(canvas, "dda_output.bmp");
}
