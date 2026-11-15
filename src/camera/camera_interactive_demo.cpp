
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>

#include "scene/camera.h"

#include <algorithm>
#include <cmath>
#include <chrono>

namespace {

constexpr double PI = 3.14159265358979323846;

// Camera navigation settings.
constexpr double MIN_PITCH = 10.0;
constexpr double MAX_PITCH = 85.0;

constexpr double MIN_DISTANCE = 350.0;
constexpr double MAX_DISTANCE = 2500.0;

constexpr double ROTATION_SENSITIVITY = 0.18;
constexpr double ZOOM_SENSITIVITY = 0.04;
constexpr double CAMERA_DAMPING = 12.0;
constexpr double MAX_PAN_DISTANCE = 350.0;

// Current and desired camera states.
Camera camera;
Camera desiredCamera;

bool dragging = false;
bool paused = false;

POINT lastMouse{0, 0};

using Clock = std::chrono::steady_clock;
Clock::time_point lastUpdate = Clock::now();

struct Planet {
    double orbit;
    double radius;
    double angle;
    double speed;
    COLORREF color;
};

Planet planets[] = {
    { 65,  4,  0.2, 1.80, RGB(170,170,170) },
    { 105, 7,  1.1, 1.30, RGB(230,180,100) },
    { 150, 8,  2.4, 1.00, RGB( 50,130,240) },
    { 195, 6,  3.2, 0.80, RGB(220, 75, 45) },
    { 265, 18, 4.5, 0.45, RGB(210,165,120) },
    { 340, 15, 5.2, 0.32, RGB(215,200,140) },
    { 410, 12, 2.0, 0.22, RGB(110,210,230) },
    { 475, 12, 0.8, 0.17, RGB( 65, 90,220) }
};

constexpr int planetCount =
    sizeof(planets) / sizeof(planets[0]);

// Smooth interpolation between camera states.
void updateCamera(double deltaTime) {
    double alpha =
        1.0 - std::exp(-CAMERA_DAMPING * deltaTime);

    camera.yaw +=
        (desiredCamera.yaw - camera.yaw) * alpha;

    camera.pitch +=
        (desiredCamera.pitch - camera.pitch) * alpha;

    camera.distance +=
        (desiredCamera.distance - camera.distance) * alpha;

    camera.target.x +=
        (desiredCamera.target.x - camera.target.x) * alpha;

    camera.target.y +=
        (desiredCamera.target.y - camera.target.y) * alpha;

    camera.target.z +=
        (desiredCamera.target.z - camera.target.z) * alpha;

    camera.fov = desiredCamera.fov;
}

// Continuous, frame-rate-independent keyboard navigation.
void updateNavigation(HWND hwnd, double deltaTime) {
    if (GetFocus() != hwnd) {
        return;
    }

    double horizontal = 0.0;
    double vertical = 0.0;

    if (GetAsyncKeyState('A') & 0x8000)
        horizontal -= 1.0;

    if (GetAsyncKeyState('D') & 0x8000)
        horizontal += 1.0;

    if (GetAsyncKeyState('W') & 0x8000)
        vertical += 1.0;

    if (GetAsyncKeyState('S') & 0x8000)
        vertical -= 1.0;

    // Normalize diagonal movement.
    double length = std::sqrt(
        horizontal * horizontal +
        vertical * vertical
    );

    if (length > 1.0) {
        horizontal /= length;
        vertical /= length;
    }

    // Adjust movement speed to camera distance.
    double speed = desiredCamera.distance * 0.30;

    // Optional precision movement with Ctrl.
    if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
        speed *= 0.35;

    double movement = speed * deltaTime;
    double yawRadians = camera.yaw * PI / 180.0;

    // Translate along the XZ orbital plane.
    desiredCamera.target.x +=
        (horizontal * std::cos(yawRadians) -
         vertical * std::sin(yawRadians)) * movement;

    desiredCamera.target.z +=
        (-horizontal * std::sin(yawRadians) -
          vertical * std::cos(yawRadians)) * movement;

    // Keep the target on the orbital plane.
    desiredCamera.target.y = 0.0;

    // Prevent panning too far from the Solar System.
    double targetDistance = std::sqrt(
        desiredCamera.target.x * desiredCamera.target.x +
        desiredCamera.target.z * desiredCamera.target.z
    );

    if (targetDistance > MAX_PAN_DISTANCE) {
        double scale = MAX_PAN_DISTANCE / targetDistance;

        desiredCamera.target.x *= scale;
        desiredCamera.target.z *= scale;
    }
}

// Convert orbital parameters into 3D world coordinates.
Vec3 planetPosition(const Planet& p) {
    return {
        p.orbit * std::cos(p.angle),
        0.0,
        p.orbit * std::sin(p.angle)
    };
}

// Draw a planet using its perspective-projected position.
void drawPlanet(
    HDC dc,
    const Vec3& world,
    double radius,
    COLORREF color,
    int width,
    int height
) {
    ScreenPoint p = camera.project(world, width, height);

    if (!p.visible) {
        return;
    }

    double focal = (height * 0.5) /
        std::tan(camera.fov * PI / 360.0);

    // Perspective scaling based on camera-space depth.
    int r = std::max(
        2,
        static_cast<int>(radius * focal / p.depth)
    );

    HBRUSH brush = CreateSolidBrush(color);
    HGDIOBJ oldBrush = SelectObject(dc, brush);
    HGDIOBJ oldPen =
        SelectObject(dc, GetStockObject(NULL_PEN));

    int x = static_cast<int>(p.x);
    int y = static_cast<int>(p.y);

    Ellipse(dc, x-r, y-r, x+r, y+r);

    SelectObject(dc, oldBrush);
    SelectObject(dc, oldPen);
    DeleteObject(brush);
}

// Render the Solar System.
void drawScene(HDC dc, int width, int height) {
    RECT background{0, 0, width, height};

    HBRUSH black = CreateSolidBrush(RGB(5, 8, 20));
    FillRect(dc, &background, black);
    DeleteObject(black);

    // Draw orbital paths.
    HPEN orbitPen = CreatePen(
        PS_SOLID, 1, RGB(65, 75, 95)
    );

    HGDIOBJ oldPen = SelectObject(dc, orbitPen);

    for (const Planet& planet : planets) {
        for (int i = 0; i < 360; ++i) {
            double a = i * PI / 180.0;
            double b = (i + 1) * PI / 180.0;

            Vec3 w1{
                planet.orbit * std::cos(a),
                0.0,
                planet.orbit * std::sin(a)
            };

            Vec3 w2{
                planet.orbit * std::cos(b),
                0.0,
                planet.orbit * std::sin(b)
            };

            ScreenPoint p1 =
                camera.project(w1, width, height);

            ScreenPoint p2 =
                camera.project(w2, width, height);

            if (p1.visible && p2.visible) {
                MoveToEx(
                    dc,
                    static_cast<int>(p1.x),
                    static_cast<int>(p1.y),
                    nullptr
                );

                LineTo(
                    dc,
                    static_cast<int>(p2.x),
                    static_cast<int>(p2.y)
                );
            }
        }
    }

    SelectObject(dc, oldPen);
    DeleteObject(orbitPen);

    // Depth-based drawing order.
    struct Body {
        Vec3 position;
        double radius;
        COLORREF color;
        double depth;
    };

    Body bodies[planetCount + 1];

    // Sun.
    bodies[0] = {
        {0.0, 0.0, 0.0},
        28.0,
        RGB(255, 190, 45),
        camera.project(
            {0.0, 0.0, 0.0}, width, height
        ).depth
    };

    // Planets.
    for (int i = 0; i < planetCount; ++i) {
        Vec3 pos = planetPosition(planets[i]);

        bodies[i + 1] = {
            pos,
            planets[i].radius,
            planets[i].color,
            camera.project(pos, width, height).depth
        };
    }

    // Painter's Algorithm:
    // Render far objects first and near objects last.
    std::sort(
        bodies,
        bodies + planetCount + 1,
        [](const Body& a, const Body& b) {
            return a.depth > b.depth;
        }
    );

    for (const Body& body : bodies) {
        drawPlanet(
            dc,
            body.position,
            body.radius,
            body.color,
            width,
            height
        );
    }

    // On-screen information.
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(230, 235, 245));

