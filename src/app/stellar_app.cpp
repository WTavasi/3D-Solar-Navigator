#include "app/stellar_app.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "algorithms/curves/bezier_curve.h"
#include "algorithms/curves/catmull_rom_spline.h"
#include "algorithms/geometry.h"
#include "algorithms/lines/digital_differential_analyzer.h"
#include "algorithms/orbit/orbit_math.h"
#include "algorithms/polygons/polygon_clipping.h"
#include "algorithms/polygons/polygon_filling.h"
#include "algorithms/polygons/polygon_representation_and_operations.h"
#include "algorithms/shapes/midpoint_circle.h"
#include "algorithms/shapes/midpoint_ellipse.h"
#include "app/bitmap_font.h"
#include "app/planet_facts.h"
#include "scene/solar_system_scene.h"

namespace {

constexpr double PI = 3.14159265358979323846;

// Sizes in world units (the same units as the scene's orbit radii).
constexpr double PLANET_SIZE_SCALE = 1.4;
constexpr double SUN_WORLD_RADIUS = 30.0;
constexpr double MOON_WORLD_RADIUS = 3.5;
constexpr double MOON_ORBIT_GAP = 14.0;
constexpr double MOON_PERIOD_DAYS = 27.3;
constexpr double COMET_PERIOD_DAYS = 400.0;
constexpr double COMET_WORLD_RADIUS = 3.0;

constexpr double START_YAW = 0.0;
constexpr double START_PITCH = 35.0;
constexpr double START_DISTANCE = 950.0;

// Spin faster than this many degrees per real second is shown at this
// rate, so a quickly turning planet still looks like it is turning.
constexpr double MAX_VISIBLE_SPIN = 180.0;

constexpr double TOUR_SECONDS = 25.0;

constexpr Color BACKGROUND = {8, 10, 24};
constexpr Color ORBIT = {55, 60, 88};
constexpr Color ORBIT_SELECTED = {130, 140, 190};
constexpr Color BELT = {120, 120, 132};
constexpr Color TEXT = {230, 235, 245};
constexpr Color TEXT_DIM = {150, 160, 185};
constexpr Color ACCENT = {255, 200, 60};
constexpr Color PANEL = {18, 22, 46};
constexpr Color PANEL_BORDER = {110, 125, 190};
constexpr Color COMET_COLOR = {200, 230, 255};
constexpr Color MOON_COLOR = {190, 190, 185};

// ---------------------------------------------------------------- vectors

Vec3 add(Vec3 a, Vec3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
Vec3 sub(Vec3 a, Vec3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
Vec3 scale(Vec3 a, double s) { return {a.x * s, a.y * s, a.z * s}; }
double dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

Vec3 cross(Vec3 a, Vec3 b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

Vec3 normalize(Vec3 v) {
    double length = std::sqrt(dot(v, v));
    return length < 1e-12 ? Vec3{0, 0, 0} : scale(v, 1.0 / length);
}

// The camera's own axes, built the same way as Camera::project.
struct CameraBasis {
    Vec3 right;
    Vec3 up;
    Vec3 back;   // points from the scene towards the camera
};

CameraBasis basisOf(const Camera& camera) {
    Vec3 forward = normalize(sub(camera.target, camera.getPosition()));
    Vec3 right = normalize(cross(forward, {0, 1, 0}));
    Vec3 up = cross(right, forward);
    return {right, up, scale(forward, -1.0)};
}

// ---------------------------------------------------------------- colour

Color mix(Color a, Color b, double t) {
    t = std::clamp(t, 0.0, 1.0);
    return {static_cast<std::uint8_t>(a.red + (b.red - a.red) * t),
            static_cast<std::uint8_t>(a.green + (b.green - a.green) * t),
            static_cast<std::uint8_t>(a.blue + (b.blue - a.blue) * t)};
}

Color shade(Color c, double intensity) {
    intensity = std::clamp(intensity, 0.0, 1.0);
    return {static_cast<std::uint8_t>(c.red * intensity),
            static_cast<std::uint8_t>(c.green * intensity),
            static_cast<std::uint8_t>(c.blue * intensity)};
}

// Repeatable pseudo-random value in [0, 1) for a grid cell.
double hash01(int i, int j) {
    std::uint32_t h = static_cast<std::uint32_t>(i) * 374761393u +
                      static_cast<std::uint32_t>(j) * 668265263u;
    h = (h ^ (h >> 13)) * 1274126177u;
    h ^= h >> 16;
    return (h & 0xFFFFFF) / 16777216.0;
}

// Smooth value noise over longitude (wraps every 16 cells) and latitude.
double valueNoise(double lon, double lat, int cellsAround) {
    double u = (lon / (2.0 * PI) + 1.0) * cellsAround;
    double v = (lat / PI + 0.5) * (cellsAround / 2);
    int i0 = static_cast<int>(std::floor(u));
    int j0 = static_cast<int>(std::floor(v));
    double fu = u - i0;
    double fv = v - j0;
    fu = fu * fu * (3 - 2 * fu);
    fv = fv * fv * (3 - 2 * fv);
    auto cell = [&](int i, int j) {
        return hash01(((i % cellsAround) + cellsAround) % cellsAround, j);
    };
    double top = cell(i0, j0) + (cell(i0 + 1, j0) - cell(i0, j0)) * fu;
    double bottom = cell(i0, j0 + 1) + (cell(i0 + 1, j0 + 1) - cell(i0, j0 + 1)) * fu;
    return top + (bottom - top) * fv;
}

double wrapAngle(double radians) {
    return std::atan2(std::sin(radians), std::cos(radians));
}

// Texture mapping: the surface colour at a latitude and longitude
// (radians), for each body. Longitude already includes the spin.
Color surfaceColor(const std::string& name, Color base, double lat, double lon) {
    if (name == "Jupiter") {
        double band = 0.5 + 0.5 * std::sin(lat * 14.0);
        Color c = mix(base, {240, 220, 185}, band * 0.65);
        double du = wrapAngle(lon) / 0.35;
        double dv = (lat + 0.38) / 0.11;
        if (du * du + dv * dv < 1.0) {
            c = {200, 90, 60};   // Great Red Spot
        }
        return c;
    }

    if (name == "Saturn") {
        double band = 0.5 + 0.5 * std::sin(lat * 11.0);
        return mix(base, {250, 235, 185}, band * 0.4);
    }

    if (name == "Earth") {
        if (std::fabs(lat) > 1.18) {
            return {235, 240, 245};   // ice caps
        }
        double land = valueNoise(lon, lat, 16) * 0.7 + valueNoise(lon * 2, lat * 2, 16) * 0.3;
        return land > 0.56 ? Color{70, 145, 75} : base;
    }

    if (name == "Mars") {
        if (std::fabs(lat) > 1.3) {
            return {240, 235, 230};
        }
        return valueNoise(lon, lat, 12) > 0.6 ? Color{150, 55, 40} : base;
    }

    if (name == "Mercury" || name == "Moon") {
        double craters = valueNoise(lon * 3, lat * 3, 16);
        return mix(base, shade(base, 0.6), craters > 0.62 ? 0.8 : 0.0);
    }

    if (name == "Venus") {
        return mix(base, {250, 235, 200}, 0.25 + 0.25 * std::sin(lat * 6.0 + lon));
    }

    if (name == "Uranus" || name == "Neptune") {
        return mix(base, shade(base, 0.75), 0.5 + 0.5 * std::sin(lat * 5.0));
    }

    return base;
}

// ---------------------------------------------------------------- drawing

// A circle as a polygon, so it can be filled by the scan-line algorithm.
std::vector<Point> circlePolygon(double cx, double cy, double radius) {
    int sides = std::clamp(static_cast<int>(radius * 2), 12, 72);
    std::vector<Point> polygon;

    for (int i = 0; i < sides; i++) {
        double a = 2.0 * PI * i / sides;
        polygon.push_back({cx + radius * std::cos(a), cy + radius * std::sin(a)});
    }

    return polygon;
}

void fillPolygonSolid(RasterCanvas& canvas, const std::vector<Point>& polygon,
                      const Color& color) {
    for (const FillSpan& span : computeFillSpans(polygon)) {
        canvas.drawLine(span.startX, span.y, span.endX, span.y, color);
    }
}

void outlinePolygon(RasterCanvas& canvas, const std::vector<Point>& polygon,
                    const Color& color) {
    for (std::size_t i = 0; i < polygon.size(); i++) {
        const Point& a = polygon[i];
        const Point& b = polygon[(i + 1) % polygon.size()];
        canvas.drawLine(static_cast<int>(std::lround(a.x)), static_cast<int>(std::lround(a.y)),
                        static_cast<int>(std::lround(b.x)), static_cast<int>(std::lround(b.y)),
                        color);
    }
}

void drawCircle(RasterCanvas& canvas, int cx, int cy, int radius, const Color& color) {
    for (const auto& p : drawMidpointCircle(cx, cy, radius)) {
        canvas.setPixel(p.first, p.second, color);
    }
}

// A lit, textured sphere: the disc is filled with the scan-line algorithm
// and every pixel is coloured from its surface point (texture) and how
// directly that point faces the Sun (diffuse lighting).
void drawSphere(RasterCanvas& canvas, const BodyView& body, const CameraBasis& basis,
                double spinDegrees) {
    const int r = body.screenRadius;
    const double cx = body.screen.x;
    const double cy = body.screen.y;

    Vec3 toSun = normalize(scale(body.world, -1.0));
    const double spin = spinDegrees * PI / 180.0;

    for (const FillSpan& span : computeFillSpans(circlePolygon(cx, cy, r + 0.5))) {
        for (int x = span.startX; x <= span.endX; x++) {
            double dx = (x - cx) / r;
            double dy = (span.y - cy) / r;
            double d2 = dx * dx + dy * dy;

            if (d2 > 1.0) {
                double length = std::sqrt(d2);
                dx /= length;
                dy /= length;
                d2 = 1.0;
            }

            double dz = std::sqrt(1.0 - d2);

            if (body.kind == BodyKind::Sun) {
                canvas.setPixel(x, span.y, mix({255, 248, 210}, {255, 175, 30}, d2));
                continue;
            }

            Vec3 normal = add(add(scale(basis.right, dx), scale(basis.up, -dy)),
                              scale(basis.back, dz));
            double lat = std::asin(std::clamp(normal.y, -1.0, 1.0));
            double lon = std::atan2(normal.z, normal.x) - spin;

            Color surface = r <= 2 ? body.color : surfaceColor(body.name, body.color, lat, lon);
            double light = 0.14 + 0.86 * std::max(0.0, dot(normal, toSun));
            canvas.setPixel(x, span.y, shade(surface, light));
        }
    }
}

// Saturn's rings: circles tilted 20 degrees, split into the half behind
// the planet and the half in front of it.
void drawRings(RasterCanvas& canvas, const Camera& camera, const BodyView& body,
               bool frontHalf) {
    const double tilt = 20.0 * PI / 180.0;
    const int samples = 120;

    for (double factor : {1.45, 1.75, 2.05}) {
        double radius = body.worldRadius * factor;
        ScreenPoint previous;

        for (int i = 0; i <= samples; i++) {
            double a = 2.0 * PI * i / samples;
            Vec3 point = add(body.world, {radius * std::cos(a),
                                          radius * std::sin(a) * std::sin(tilt),
                                          radius * std::sin(a) * std::cos(tilt)});
            ScreenPoint current = camera.project(point, canvas.getWidth(), canvas.getHeight());

            bool inFront = current.depth < body.screen.depth;
            if (i > 0 && previous.visible && current.visible && inFront == frontHalf) {
                Color color = factor > 1.9 ? Color{170, 150, 105} : RING_COLOR;
                canvas.drawLine(static_cast<int>(previous.x), static_cast<int>(previous.y),
                                static_cast<int>(current.x), static_cast<int>(current.y),
                                color);
            }
            previous = current;
        }
    }
}

// A flat loop in 3D (an orbit), projected and joined with Bresenham lines.
void drawLoop(RasterCanvas& canvas, const Camera& camera, Vec3 center,
              double radiusX, double radiusZ, const Color& color) {
    const int samples = 160;
    ScreenPoint previous;

    for (int i = 0; i <= samples; i++) {
        double a = 2.0 * PI * i / samples;
        Vec3 point = add(center, {radiusX * std::cos(a), 0, radiusZ * std::sin(a)});
        ScreenPoint current = camera.project(point, canvas.getWidth(), canvas.getHeight());

        if (i > 0 && previous.visible && current.visible) {
            canvas.drawLine(static_cast<int>(previous.x), static_cast<int>(previous.y),
                            static_cast<int>(current.x), static_cast<int>(current.y), color);
        }
        previous = current;
    }
}

// Splits text into lines of at most maxChars, breaking at spaces.
std::vector<std::string> wrapText(const std::string& text, std::size_t maxChars) {
    std::vector<std::string> lines;
    std::string line;
    std::string word;

    auto flushWord = [&]() {
        if (word.empty()) {
            return;
        }
        if (!line.empty() && line.size() + 1 + word.size() > maxChars) {
            lines.push_back(line);
            line.clear();
        }
        line += (line.empty() ? "" : " ") + word;
        word.clear();
    };

    for (char c : text) {
        if (c == ' ') {
            flushWord();
        } else {
            word += c;
        }
    }
    flushWord();

    if (!line.empty()) {
        lines.push_back(line);
    }
    return lines;
}

std::string formatNumber(double value) {
    char buffer[32];
    if (value == std::floor(value)) {
        std::snprintf(buffer, sizeof(buffer), "%.0f", value);
    } else {
        std::snprintf(buffer, sizeof(buffer), "%.2f", value);
    }
    return buffer;
}

// The comet's closed path: two cubic Bezier curves that meet smoothly at
// the point nearest the Sun and the point farthest from it.
Vec3 cometPathPoint(double t) {
    // (x, z) in the orbital plane, plus a separate curve for height (y).
    const Point a0 = {-70, 0}, a1 = {-200, 160}, a2 = {500, 520}, a3 = {650, 300};
    const Point b0 = a3, b1 = {800, 80}, b2 = {60, -160}, b3 = a0;
    const Point h0 = {10, 0}, h1 = {20, 0}, h2 = {-20, 0}, h3 = {-80, 0};
    const Point g0 = h3, g1 = {-140, 0}, g2 = {0, 0}, g3 = h0;

    t = t - std::floor(t);   // 0..1 for a full lap
    if (t < 0.5) {
        double s = t * 2.0;
        Point p = calculateBezierPoint(s, a0, a1, a2, a3);
        Point h = calculateBezierPoint(s, h0, h1, h2, h3);
        return {p.x, h.x, p.y};
    }

    double s = (t - 0.5) * 2.0;
    Point p = calculateBezierPoint(s, b0, b1, b2, b3);
    Point h = calculateBezierPoint(s, g0, g1, g2, g3);
    return {p.x, h.x, p.y};
}

// Tour keyframes: (yaw, pitch) and (distance, 0).
const std::vector<Point>& tourAngles() {
    static const std::vector<Point> keys = {
        {0, 35}, {50, 20}, {120, 12}, {200, 60}, {280, 85}, {360, 35}};
    return keys;
}

const std::vector<Point>& tourDistances() {
    static const std::vector<Point> keys = {
        {950, 0}, {760, 0}, {520, 0}, {900, 0}, {1300, 0}, {950, 0}};
    return keys;
}

}  // namespace

// ================================================================= app

StellarApp::StellarApp(int width, int height)
    : width_(width), height_(height), spin_(solarSystemPlanets().size(), 0.0) {
    resetView();
}

void StellarApp::resize(int width, int height) {
    width_ = std::max(1, width);
    height_ = std::max(1, height);
}

std::vector<double> StellarApp::speedSteps() {
    return {0.25, 1.0, 5.0, 20.0, 60.0, 180.0, 365.0};
}

double StellarApp::daysPerSecond() const {
    return speedSteps().at(static_cast<std::size_t>(speedIndex_));
}

double StellarApp::tourDurationSeconds() {
    return TOUR_SECONDS;
}

void StellarApp::resetView() {
    camera_.target = {0, 0, 0};
    camera_.yaw = START_YAW;
    camera_.pitch = START_PITCH;
    camera_.distance = START_DISTANCE;
    camera_.fov = 60.0;
}

void StellarApp::setDay(double day) {
    day_ = day;
}

Vec3 StellarApp::planetWorldPosition(int planetIndex) const {
    std::vector<Planet> planets = solarSystemPlanets();
    std::vector<double> periods = solarSystemOrbitalPeriods();
    const Planet& planet = planets.at(static_cast<std::size_t>(planetIndex));

    double angle = orbit::advanceAngle(planet.angle, day_,
                                       periods.at(static_cast<std::size_t>(planetIndex)));
    double radians = orbit::degreesToRadians(angle);
    return {planet.orbitX * std::cos(radians), 0.0, planet.orbitY * std::sin(radians)};
}

// A transform within a transform: the Moon circles Earth, which circles
// the Sun. Its world position is Earth's position plus its own orbit.
Vec3 StellarApp::moonWorldPosition() const {
    const Vec3 earth = planetWorldPosition(2);
    const double earthRadius = solarSystemPlanets().at(2).radius * PLANET_SIZE_SCALE;
    const double orbitRadius = earthRadius + MOON_ORBIT_GAP;
    double angle = orbit::degreesToRadians(orbit::advanceAngle(0.0, day_, MOON_PERIOD_DAYS));
    return add(earth, {orbitRadius * std::cos(angle), 0.0, orbitRadius * std::sin(angle)});
}

Vec3 StellarApp::cometWorldPosition() const {
    return cometPathPoint(day_ / COMET_PERIOD_DAYS);
}

double StellarApp::spinDegrees(int planetIndex) const {
    return spin_.at(static_cast<std::size_t>(planetIndex));
}

std::vector<BodyView> StellarApp::bodies() const {
    std::vector<BodyView> list;
    std::vector<Planet> planets = solarSystemPlanets();

    BodyView sun;
    sun.name = "Sun";
    sun.kind = BodyKind::Sun;
    sun.world = {0, 0, 0};
    sun.worldRadius = SUN_WORLD_RADIUS;
    sun.color = SUN_COLOR;
    list.push_back(sun);

    for (std::size_t i = 0; i < planets.size(); i++) {
        BodyView body;
        body.name = planets[i].name;
        body.kind = BodyKind::Planet;
        body.planetIndex = static_cast<int>(i);
        body.world = planetWorldPosition(static_cast<int>(i));
        body.worldRadius = planets[i].radius * PLANET_SIZE_SCALE;
        body.color = planets[i].color;
        list.push_back(body);
    }

    BodyView moon;
    moon.name = "Moon";
    moon.kind = BodyKind::Moon;
    moon.world = moonWorldPosition();
    moon.worldRadius = MOON_WORLD_RADIUS;
    moon.color = MOON_COLOR;
    list.push_back(moon);

    BodyView comet;
    comet.name = "Comet";
    comet.kind = BodyKind::Comet;
    comet.world = cometWorldPosition();
    comet.worldRadius = COMET_WORLD_RADIUS;
    comet.color = COMET_COLOR;
    list.push_back(comet);

    const double focal = (height_ / 2.0) / std::tan(camera_.fov * PI / 360.0);

    for (BodyView& body : list) {
        body.screen = camera_.project(body.world, width_, height_);
        double depth = std::max(body.screen.depth, 1.0);
        body.screenRadius = std::max(2, static_cast<int>(body.worldRadius * focal / depth));
    }

    // Painter's algorithm order: farthest first.
    std::stable_sort(list.begin(), list.end(), [](const BodyView& a, const BodyView& b) {
        return a.screen.depth > b.screen.depth;
    });

    return list;
}

std::string StellarApp::pick(int x, int y) const {
    std::string best;
    double bestDepth = 1e18;

    for (const BodyView& body : bodies()) {
        if (!body.screen.visible || body.kind == BodyKind::Comet) {
            continue;
        }

        double reach = std::max(8, body.screenRadius + 4);
        double distance = std::hypot(x - body.screen.x, y - body.screen.y);

        if (distance <= reach && body.screen.depth < bestDepth) {
            best = body.name;
            bestDepth = body.screen.depth;
        }
    }

    return best;
}

bool StellarApp::select(const std::string& name) {
    if (name.empty()) {
        if (!selected_.empty()) {
            returning_ = true;   // ease the view back to the Sun
        }
        selected_.clear();
        return true;
    }

    if (factsFor(name).name.empty()) {
        return false;
    }

    selected_ = name;
    returning_ = false;
    return true;
}

Vec3 StellarApp::selectedWorldPosition(bool& found) const {
    found = false;
    for (const BodyView& body : bodies()) {
        if (body.name == selected_) {
            found = true;
            return body.world;
        }
    }
    return {0, 0, 0};
}

// ---------------------------------------------------------------- time

void StellarApp::update(double seconds) {
    if (seconds <= 0.0) {
        return;
    }

    clock_ += seconds;

    if (!paused_) {
        const double days = seconds * daysPerSecond();
        day_ += days;

        std::vector<BodyFacts> facts = solarSystemFacts();
        for (std::size_t i = 0; i < spin_.size(); i++) {
            double perSecond = 360.0 * daysPerSecond() / facts.at(i + 1).rotationDays;
            perSecond = std::clamp(perSecond, -MAX_VISIBLE_SPIN, MAX_VISIBLE_SPIN);
            spin_[i] = orbit::normalizeAngle(spin_[i] + perSecond * seconds);
        }
    }

    if (touring_) {
        tourTime_ += seconds;
        applyTour();
        return;
    }

    // Click-to-focus: ease the camera target towards the selected body,
    // or back to the Sun after the selection is cleared.
    bool found = false;
    Vec3 goal = selectedWorldPosition(found);
    if (!found && !returning_) {
        return;
    }

    double ease = std::min(1.0, seconds * 3.0);
    camera_.target = add(camera_.target, scale(sub(goal, camera_.target), ease));

    if (!found && std::sqrt(dot(camera_.target, camera_.target)) < 0.5) {
        camera_.target = {0, 0, 0};
        returning_ = false;
    }
}

void StellarApp::startTour() {
    touring_ = true;
    tourTime_ = 0.0;
    selected_.clear();
    camera_.target = {0, 0, 0};
    applyTour();
}

void StellarApp::stopTour() {
    touring_ = false;
}

void StellarApp::applyTour() {
    const int steps = 120;
    static const std::vector<Point> angles = generateCatmullRomSpline(tourAngles(), steps);
    static const std::vector<Point> distances = generateCatmullRomSpline(tourDistances(), steps);

    double progress = std::clamp(tourTime_ / TOUR_SECONDS, 0.0, 1.0);
    std::size_t index = static_cast<std::size_t>(std::lround(progress * (angles.size() - 1)));

    camera_.yaw = orbit::normalizeAngle(angles[index].x);
    camera_.pitch = std::clamp(angles[index].y, -89.0, 89.0);
    camera_.distance = distances[index].x;
    camera_.target = {0, 0, 0};

    if (progress >= 1.0) {
        touring_ = false;
    }
}

// ---------------------------------------------------------------- input

void StellarApp::mouseDown(int x, int y) {
    buttonDown_ = true;
    dragged_ = false;
    downX_ = lastX_ = x;
    downY_ = lastY_ = y;
}

void StellarApp::mouseMove(int x, int y) {
    if (!buttonDown_) {
        return;
    }

    if (std::abs(x - downX_) + std::abs(y - downY_) > 3) {
        dragged_ = true;
    }

    if (dragged_) {
        stopTour();
        camera_.rotate((x - lastX_) * 0.35, (y - lastY_) * 0.35);
    }

    lastX_ = x;
    lastY_ = y;
}

void StellarApp::mouseUp(int x, int y) {
    if (buttonDown_ && !dragged_) {
        select(pick(x, y));
    }
    buttonDown_ = false;
}

void StellarApp::mouseWheel(int notches) {
    stopTour();
    camera_.zoom(std::pow(notches > 0 ? 0.9 : 1.1, std::abs(notches)));
}

void StellarApp::keyDown(char key) {
    if (key >= 'a' && key <= 'z') {
        key = static_cast<char>(key - 'a' + 'A');
    }

    const double step = camera_.distance * 0.04;
    const int lastSpeed = static_cast<int>(speedSteps().size()) - 1;
    std::vector<Planet> planets = solarSystemPlanets();

    switch (key) {
        case 'W': stopTour(); selected_.clear(); returning_ = false; camera_.pan(0, step); break;
        case 'S': stopTour(); selected_.clear(); returning_ = false; camera_.pan(0, -step); break;
        case 'A': stopTour(); selected_.clear(); returning_ = false; camera_.pan(-step, 0); break;
        case 'D': stopTour(); selected_.clear(); returning_ = false; camera_.pan(step, 0); break;
        case '+': case '=': speedIndex_ = std::min(lastSpeed, speedIndex_ + 1); break;
        case '-': case '_': speedIndex_ = std::max(0, speedIndex_ - 1); break;
        case ' ': paused_ = !paused_; break;
        case 'F': if (touring_) { stopTour(); } else { startTour(); } break;
        case 'R': stopTour(); selected_.clear(); returning_ = false; resetView(); break;
        case 27: stopTour(); select(""); break;
        case '0': select("Sun"); break;
        case '9': select("Moon"); break;
        default:
            if (key >= '1' && key <= '8') {
                select(planets.at(static_cast<std::size_t>(key - '1')).name);
            }
            break;
    }
}

// ---------------------------------------------------------------- render

void StellarApp::render(RasterCanvas& canvas) const {
    const int w = canvas.getWidth();
    const int h = canvas.getHeight();
    const CameraBasis basis = basisOf(camera_);
    const std::vector<Planet> planets = solarSystemPlanets();

    // Background and a fixed field of stars.
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            canvas.setPixel(x, y, BACKGROUND);
        }
    }
    for (int i = 0; i < 260; i++) {
        int x = static_cast<int>(hash01(i, 1) * w);
        int y = static_cast<int>(hash01(i, 2) * h);
        auto level = static_cast<std::uint8_t>(70 + hash01(i, 3) * 150);
        canvas.setPixel(x, y, {level, level, static_cast<std::uint8_t>(std::min(255, level + 20))});
    }

    // Orbits (Bresenham lines between projected points).
    for (const Planet& planet : planets) {
        drawLoop(canvas, camera_, {0, 0, 0}, planet.orbitX, planet.orbitY,
                 selected_ == planet.name ? ORBIT_SELECTED : ORBIT);
    }

    const double earthRadius = planets.at(2).radius * PLANET_SIZE_SCALE;
    drawLoop(canvas, camera_, planetWorldPosition(2), earthRadius + MOON_ORBIT_GAP,
             earthRadius + MOON_ORBIT_GAP, {45, 50, 72});

    // Asteroid belt between Mars and Jupiter.
    for (int i = 0; i < 180; i++) {
        double a = 2.0 * PI * i / 180.0;
        double spread = hash01(i, 7) * 14.0;
        Vec3 rock = {(300 + spread) * std::cos(a), (hash01(i, 8) - 0.5) * 6.0,
                     (192 + spread * 0.65) * std::sin(a)};
        ScreenPoint p = camera_.project(rock, w, h);
        if (p.visible) {
            canvas.setPixel(static_cast<int>(p.x), static_cast<int>(p.y), BELT);
        }
    }

    // Bodies, farthest first (painter's algorithm).
    const std::vector<BodyView> list = bodies();
    const BodyView* sunView = nullptr;
    for (const BodyView& body : list) {
        if (body.kind == BodyKind::Sun) {
            sunView = &body;
        }
    }

    for (const BodyView& body : list) {
        if (!body.screen.visible) {
            continue;
        }

        const int cx = static_cast<int>(std::lround(body.screen.x));
        const int cy = static_cast<int>(std::lround(body.screen.y));

        if (body.kind == BodyKind::Sun) {
            drawCircle(canvas, cx, cy, body.screenRadius + 4, {255, 170, 40});
            drawCircle(canvas, cx, cy, body.screenRadius + 8, {150, 90, 20});
        }

        if (body.kind == BodyKind::Comet && sunView != nullptr && sunView->screen.visible) {
            // Tail drawn with DDA, pointing away from the Sun and longer
            // the closer the comet is to it.
            double away = std::hypot(body.screen.x - sunView->screen.x,
                                     body.screen.y - sunView->screen.y);
            double closeness = std::clamp(1.0 - std::sqrt(dot(body.world, body.world)) / 900.0, 0.15, 1.0);
            double length = 60.0 * closeness;
            if (away > 1.0) {
                int tx = static_cast<int>(body.screen.x + (body.screen.x - sunView->screen.x) / away * length);
                int ty = static_cast<int>(body.screen.y + (body.screen.y - sunView->screen.y) / away * length);
                std::vector<Pixel> tail = drawDDA(cx, cy, tx, ty);
                for (std::size_t k = 0; k < tail.size(); k++) {
                    double fade = static_cast<double>(k) / std::max<std::size_t>(1, tail.size());
                    canvas.setPixel(tail[k].x, tail[k].y, mix(COMET_COLOR, BACKGROUND, fade));
                    canvas.setPixel(tail[k].x, tail[k].y + 1, mix(COMET_COLOR, BACKGROUND, 0.4 + fade));
                }
            }
        }

        const bool ringed = body.planetIndex >= 0 &&
                            planets.at(static_cast<std::size_t>(body.planetIndex)).hasRings;
        if (ringed) {
            drawRings(canvas, camera_, body, false);
        }

        double spin = body.planetIndex >= 0 ? spinDegrees(body.planetIndex) : 0.0;
        drawSphere(canvas, body, basis, spin);

        if (ringed) {
            drawRings(canvas, camera_, body, true);
        }
    }

    // Selection marker and information panel.
    for (const BodyView& body : list) {
        if (body.name != selected_ || !body.screen.visible) {
            continue;
        }

        const int cx = static_cast<int>(std::lround(body.screen.x));
        const int cy = static_cast<int>(std::lround(body.screen.y));
        drawCircle(canvas, cx, cy, body.screenRadius + 5, ACCENT);

        // A diamond that spins and pulses: rotate, scale, then move it
        // onto the body with the polygon transformation functions.
        std::vector<Point> marker = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
        scalePolygon(marker, body.screenRadius + 12.0, {0, 0});
        rotatePolygon(marker, std::fmod(clock_ * 90.0, 360.0), {0, 0});
        scalePolygon(marker, 1.0 + 0.08 * std::sin(clock_ * 4.0), {0, 0});
        translatePolygon(marker, cx, cy);
        outlinePolygon(canvas, marker, ACCENT);

        // Information panel beside the body.
        const BodyFacts facts = factsFor(body.name);
        const int textScale = 2;
        const int panelWidth = 520;
        const std::size_t maxChars =
            static_cast<std::size_t>((panelWidth - 28) / ((font::GLYPH_WIDTH + font::GLYPH_SPACING) * textScale));

        std::vector<std::pair<std::string, Color>> lines;
        lines.push_back({facts.type, TEXT_DIM});
        auto addField = [&](const std::string& label, const std::string& value) {
            for (const std::string& line : wrapText(label + ": " + value, maxChars)) {
                lines.push_back({line, TEXT});
            }
        };
        addField("Size", facts.diameter);
        addField("Distance", facts.distance);
        addField("Year", facts.year);
        addField("Day", facts.day);
        addField("Moons", facts.moons);
        for (const std::string& line : wrapText(facts.fact, maxChars)) {
            lines.push_back({line, ACCENT});
        }

        const int titleHeight = font::lineHeight(3) + 6;
        const int panelHeight = 18 + titleHeight + static_cast<int>(lines.size()) * font::lineHeight(textScale) + 8;

        int x0 = cx + body.screenRadius + 28;
        if (x0 + panelWidth > w - 10) {
            x0 = cx - body.screenRadius - 28 - panelWidth;
        }
        int y0 = cy - panelHeight / 2;

        // The panel may stick out past the top or bottom of the window:
        // clip it to the screen with Sutherland-Hodgman before filling.
        std::vector<Point> panel = {{static_cast<double>(x0), static_cast<double>(y0)},
                                    {static_cast<double>(x0 + panelWidth), static_cast<double>(y0)},
                                    {static_cast<double>(x0 + panelWidth), static_cast<double>(y0 + panelHeight)},
                                    {static_cast<double>(x0), static_cast<double>(y0 + panelHeight)}};
        std::vector<Point> visible = sutherlandHodgmanClip(panel, 0, 0, w - 1, h - 1);

        canvas.drawLine(cx, cy, x0 < cx ? x0 + panelWidth : x0, cy, PANEL_BORDER);
        fillPolygonSolid(canvas, visible, PANEL);
        outlinePolygon(canvas, visible, PANEL_BORDER);

        font::drawText(canvas, x0 + 14, y0 + 14, facts.name, ACCENT, 3);
        int ty = y0 + 14 + titleHeight;
        for (const auto& line : lines) {
            font::drawText(canvas, x0 + 14, ty, line.first, line.second, textScale);
            ty += font::lineHeight(textScale);
        }
    }

    // Top-view map in the top-right corner, drawn with midpoint ellipses.
    {
        const int mapW = 240, mapH = 170;
        const int mx = w - mapW - 12, my = 12;
        std::vector<Point> frame = {{static_cast<double>(mx), static_cast<double>(my)},
                                    {static_cast<double>(mx + mapW), static_cast<double>(my)},
                                    {static_cast<double>(mx + mapW), static_cast<double>(my + mapH)},
                                    {static_cast<double>(mx), static_cast<double>(my + mapH)}};
        fillPolygonSolid(canvas, frame, PANEL);
        outlinePolygon(canvas, frame, PANEL_BORDER);
        font::drawText(canvas, mx + 8, my + 6, "TOP VIEW", TEXT_DIM, 1);

        const double s = 0.185;
        const int ox = mx + mapW / 2, oy = my + mapH / 2 + 6;
        for (const Planet& planet : planets) {
            Color c = selected_ == planet.name ? ORBIT_SELECTED : ORBIT;
            for (const auto& p : drawMidpointEllipse(ox, oy, static_cast<int>(planet.orbitX * s),
                                                     static_cast<int>(planet.orbitY * s))) {
                canvas.setPixel(p.first, p.second, c);
            }
        }

        fillPolygonSolid(canvas, circlePolygon(ox, oy, 3), SUN_COLOR);
        for (std::size_t i = 0; i < planets.size(); i++) {
            Vec3 p = planetWorldPosition(static_cast<int>(i));
            int px = ox + static_cast<int>(p.x * s);
            int py = oy + static_cast<int>(p.z * s);
            fillPolygonSolid(canvas, circlePolygon(px, py, 2), planets[i].color);
            if (selected_ == planets[i].name) {
                drawCircle(canvas, px, py, 5, ACCENT);
            }
        }

        // Which way the camera is looking from.
        Vec3 eye = sub(camera_.getPosition(), camera_.target);
        double flat = std::hypot(eye.x, eye.z);
        if (flat > 1e-6) {
            canvas.drawLine(ox, oy, ox + static_cast<int>(eye.x / flat * 40),
                            oy + static_cast<int>(eye.z / flat * 40), TEXT_DIM);
        }
    }

    // Heads-up text.
    font::drawText(canvas, 16, 14, "PROJECT STELLAR", TEXT, 3);
    std::string status = "DAY " + std::to_string(static_cast<long long>(std::floor(day_))) +
                         "   SPEED " + formatNumber(daysPerSecond()) + " DAYS/S";
    if (paused_) {
        status += "   PAUSED";
    }
    font::drawText(canvas, 16, 50, status, paused_ ? ACCENT : TEXT_DIM, 2);

    if (touring_) {
        font::drawText(canvas, 16, 74, "GUIDED TOUR - PRESS F OR DRAG TO STOP", ACCENT, 2);
    } else if (selected_.empty()) {
        font::drawText(canvas, 16, 74, "CLICK A PLANET (OR PRESS 0-9) FOR FACTS", TEXT_DIM, 2);
    }

    font::drawText(canvas, 16, h - 48,
                   "DRAG ROTATE   WHEEL ZOOM   WASD PAN   R RESET   F TOUR", TEXT_DIM, 2);
    font::drawText(canvas, 16, h - 24,
                   "+/- SPEED   SPACE PAUSE   0 SUN  1-8 PLANETS  9 MOON   ESC CLEAR", TEXT_DIM, 2);
}
