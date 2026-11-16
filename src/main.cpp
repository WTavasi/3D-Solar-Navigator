// Project Stellar: the application entry point.
//
// On Windows, `stellar` opens an interactive window. The window code only
// copies StellarApp's canvas to the screen and forwards mouse and keyboard
// input; every pixel is drawn by the project's own algorithms.
//
// On any system, `stellar --snapshot` saves a still image of the app
// instead (see docs/RUNNING.md for all options).

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "app/stellar_app.h"
#include "raster/raster_canvas.h"

namespace {

struct Options {
    bool snapshot = false;
    bool help = false;
    double day = 0.0;
    std::string select;
    std::string output = "stellar_snapshot.bmp";
    int width = 1280;
    int height = 800;
    std::string error;
};

void printUsage() {
    std::cout <<
        "Project Stellar\n"
        "\n"
        "  stellar                 open the interactive window (Windows)\n"
        "  stellar --snapshot      save a still image instead (any system)\n"
        "\n"
        "Snapshot options:\n"
        "  --day N                 simulation day to show (default 0)\n"
        "  --select NAME           open the facts panel for Sun, a planet or Moon\n"
        "  --out FILE.bmp          where to save (default stellar_snapshot.bmp)\n"
        "  --size WIDTHxHEIGHT     image size (default 1280x800)\n";
}

Options parseOptions(int argc, char** argv) {
    Options options;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        bool hasValue = i + 1 < argc;

        if (arg == "--snapshot") {
            options.snapshot = true;
        } else if (arg == "--help" || arg == "-h") {
            options.help = true;
        } else if (arg == "--day" && hasValue) {
            options.day = std::atof(argv[++i]);
        } else if (arg == "--select" && hasValue) {
            options.select = argv[++i];
        } else if (arg == "--out" && hasValue) {
            options.output = argv[++i];
        } else if (arg == "--size" && hasValue) {
            std::string size = argv[++i];
            std::size_t x = size.find('x');
            if (x == std::string::npos) {
                options.error = "--size needs WIDTHxHEIGHT, for example 1280x800";
            } else {
                options.width = std::atoi(size.substr(0, x).c_str());
                options.height = std::atoi(size.substr(x + 1).c_str());
            }
        } else {
            options.error = "Unknown option: " + arg;
        }
    }

    if (options.width < 200 || options.height < 200) {
        options.error = "The image must be at least 200 x 200 pixels.";
    }

    return options;
}

int saveSnapshot(const Options& options) {
    RasterCanvas canvas(options.width, options.height);
    StellarApp app(options.width, options.height);

    app.setDay(options.day);
    app.setPaused(true);

    if (!options.select.empty() && !app.select(options.select)) {
        std::cerr << "Unknown body: " << options.select
                  << " (use Sun, Mercury ... Neptune, or Moon)\n";
        return 1;
    }

    // Let the camera settle on the selected body, as it would on screen.
    for (int frame = 0; frame < 90; frame++) {
        app.update(1.0 / 30.0);
    }

    app.render(canvas);

    if (!canvas.saveBMP(options.output)) {
        std::cerr << "Could not save " << options.output << "\n";
        return 1;
    }

    std::cout << "Saved " << options.output << "\n";
    return 0;
}

}  // namespace

#ifdef _WIN32

#include <windows.h>
#include <windowsx.h>

namespace window {

StellarApp* app = nullptr;
RasterCanvas* canvas = nullptr;
std::vector<std::uint32_t> pixels;
BITMAPINFO bitmapInfo = {};
std::chrono::steady_clock::time_point lastTick;

void resizeBuffers(int width, int height) {
    delete canvas;
    canvas = new RasterCanvas(width, height);
    pixels.assign(static_cast<std::size_t>(width) * height, 0);
    app->resize(width, height);

    bitmapInfo = {};
    bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo.bmiHeader.biWidth = width;
    bitmapInfo.bmiHeader.biHeight = -height;   // negative: rows top to bottom
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;
}

// Copies the canvas to the window. Windows expects blue, green, red order.
void present(HDC dc) {
    const int width = canvas->getWidth();
    const int height = canvas->getHeight();

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Color c = canvas->getPixel(x, y);
            pixels[static_cast<std::size_t>(y) * width + x] =
                (static_cast<std::uint32_t>(c.red) << 16) |
                (static_cast<std::uint32_t>(c.green) << 8) | c.blue;
        }
    }

    StretchDIBits(dc, 0, 0, width, height, 0, 0, width, height, pixels.data(),
                  &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
}