    const char* title =
        "PROJECT STELLAR - 3D CAMERA NAVIGATION";

    TextOutA(dc, 20, 20, title, lstrlenA(title));

    const char* controls =
        "Drag: Rotate | Wheel: Zoom | WASD: Pan | "
        "R: Reset | Space: Pause";

    TextOutA(
        dc, 20, 45, controls, lstrlenA(controls)
    );

    const char* hint =
        "Hold Ctrl for precise navigation";

    TextOutA(
        dc, 20, 65, hint, lstrlenA(hint)
    );

    if (paused) {
        const char* status = "ANIMATION PAUSED";

        SetTextColor(dc, RGB(255, 190, 45));

        TextOutA(
            dc, 20, 90, status, lstrlenA(status)
        );
    }
}

// Handle Windows events.
LRESULT CALLBACK windowProc(
    HWND hwnd,
    UINT message,
    WPARAM wp,
    LPARAM lp
) {
    switch (message) {

    case WM_CREATE:
        camera.reset();
        desiredCamera = camera;

        lastUpdate = Clock::now();

        // Request approximately 60 updates per second.
        SetTimer(hwnd, 1, 16, nullptr);
        return 0;

    case WM_TIMER: {
        auto now = Clock::now();

        double deltaTime =
            std::chrono::duration<double>(
                now - lastUpdate
            ).count();

        lastUpdate = now;

        // Avoid large jumps after temporary stalls.
        deltaTime = std::clamp(
            deltaTime, 0.0, 0.1
        );

        updateNavigation(hwnd, deltaTime);
        updateCamera(deltaTime);

        if (!paused) {
            for (Planet& p : planets) {
                p.angle = std::fmod(
                    p.angle + p.speed * deltaTime,
                    2.0 * PI
                );
            }
        }

        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;
    }

    case WM_LBUTTONDOWN:
        SetFocus(hwnd);

        dragging = true;

        lastMouse.x = GET_X_LPARAM(lp);
        lastMouse.y = GET_Y_LPARAM(lp);

        SetCapture(hwnd);
        return 0;

    case WM_LBUTTONUP:
        dragging = false;
        ReleaseCapture();
        return 0;

    case WM_CAPTURECHANGED:
        dragging = false;
        return 0;

    case WM_MOUSEMOVE:
        if (dragging) {
            int x = GET_X_LPARAM(lp);
            int y = GET_Y_LPARAM(lp);

            double deltaX =
                static_cast<double>(x - lastMouse.x);

            double deltaY =
                static_cast<double>(y - lastMouse.y);

            // Horizontal orbit rotation.
            desiredCamera.yaw +=
                deltaX * ROTATION_SENSITIVITY;

            // Restrict camera to above the orbital plane.
            desiredCamera.pitch = std::clamp(
                desiredCamera.pitch -
                    deltaY * ROTATION_SENSITIVITY,
                MIN_PITCH,
                MAX_PITCH
            );

            lastMouse = {x, y};
        }

        return 0;

    case WM_MOUSEWHEEL: {
        double wheelSteps =
            static_cast<double>(
                GET_WHEEL_DELTA_WPARAM(wp)
            ) / WHEEL_DELTA;

        // Smooth multiplicative zoom.
        double zoomFactor = std::exp(
            -wheelSteps * ZOOM_SENSITIVITY
        );

        desiredCamera.distance = std::clamp(
            desiredCamera.distance * zoomFactor,
            MIN_DISTANCE,
            MAX_DISTANCE
        );

        return 0;
    }

    case WM_KEYDOWN:
        switch (wp) {

        case 'R':
            desiredCamera.reset();
            camera = desiredCamera;
            break;

        case VK_SPACE:
            // Prevent repeated pause toggling.
            if ((lp & (1L << 30)) == 0) {
                paused = !paused;
            }
            break;
        }

        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;

    case WM_ERASEBKGND:
        // Background is drawn in the back buffer.
        return 1;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC screen = BeginPaint(hwnd, &ps);

        RECT rect;
        GetClientRect(hwnd, &rect);

        int w = rect.right;
        int h = rect.bottom;

        if (w > 0 && h > 0) {
            // Prepare a complete frame in memory.
            HDC memory = CreateCompatibleDC(screen);

            HBITMAP bitmap =
                CreateCompatibleBitmap(screen, w, h);

            HGDIOBJ old =
                SelectObject(memory, bitmap);

            drawScene(memory, w, h);

            // Bit-block transfer to the visible window.
            BitBlt(
                screen,
                0, 0, w, h,
                memory,
                0, 0,
                SRCCOPY
            );

            SelectObject(memory, old);
            DeleteObject(bitmap);
            DeleteDC(memory);
        }

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        KillTimer(hwnd, 1);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, message, wp, lp);
}

} // namespace

int WINAPI WinMain(
    HINSTANCE instance,
    HINSTANCE,
    LPSTR,
    int showCommand
) {
    WNDCLASSA wc{};

    wc.lpfnWndProc = windowProc;
    wc.hInstance = instance;
    wc.lpszClassName = "StellarCameraDemo";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    if (!RegisterClassA(&wc)) {
        return 1;
    }

    HWND hwnd = CreateWindowExA(
        0,
        wc.lpszClassName,
        "Project Stellar - Interactive 3D Camera",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1200,
        800,
        nullptr,
        nullptr,
        instance,
        nullptr
    );

    if (!hwnd) {
        return 1;
    }

    ShowWindow(hwnd, showCommand);
    UpdateWindow(hwnd);

    MSG msg{};

    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}
