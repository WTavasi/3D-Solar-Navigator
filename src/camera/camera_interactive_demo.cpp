
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>

#include "scene/camera.h"

#include <algorithm>
#include <cmath>

namespace {

constexpr double PI = 3.14159265358979323846;

Camera camera;
bool dragging = false;
bool paused = false;
POINT lastMouse{0, 0};

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

Vec3 planetPosition(const Planet& p) {
    return {
        p.orbit * std::cos(p.angle),
        0.0,
        p.orbit * std::sin(p.angle)
    };
}

void drawPlanet(
    HDC dc, const Vec3& world, double radius,
    COLORREF color, int width, int height
) {
    ScreenPoint p = camera.project(world, width, height);
    if (!p.visible) return;

    double focal = (height * 0.5) /
        std::tan(camera.fov * PI / 360.0);

    int r = std::max(2, static_cast<int>(
        radius * focal / p.depth
    ));

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

void drawScene(HDC dc, int width, int height) {
    RECT background{0, 0, width, height};
    HBRUSH black = CreateSolidBrush(RGB(5, 8, 20));
    FillRect(dc, &background, black);
    DeleteObject(black);

    HPEN orbitPen = CreatePen(
        PS_SOLID, 1, RGB(65, 75, 95)
    );
    HGDIOBJ oldPen = SelectObject(dc, orbitPen);

    // Perspective-project orbital paths.
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

            ScreenPoint p1 = camera.project(
                w1, width, height
            );
            ScreenPoint p2 = camera.project(
                w2, width, height
            );

            if (p1.visible && p2.visible) {
                MoveToEx(dc, int(p1.x), int(p1.y), nullptr);
                LineTo(dc, int(p2.x), int(p2.y));
            }
        }
    }

    SelectObject(dc, oldPen);
    DeleteObject(orbitPen);

    // Draw distant objects first.
    struct Body {
        Vec3 position;
        double radius;
        COLORREF color;
        double depth;
    };

    Body bodies[planetCount + 1];

    bodies[0] = {
        {0, 0, 0}, 28, RGB(255, 190, 45),
        camera.project({0,0,0}, width, height).depth
    };

    for (int i = 0; i < planetCount; ++i) {
        Vec3 pos = planetPosition(planets[i]);

        bodies[i + 1] = {
            pos,
            planets[i].radius,
            planets[i].color,
            camera.project(pos, width, height).depth
        };
    }

    std::sort(
        bodies, bodies + planetCount + 1,
        [](const Body& a, const Body& b) {
            return a.depth > b.depth;
        }
    );

    for (const Body& body : bodies) {
        drawPlanet(
            dc, body.position, body.radius,
            body.color, width, height
        );
    }

    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(230, 235, 245));

    const char* title = "PROJECT STELLAR - 3D CAMERA DEMO";
    TextOutA(dc, 20, 20, title, lstrlenA(title));

    const char* controls =
        "Drag: rotate | Wheel: zoom | WASD: pan | "
        "R: reset | Space: pause";
    TextOutA(
        dc, 20, 45, controls, lstrlenA(controls)
    );
}

LRESULT CALLBACK windowProc(
    HWND hwnd, UINT message, WPARAM wp, LPARAM lp
) {
    switch (message) {

    case WM_CREATE:
        SetTimer(hwnd, 1, 33, nullptr);
        return 0;

    case WM_TIMER:
        if (!paused) {
            for (Planet& p : planets) {
                p.angle += p.speed * 0.033;
                if (p.angle >= 2 * PI)
                    p.angle -= 2 * PI;
            }
        }
        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;

    case WM_LBUTTONDOWN:
        dragging = true;
        lastMouse.x = GET_X_LPARAM(lp);
        lastMouse.y = GET_Y_LPARAM(lp);
        SetCapture(hwnd);
        return 0;

    case WM_LBUTTONUP:
        dragging = false;
        ReleaseCapture();
        return 0;

    case WM_MOUSEMOVE:
        if (dragging) {
            int x = GET_X_LPARAM(lp);
            int y = GET_Y_LPARAM(lp);

            camera.rotate(
                (x - lastMouse.x) * 0.35,
                -(y - lastMouse.y) * 0.35
            );

            lastMouse = {x, y};
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        return 0;

    case WM_MOUSEWHEEL:
        camera.zoom(
            GET_WHEEL_DELTA_WPARAM(wp) > 0
                ? 0.90 : 1.10
        );
        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;

    case WM_KEYDOWN:
        switch (wp) {
        case 'W': camera.pan(0, 25); break;
        case 'S': camera.pan(0, -25); break;
        case 'A': camera.pan(-25, 0); break;
        case 'D': camera.pan(25, 0); break;
        case 'R': camera.reset(); break;
        case VK_SPACE: paused = !paused; break;
        }
        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC screen = BeginPaint(hwnd, &ps);

        RECT rect;
        GetClientRect(hwnd, &rect);
        int w = rect.right;
        int h = rect.bottom;

        // Double buffering reduces visual flickering.
        if (w > 0 && h > 0) {
            HDC memory = CreateCompatibleDC(screen);
            HBITMAP bitmap =
                CreateCompatibleBitmap(screen, w, h);
            HGDIOBJ old = SelectObject(memory, bitmap);

            drawScene(memory, w, h);
            BitBlt(screen, 0, 0, w, h,
                   memory, 0, 0, SRCCOPY);

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
    HINSTANCE instance, HINSTANCE,
    LPSTR, int showCommand
) {
    WNDCLASSA wc{};
    wc.lpfnWndProc = windowProc;
    wc.hInstance = instance;
    wc.lpszClassName = "StellarCameraDemo";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    if (!RegisterClassA(&wc))
        return 1;

    HWND hwnd = CreateWindowExA(
        0, wc.lpszClassName,
        "Project Stellar - Interactive 3D Camera",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1200, 800,
        nullptr, nullptr, instance, nullptr
    );

    if (!hwnd)
        return 1;

    ShowWindow(hwnd, showCommand);
    UpdateWindow(hwnd);

    MSG msg{};
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}
