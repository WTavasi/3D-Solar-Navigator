# Running Project Stellar

This guide explains how to build and run the full application, the demos and the tests, starting from a fresh computer.

- **Windows:** the full interactive application opens in a window.
- **macOS and Linux:** the application saves still images (snapshot mode), and all the tests run.

## 1. What you need

| Tool | Why | Check it is installed |
|---|---|---|
| A C++17 compiler | Builds the code | `g++ --version` (or `clang++ --version`) |
| CMake 3.20 or newer | Configures the build | `cmake --version` |
| Git | Downloads the project | `git --version` |

### 1.1 Windows: install MSYS2 (compiler and CMake)

1. Download the installer from <https://www.msys2.org> and run it with the default options.
2. When it finishes, open **MSYS2 UCRT64** from the Start menu.
3. Install the compiler, CMake and Make by typing this and pressing Enter (answer `Y` when asked):

   ```bash
   pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-make
   ```

4. Add the tools to Windows' PATH so PowerShell can find them:
   1. Press the Windows key, type **environment variables**, and open **Edit the system environment variables**.
   2. Click **Environment Variables...**.
   3. Under **User variables**, select **Path**, click **Edit**, then **New**, and add `C:\msys64\ucrt64\bin`.
   4. Click **OK** on every window.
5. Close and reopen PowerShell, then check: `g++ --version` and `cmake --version` should both print a version.

### 1.2 macOS

1. Open **Terminal**.
2. Install Apple's compiler: `xcode-select --install` (skip if it says it is already installed).
3. Install CMake. With Homebrew: `brew install cmake`. Without Homebrew, download it from <https://cmake.org/download> and follow the installer.

### 1.3 Linux (Ubuntu or Debian)

```bash
sudo apt install build-essential cmake git
```

## 2. Get the code

```bash
git clone https://github.com/WTavasi/3d-solar-navigator.git
cd 3d-solar-navigator
```

If you already have the project, update it instead with `git pull`.

## 3. Build

Run these from the project folder.

**Windows (PowerShell):**

```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
```

**macOS and Linux:**

```bash
cmake -S . -B build
cmake --build build
```

The first command prepares the `build` folder. The second compiles everything: the application, the algorithm demos and the tests. If you change the code, only the second command needs to be run again.

> If the first command fails with an error about a different generator, delete the `build` folder and run it again.

## 4. Run the application

### 4.1 Windows: the interactive window

```powershell
.\build\stellar.exe
```

A window opens showing the Solar System in motion.

| Control | What it does |
|---|---|
| Drag with the left mouse button | Rotate the view around the target |
| Mouse wheel | Zoom in and out |
| W, A, S, D | Move (pan) the view |
| Click a planet, the Sun or the Moon | Select it: the camera centres on it and a facts panel opens |
| 0 | Select the Sun |
| 1 to 8 | Select Mercury to Neptune |
| 9 | Select the Moon |
| Esc | Clear the selection; the view returns to the Sun |
| + and - | Speed time up or slow it down (0.25 to 365 days per second) |
| Space | Pause or resume time |
| F | Start or stop the guided camera tour |
| R | Reset the view |

The window can be resized or maximised.

### 4.2 Any system: save a still image (snapshot mode)

```bash
./build/stellar --snapshot
```

This saves `stellar_snapshot.bmp` in the current folder. On Windows use `.\build\stellar.exe --snapshot`. On macOS and Linux, running `./build/stellar` without options does the same, because the interactive window is Windows-only.

Options:

| Option | Example | Meaning |
|---|---|---|
| `--day N` | `--day 120` | Show the Solar System N days after the start |
| `--select NAME` | `--select Saturn` | Centre on a body and show its facts panel |
| `--out FILE` | `--out saturn.bmp` | Where to save the image |
| `--size WxH` | `--size 1920x1080` | Image size in pixels |
| `--help` | | List the options |

Example:

```bash
./build/stellar --snapshot --day 120 --select Saturn --out saturn.bmp
```

Open the BMP in any image viewer (Preview on macOS, Photos on Windows).

## 5. Run the other programs

| Program | What it shows | Systems |
|---|---|---|
| `camera_interactive_demo` | Eugene's interactive camera demo | Windows |
| `digital_differential_analyzer`, `bresenham_line_drawing`, `polygon_representation_and_operations`, `polygon_clipping`, `polygon_filling`, `midpoint_circle_drawing`, `midpoint_ellipse_drawing`, `bezier_curve`, `catmull_rom_spline` | One algorithm each, saved as a BMP | All |

Run them the same way, for example `./build/bresenham_line_drawing` (add `.exe` on Windows).

## 6. Run the tests

```bash
ctest --test-dir build --output-on-failure
```

A successful run ends with:

```text
100% tests passed, 0 tests failed out of 32
```

The visual tests save their images into `output/`, including three frames of the application (`stellar_app_overview.bmp`, `stellar_app_selected.bmp`, `stellar_app_tour.bmp`). See `docs/TESTING.md` for running one test at a time and for the full test report.

## 7. Troubleshooting

| Problem | Fix |
|---|---|
| `cmake` or `g++` is "not recognized" (Windows) | The PATH step in section 1.1 was missed, or PowerShell was not reopened afterwards. |
| `No CMAKE_CXX_COMPILER could be found` | The compiler is not installed or not on PATH. See section 1. |
| `Error: generator : MinGW Makefiles does not match the generator used previously` | Delete the `build` folder and run the build commands again. |
| On macOS, `stellar` saves an image instead of opening a window | Expected. The window uses the Windows API; use snapshot mode or run it on Windows. |
| VS Code underlines `#include <cmath>` in red | An editor setting only; the build is unaffected. Install the CMake Tools extension and run **CMake: Configure**. |