// Turns a Windows key code into the character StellarApp::keyDown expects.
char keyFor(WPARAM key) {
    if ((key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9')) {
        return static_cast<char>(key);
    }

    switch (key) {
        case VK_SPACE: return ' ';
        case VK_ESCAPE: return 27;
        case VK_OEM_PLUS: case VK_ADD: return '+';
        case VK_OEM_MINUS: case VK_SUBTRACT: return '-';
        case VK_NUMPAD0: case VK_NUMPAD1: case VK_NUMPAD2: case VK_NUMPAD3:
        case VK_NUMPAD4: case VK_NUMPAD5: case VK_NUMPAD6: case VK_NUMPAD7:
        case VK_NUMPAD8: case VK_NUMPAD9:
            return static_cast<char>('0' + (key - VK_NUMPAD0));
        default: return 0;
    }
}

LRESULT CALLBACK windowProc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp) {
    switch (message) {
        case WM_CREATE:
            lastTick = std::chrono::steady_clock::now();
            SetTimer(hwnd, 1, 16, nullptr);   // about 60 frames a second
            return 0;

        case WM_TIMER: {
            auto now = std::chrono::steady_clock::now();
            double seconds = std::chrono::duration<double>(now - lastTick).count();
            lastTick = now;

            app->update(std::min(seconds, 0.1));
            app->render(*canvas);
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        case WM_PAINT: {
            PAINTSTRUCT paint;
            HDC dc = BeginPaint(hwnd, &paint);
            present(dc);
            EndPaint(hwnd, &paint);
            return 0;
        }

        case WM_ERASEBKGND:
            return 1;   // the whole window is redrawn every frame

        case WM_SIZE: {
            int width = LOWORD(lp);
            int height = HIWORD(lp);
            if (width > 0 && height > 0) {
                resizeBuffers(width, height);
            }
            return 0;
        }

        case WM_LBUTTONDOWN:
            SetCapture(hwnd);
            app->mouseDown(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
            return 0;

        case WM_MOUSEMOVE:
            app->mouseMove(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
            return 0;

        case WM_LBUTTONUP:
            ReleaseCapture();
            app->mouseUp(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
            return 0;

        case WM_MOUSEWHEEL:
            app->mouseWheel(GET_WHEEL_DELTA_WPARAM(wp) / WHEEL_DELTA);
            return 0;

        case WM_KEYDOWN: {
            char key = keyFor(wp);
            if (key != 0) {
                app->keyDown(key);
            }
            return 0;
        }

        case WM_DESTROY:
            KillTimer(hwnd, 1);
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(hwnd, message, wp, lp);
    }
}

int run(const Options& options) {
    HINSTANCE instance = GetModuleHandle(nullptr);
    const char* className = "ProjectStellarWindow";

    WNDCLASSA windowClass = {};
    windowClass.lpfnWndProc = windowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = className;
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassA(&windowClass);

    StellarApp application(options.width, options.height);
    app = &application;
    canvas = new RasterCanvas(options.width, options.height);

    RECT frame = {0, 0, options.width, options.height};
    AdjustWindowRect(&frame, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindowA(className, "Project Stellar - Solar System",
                              WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                              frame.right - frame.left, frame.bottom - frame.top,
                              nullptr, nullptr, instance, nullptr);
    if (hwnd == nullptr) {
        std::cerr << "Could not create the window.\n";
        return 1;
    }

    ShowWindow(hwnd, SW_SHOW);

    MSG message;
    while (GetMessage(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    delete canvas;
    canvas = nullptr;
    app = nullptr;
    return 0;
}

}  // namespace window

#endif

int main(int argc, char** argv) {
    Options options = parseOptions(argc, argv);

    if (!options.error.empty()) {
        std::cerr << options.error << "\n\n";
        printUsage();
        return 1;
    }

    if (options.help) {
        printUsage();
        return 0;
    }

#ifdef _WIN32
    if (!options.snapshot) {
        return window::run(options);
    }
#else
    if (!options.snapshot) {
        std::cout << "The interactive window runs on Windows. "
                     "Saving a still image instead.\n";
    }
#endif

    return saveSnapshot(options);
}
