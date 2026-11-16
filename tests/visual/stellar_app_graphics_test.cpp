// Visual test: frames of the Stellar application.
// Output: output/stellar_app_overview.bmp
//         output/stellar_app_selected.bmp
//         output/stellar_app_tour.bmp

#include "app/stellar_app.h"
#include "raster/raster_canvas.h"
#include "visual_output.h"

namespace {

// The app paints its own background, so compare against that colour.
const Color APP_BACKGROUND = {8, 10, 24};

void settle(StellarApp& app, double seconds) {
    for (double t = 0.0; t < seconds; t += 1.0 / 30.0) {
        app.update(1.0 / 30.0);
    }
}

}  // namespace

int main() {
    const int width = 1280;
    const int height = 800;
    int failures = 0;

    {
        StellarApp app(width, height);
        RasterCanvas canvas(width, height);
        app.setDay(80);
        app.render(canvas);
        failures += saveVisualOutput(canvas, "stellar_app_overview.bmp", APP_BACKGROUND);
    }

    {
        StellarApp app(width, height);
        RasterCanvas canvas(width, height);
        app.setDay(80);
        app.setPaused(true);
        app.select("Earth");
        app.mouseWheel(4);
        settle(app, 3.0);
        app.render(canvas);
        failures += saveVisualOutput(canvas, "stellar_app_selected.bmp", APP_BACKGROUND);
    }

    {
        StellarApp app(width, height);
        RasterCanvas canvas(width, height);
        app.setDay(80);
        app.setPaused(true);
        app.startTour();
        settle(app, StellarApp::tourDurationSeconds() * 0.45);
        app.render(canvas);
        failures += saveVisualOutput(canvas, "stellar_app_tour.bmp", APP_BACKGROUND);
    }

    return failures == 0 ? 0 : 1;
}
