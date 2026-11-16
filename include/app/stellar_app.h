#ifndef APP_STELLAR_APP_H
#define APP_STELLAR_APP_H

// Project Stellar: the interactive Solar System application.
//
// StellarApp holds the whole simulation and draws every frame onto a
// RasterCanvas using the project's own algorithms. It knows nothing about
// windows: src/main.cpp opens the window, forwards mouse and keyboard
// input, calls update() and render(), and copies the canvas to the screen.
// That split lets the tests run the application on any computer.
//
// Where each project technique is used:
//   orbit_math.h         planet, moon and spin motion over time
//   Camera               3D view: rotate, zoom, pan, perspective projection
//   Bresenham lines      orbits, rings, outlines, panel borders
//   DDA                  the comet's tail
//   Midpoint circle      the Sun's corona, the selection ring, map dots
//   Midpoint ellipse     orbits on the top-view map
//   Scan-line filling    every planet disc, panel and map background
//   Sutherland-Hodgman   keeps the information panel on screen
//   Polygon transforms   the spinning, pulsing selection marker
//   Bezier curve         the comet's path
//   Catmull-Rom spline   the guided camera tour
//   Bitmap font          all on-screen text

#include <string>
#include <vector>

#include "raster/raster_canvas.h"
#include "scene/camera.h"

enum class BodyKind { Sun, Planet, Moon, Comet };

// A body as it appears this frame.
struct BodyView {
    std::string name;
    BodyKind kind = BodyKind::Planet;
    int planetIndex = -1;       // index into solarSystemPlanets(), or -1
    Vec3 world;                 // centre in world space
    double worldRadius = 0.0;
    Color color = {0, 0, 0};
    ScreenPoint screen;         // projected centre
    int screenRadius = 0;       // radius on screen in pixels
};

class StellarApp {
public:
    StellarApp(int width, int height);

    void resize(int width, int height);

    // Advances the simulation by real seconds: orbits, spin, the guided
    // tour and the camera following a selected body.
    void update(double seconds);

    // Draws the current frame. The canvas should match the app's size.
    void render(RasterCanvas& canvas) const;

    // Input from the window. Wheel notches > 0 zoom in.
    void mouseDown(int x, int y);
    void mouseMove(int x, int y);
    void mouseUp(int x, int y);
    void mouseWheel(int notches);

    // Keys: W A S D pan, + and - speed, space pause, F tour, R reset view,
    // 0 Sun, 1 to 8 planets, 9 Moon, Esc clear selection (code 27).
    void keyDown(char key);

    // State, for the window layer and the tests.
    int width() const { return width_; }
    int height() const { return height_; }
    double day() const { return day_; }
    double daysPerSecond() const;
    bool isPaused() const { return paused_; }
    bool tourActive() const { return touring_; }
    const Camera& camera() const { return camera_; }
    const std::string& selectedName() const { return selected_; }

    // Every body, projected for the current camera, sorted far to near.
    std::vector<BodyView> bodies() const;

    Vec3 planetWorldPosition(int planetIndex) const;
    Vec3 moonWorldPosition() const;
    Vec3 cometWorldPosition() const;
    double spinDegrees(int planetIndex) const;

    // Name of the front-most body under a screen point, or "".
    std::string pick(int x, int y) const;

    // Selects a body by name ("" clears). Returns false for unknown names.
    bool select(const std::string& name);

    void setDay(double day);
    void setPaused(bool paused) { paused_ = paused; }

    static std::vector<double> speedSteps();

    // The guided tour flies the camera through fixed viewpoints along
    // Catmull-Rom splines; any camera input stops it.
    static double tourDurationSeconds();
    void startTour();
    void stopTour();

private:
    int width_;
    int height_;
    Camera camera_;

    double day_ = 0.0;
    double clock_ = 0.0;          // real seconds, for animation
    int speedIndex_ = 2;
    bool paused_ = false;
    std::vector<double> spin_;    // degrees, one per planet

    std::string selected_;
    bool returning_ = false;      // easing back to the Sun

    bool buttonDown_ = false;
    bool dragged_ = false;
    int downX_ = 0;
    int downY_ = 0;
    int lastX_ = 0;
    int lastY_ = 0;

    bool touring_ = false;
    double tourTime_ = 0.0;

    void resetView();
    void applyTour();
    Vec3 selectedWorldPosition(bool& found) const;
};

#endif
