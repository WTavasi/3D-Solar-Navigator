#ifndef VISUAL_OUTPUT_H
#define VISUAL_OUTPUT_H

// Shared helpers for the visual tests in tests/visual. Each visual test
// draws with the project's own algorithms and saves a BMP into output/
// as evidence. The test fails (exit code 1) if nothing was drawn or the
// image could not be saved, so a broken visual test can no longer report
// success.

#include <cmath>
#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

#include "algorithms/geometry.h"
#include "algorithms/shapes/midpoint_circle.h"
#include "raster/raster_canvas.h"

// A dark "space" background so white, yellow and cyan lines show up.
constexpr Color VISUAL_BACKGROUND = {12, 14, 32};

constexpr Color VISUAL_WHITE = {240, 240, 240};
constexpr Color VISUAL_GREY = {110, 115, 140};
constexpr Color VISUAL_RED = {255, 85, 85};
constexpr Color VISUAL_GREEN = {90, 220, 120};
constexpr Color VISUAL_DARK_GREEN = {30, 90, 50};
constexpr Color VISUAL_BLUE = {90, 150, 255};
constexpr Color VISUAL_YELLOW = {255, 220, 0};
constexpr Color VISUAL_CYAN = {0, 220, 255};
constexpr Color VISUAL_ORANGE = {255, 150, 40};
constexpr Color VISUAL_MAGENTA = {230, 90, 230};

inline RasterCanvas makeVisualCanvas(int width, int height) {
    return RasterCanvas(width, height,
                        VISUAL_BACKGROUND.red,
                        VISUAL_BACKGROUND.green,
                        VISUAL_BACKGROUND.blue);
}

inline int roundToPixel(double value) {
    return static_cast<int>(std::lround(value));
}

inline void drawPixels(RasterCanvas& canvas, const std::vector<Pixel>& pixels,
                       const Color& color) {
    for (const Pixel& pixel : pixels) {
        canvas.setPixel(pixel.x, pixel.y, color);
    }
}

// Joins consecutive points with straight lines.
inline void drawPolyline(RasterCanvas& canvas, const std::vector<Point>& points,
                         const Color& color) {
    for (std::size_t i = 1; i < points.size(); i++) {
        canvas.drawLine(roundToPixel(points[i - 1].x), roundToPixel(points[i - 1].y),
                        roundToPixel(points[i].x), roundToPixel(points[i].y),
                        color);
    }
}

// Joins consecutive vertices and closes the shape.
inline void drawPolygonOutline(RasterCanvas& canvas,
                               const std::vector<Point>& polygon,
                               const Color& color) {
    if (polygon.size() < 2) {
        return;
    }

    std::vector<Point> closed = polygon;
    closed.push_back(polygon.front());
    drawPolyline(canvas, closed, color);
}

// Marks a point with a small midpoint circle.
inline void markPoint(RasterCanvas& canvas, const Point& point, int radius,
                      const Color& color) {
    for (const auto& pixel :
         drawMidpointCircle(roundToPixel(point.x), roundToPixel(point.y), radius)) {
        canvas.setPixel(pixel.first, pixel.second, color);
    }
}

inline int countDrawnPixels(const RasterCanvas& canvas, const Color& background) {
    int count = 0;

    for (int y = 0; y < canvas.getHeight(); y++) {
        for (int x = 0; x < canvas.getWidth(); x++) {
            if (canvas.getPixel(x, y) != background) {
                count++;
            }
        }
    }

    return count;
}

// Saves the image into the project's output/ folder (CMake passes its
// path as STELLAR_OUTPUT_DIR) and returns the exit code for main().
inline int saveVisualOutput(const RasterCanvas& canvas,
                            const std::string& fileName,
                            const Color& background = VISUAL_BACKGROUND) {
#ifdef STELLAR_OUTPUT_DIR
    const std::filesystem::path directory = STELLAR_OUTPUT_DIR;
#else
    const std::filesystem::path directory = "output";
#endif

    if (countDrawnPixels(canvas, background) == 0) {
        std::cerr << "FAILED: nothing was drawn for " << fileName << "\n";
        return 1;
    }

    std::error_code error;
    std::filesystem::create_directories(directory, error);

    const std::string path = (directory / fileName).string();

    if (!canvas.saveBMP(path)) {
        std::cerr << "FAILED: could not save " << path << "\n";
        return 1;
    }

    std::cout << "Saved " << path << "\n";
    return 0;
}

#endif
