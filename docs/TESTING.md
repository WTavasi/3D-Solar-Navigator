# Testing Guide and Test Report

This document explains how Project Stellar is tested, how to run the tests, and what the tests found.

## 1. Testing Approach

The project is tested at three levels. Each level answers a different question.

| Level | Folder | Question it answers | How it passes or fails |
|---|---|---|---|
| Unit | `tests/unit/` | Does each algorithm give the right pixels or points? | Assertions (`CHECK`) inside the test. Exit code 1 on any failure. |
| Integration | `tests/integration/` | Do the algorithms work together in the Solar System scene? | Assertions on the scene data and the rendered image. |
| Visual | `tests/visual/` | What does each algorithm look like? | Saves a BMP into `output/` as evidence. Fails if nothing is drawn or the file cannot be saved. |

All three levels are registered with CTest, so they run with one command and on every pull request through GitHub Actions.

### 1.1 The tests use the project's own code

Every algorithm lives in a header under `include/algorithms/`. The demo programs in `src/algorithms/`, the tests and the scene all include the same header. A test therefore checks the code the application actually uses, not a copy of it.

### 1.2 Kinds of checks

The unit tests combine four kinds of checks:

- **Hand-worked examples.** Small inputs whose answers were worked out on paper or taken from the course textbook (Hearn and Baker), for example the DDA line from (2, 3) to (10, 7) or the midpoint circle of radius 10.
- **Properties.** Rules that must hold for every input, checked over thousands of generated inputs. For example: every line pixel is within half a pixel of the true line, every filled pixel is inside the polygon, and a Bézier curve stays inside the convex hull of its control points.
- **Edge cases.** Zero-length lines, radius 0, very flat ellipses, polygons fully inside or outside the clipping window, fewer points than an algorithm needs.
- **Cross-checks.** Two independent methods must agree. For example: Bresenham against DDA, Bernstein-form Bézier against de Casteljau's algorithm, and a midpoint ellipse with equal radii against the midpoint circle.

### 1.3 The test framework

`tests/support/test_framework.h` is a small header written for this project, so no external library is needed.

```cpp
#include "test_framework.h"

TEST_CASE("DDA-02 a zero-length line is a single pixel") {
    CHECK_EQ(drawDDA(4, 9, 4, 9), (std::vector<Pixel>{{4, 9}}));
}

int main() {
    return testing::runAllTests();
}
```

- `CHECK(condition)` fails if the condition is false.
- `CHECK_EQ(actual, expected)` fails if the values differ and prints both.
- `CHECK_NEAR(actual, expected, tolerance)` is for decimal values.

## 2. Running the Tests

### 2.1 Build and run everything

Run these from the project root.

**Windows (PowerShell):**

```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
ctest --test-dir build --output-on-failure
```

**macOS and Linux:**

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

A successful run ends with:

```text
100% tests passed, 0 tests failed out of 22
```

### 2.2 Run one level or one test

```bash
ctest --test-dir build -L unit          # only the unit tests
ctest --test-dir build -L integration   # only the integration tests
ctest --test-dir build -L visual        # only the visual tests (regenerates output/)
ctest --test-dir build -R ellipse       # tests whose name contains "ellipse"
```

To see every individual test case, run a test program directly. On Windows add `.exe`.

```bash
./build/tests/test_midpoint_ellipse
```

```text
[PASS] ELL-01 matches the textbook first quadrant for rx = 8, ry = 6
[PASS] ELL-02 the ellipse has four-way symmetry
...
8 of 8 test cases passed.
```

### 2.3 Visual output

The visual tests save their images into `output/` in the project root, wherever they are run from. The images are deterministic, so running the tests again does not change the committed files unless the drawing code changed.

### 2.4 Adding a test

1. Create `tests/unit/test_<name>.cpp` (or `tests/integration/`).
2. Include the algorithm header and `test_framework.h`, write `TEST_CASE`s, and end with `main()` as above.
3. Register it in `tests/CMakeLists.txt` with `stellar_add_unit_test(test_<name>)`.
4. Give each case an ID (`ABC-01`) and a sentence describing the expected behaviour, and add it to the report below.

## 3. Test Report

**Result:** 96 of 96 test cases passed in 11 test programs, and all 11 visual tests produced their images. Built with GCC 13 and Clang 18 (C++17, `-Wall -Wextra -Wpedantic`, no warnings) on 28 October 2026.

**Kind:** H = hand-worked example, P = property over many inputs, E = edge case, X = cross-check, R = regression test for a defect found during testing.

### 3.1 Raster canvas (`test_raster_canvas`)

| ID | Expected behaviour | Kind | Result |
|---|---|---|---|
| RC-01 | A new canvas is white by default | H | Pass |
| RC-02 | The background colour fills every pixel | H | Pass |
| RC-03 | `setPixel` stores the colour that `getPixel` reads back | H | Pass |
| RC-04 | Pixels outside the canvas are ignored safely | E | Pass |
| RC-05 | `drawLine` colours both endpoints and nothing else | H | Pass |
| RC-06 | `drawLine` keeps only the visible part of an off-canvas line | E | Pass |
| RC-07 | `saveBMP` writes a valid 24-bit BMP header | H | Pass |
| RC-08 | `saveBMP` pads every row to a multiple of 4 bytes | E | Pass |
| RC-09 | `saveBMP` stores rows bottom-up in BGR order | H | Pass |
| RC-10 | `saveBMP` reports failure when the file cannot be created | E | Pass |

### 3.2 DDA line drawing (`test_dda`)

| ID | Expected behaviour | Kind | Result |
|---|---|---|---|
| DDA-01 | Matches the hand-worked example (2, 3) to (10, 7) | H | Pass |
| DDA-02 | A zero-length line is a single pixel | E | Pass |
| DDA-03 | Horizontal, vertical and 45 degree lines are exact | H | Pass |
| DDA-04 | Lines in all eight octants start and end on the endpoints | P | Pass |
| DDA-05 | One pixel per step along the major axis, with no gaps | P | Pass |
| DDA-06 | Every pixel is within half a pixel of the true line | P | Pass |
| DDA-07 | Works with negative and off-screen coordinates | E | Pass |

### 3.3 Bresenham line drawing (`test_bresenham`)

| ID | Expected behaviour | Kind | Result |
|---|---|---|---|
| BRE-01 | Matches the textbook example (20, 10) to (30, 18) | H | Pass |
| BRE-02 | A zero-length line is a single pixel | E | Pass |
| BRE-03 | Horizontal, vertical and 45 degree lines are exact | H | Pass |
| BRE-04 | Lines in all eight octants start and end on the endpoints | P | Pass |
| BRE-05 | One pixel per step along the major axis, with no gaps | P | Pass |
| BRE-06 | Every pixel is within half a pixel of the true line | P | Pass |
| BRE-07 | Agrees with DDA to within one pixel at every step | X | Pass |
| BRE-08 | `RasterCanvas::drawLine` draws exactly the Bresenham pixels | X | Pass |

### 3.4 Polygon operations (`test_polygon_operations`)

| ID | Expected behaviour | Kind | Result |
|---|---|---|---|
| POL-01 | Reproduces the hand-worked translate, scale, rotate example | H | Pass |
| POL-02 | Translating and translating back restores the polygon | P | Pass |
| POL-03 | Scaling keeps the fixed point and scales distances to it | P | Pass |
| POL-04 | A 90 degree rotation turns the +x axis onto the +y axis | H | Pass |
| POL-05 | Rotation preserves side lengths and undoes cleanly | P | Pass |
| POL-06 | Area is kept by translation and rotation, times s² by scaling | P | Pass |
| POL-07 | Transformations keep the number and order of vertices | P | Pass |
| POL-08 | An empty polygon is left empty | E | Pass |

### 3.5 Sutherland-Hodgman clipping (`test_polygon_clipping`)

| ID | Expected behaviour | Kind | Result |
|---|---|---|---|
| CLP-01 | The demo pentagon clips to exactly the window rectangle | H | Pass |
| CLP-02 | A polygon fully inside the window is returned unchanged | E | Pass |
| CLP-03 | A polygon fully outside the window is removed | E | Pass |
| CLP-04 | A polygon covering the whole window clips to the window | E | Pass |
| CLP-05 | Cutting off one corner gives the hand-worked pentagon (area 34) | H | Pass |
| CLP-06 | Clipped vertices stay inside the window and area never grows | P | Pass |
| CLP-07 | Vertices exactly on the window edge count as inside | E | Pass |
| CLP-08 | Clipping keeps the winding direction of the input | P | Pass |
| CLP-09 | Known limitation: a concave polygon split in two stays one outline | E | Pass |

### 3.6 Scan-line filling (`test_polygon_filling`)

| ID | Expected behaviour | Kind | Result |
|---|---|---|---|
| FIL-01 | The demo pentagon produces the hand-worked spans | H | Pass |
| FIL-02 | A rectangle fills its bottom row but not its top row | H | Pass |
| FIL-03 | A concave notch splits a scan line into two spans | H | Pass |
| FIL-04 | Every filled pixel lies inside or on the polygon | P | Pass |
| FIL-05 | Spans on a scan line are ordered and never overlap | P | Pass |
| FIL-06 | The number of filled pixels is close to the polygon area | P | Pass |
| FIL-07 | Moving the polygon by whole pixels moves the spans with it | P | Pass |
| FIL-08 | Fewer than three vertices fills nothing | E | Pass |
| FIL-09 | Vertex order (clockwise or not) gives the same fill | P | Pass |
| FIL-10 | Spans that meet at a concave vertex are joined | R | Pass |

### 3.7 Midpoint circle (`test_midpoint_circle`)

| ID | Expected behaviour | Kind | Result |
|---|---|---|---|
| CIR-01 | Matches the textbook first octant for radius 10 | H | Pass |
| CIR-02 | The circle has eight-way symmetry | P | Pass |
| CIR-03 | Every pixel is within half a pixel of the true radius (radii 1 to 300) | P | Pass |
| CIR-04 | The outline is closed with no gaps | P | Pass |
| CIR-05 | The four axis points are always drawn | P | Pass |
| CIR-06 | Radius 0 is the centre point and a negative radius is empty | E | Pass |
| CIR-07 | Moving the centre moves every pixel by the same amount | P | Pass |

### 3.8 Midpoint ellipse (`test_midpoint_ellipse`)

| ID | Expected behaviour | Kind | Result |
|---|---|---|---|
| ELL-01 | Matches the textbook first quadrant for rx = 8, ry = 6 | H | Pass |
| ELL-02 | The ellipse has four-way symmetry | P | Pass |
| ELL-03 | The four axis points are always drawn, even when very flat | R | Pass |
| ELL-04 | Every pixel is within one pixel of the true ellipse | P | Pass |
| ELL-05 | The outline is one connected piece with no gaps | P | Pass |
| ELL-06 | Equal radii draw exactly the midpoint circle | X | Pass |
| ELL-07 | Zero radii draw a point or a straight segment | R | Pass |
| ELL-08 | Moving the centre moves every pixel by the same amount | P | Pass |

### 3.9 Cubic Bézier curve (`test_bezier_curve`)

| ID | Expected behaviour | Kind | Result |
|---|---|---|---|
| BEZ-01 | The curve starts at P0 and ends at P3 | H | Pass |
| BEZ-02 | The midpoint matches the hand-worked value (5, 6) | H | Pass |
| BEZ-03 | Agrees with de Casteljau's algorithm everywhere | X | Pass |
| BEZ-04 | Moving the control points moves the curve by the same amount | P | Pass |
| BEZ-05 | The curve stays inside the convex hull of its control points | P | Pass |
| BEZ-06 | End tangents point along the first and last control legs | P | Pass |
| BEZ-07 | Symmetric control points give a symmetric curve | P | Pass |
| BEZ-08 | Evenly spaced collinear control points give a straight, even line | E | Pass |
| BEZ-09 | `generateBezierCurve` returns steps + 1 points from P0 to P3 | E | Pass |

### 3.10 Catmull-Rom spline (`test_catmull_rom_spline`)

| ID | Expected behaviour | Kind | Result |
|---|---|---|---|
| CRS-01 | The spline passes through every control point | P | Pass |
| CRS-02 | A segment runs from its second point to its third | H | Pass |
| CRS-03 | The midpoint matches the hand-worked value (3.4375, 4.0625) | H | Pass |
| CRS-04 | The tangent at a control point is half the chord around it | P | Pass |
| CRS-05 | Neighbouring segments join smoothly (same tangent) | P | Pass |
| CRS-06 | Evenly spaced points on a line give a straight, even curve | E | Pass |
| CRS-07 | The number of generated points is (n - 1) × steps + 1 | E | Pass |
| CRS-08 | Fewer than two points give no curve; two give a straight segment | E | Pass |
| CRS-09 | Moving the control points moves the curve by the same amount | P | Pass |

### 3.11 Solar System scene (`test_solar_system_scene`)

The project models the real Solar System, so the scene is checked against real facts (planet order and relative size), not real scale.

| ID | Expected behaviour | Kind | Result |
|---|---|---|---|
| SCN-01 | The eight planets are listed in order from the Sun | H | Pass |
| SCN-02 | Each orbit is larger than the one inside it, so none cross | P | Pass |
| SCN-03 | Planet sizes follow the real Solar System ranking | P | Pass |
| SCN-04 | Every planet has its own colour, different from the scenery | P | Pass |
| SCN-05 | Saturn is the only planet drawn with rings | H | Pass |
| SCN-06 | Each planet's centre sits on its own orbit | P | Pass |
| SCN-07 | The whole scene fits on the canvas | P | Pass |
| SCN-08 | No planet overlaps the Sun or another planet | P | Pass |
| SCN-09 | The rendered image shows the Sun, every planet and Saturn's rings | H | Pass |
| SCN-10 | Every orbit is visible where no body is drawn over it | P | Pass |
| SCN-11 | Rendering is repeatable and leaves the corners empty | P | Pass |

### 3.12 Visual tests

| Test | Image in `output/` | What to look for |
|---|---|---|
| `raster_canvas_test` | `raster_canvas_test.bmp` | Horizontal, diagonal and vertical lines |
| `dda_graphics_test` | `dda_output.bmp` | Rays in every direction from the centre |
| `bresenham_graphics_test` | `bresenham_output.bmp` | Lines of every slope, including steep right-to-left lines |
| `polygon_operations_graphics_test` | `polygon_operations_output.bmp` | Original, translated, scaled and rotated copies with fixed points marked |
| `polygon_clipping_graphics_test` | `polygon_clipping_output.bmp` | Filled green result inside the white window |
| `polygon_filling_graphics_test` | `polygon_filling_output.bmp` | One convex and two concave shapes, fully filled |
| `circle_graphics_test` | `circle_graphics_test.bmp` | Concentric circles and the radius 0 centre pixel |
| `ellipse_graphics_test` | `ellipse_graphics_test.bmp` | Wide, tall and very flat ellipses; flat tips reach the white ticks |
| `bezier_graphics_test` | `bezier_output.bmp` | Curve from the first to the last control point, inside the control polygon |
| `catmull_rom_graphics_test` | `catmull_rom_output.bmp` | Spline through all six control points |
| `solar_system_graphics_test` | `solar_system_output.bmp` | Sun, eight planets on their orbits, Saturn's rings |

## 4. Defects Found and Fixed

| # | Defect | How it was found | Fix |
|---|---|---|---|
| 1 | Very flat ellipses (horizontal radius at least 16 times the vertical one) stopped one pixel short of their tips, and an ellipse with a vertical radius of 0 collapsed to its centre pixel. 765 of the 40,401 radius pairs from 0 to 200 were affected. | ELL-03, ELL-07 | The algorithm finishes the tips along the horizontal axis after region 2. Orbits and Saturn's ring were not affected and are drawn exactly as before. |
| 2 | On the scan line through a concave vertex, filling produced two spans sharing a pixel (for example 0..20 and 20..40), so that pixel was filled twice. | FIL-05, FIL-10 | Spans that overlap on the same scan line are joined. |
| 3 | The old graphics tests each contained a private copy of their algorithm, so they did not test the project's code. The copies had drifted: the Catmull-Rom test never reached the first or last control point, and the polygon test only translated. | Code review | Algorithms moved into headers that the demos, tests and scene all share. |
| 4 | Several graphics tests reported success even when no image was saved (for example when `output/` did not exist). | Running the tests | `saveVisualOutput()` creates `output/`, checks the save and returns a failing exit code. |
| 5 | White elements (the clipping window, polygon outlines) were drawn on a white background and could not be seen. | Inspecting the images | Visual tests use a dark background. |
| 6 | The GitHub Actions test step ran zero tests and still passed. | Running `ctest` | Tests are registered with CTest and the workflow uses `--no-tests=error`. |
| 7 | `raster_canvas.h` used `std::abs` without including `<cstdlib>`, which only compiled because GCC includes it indirectly. | Code review | Added the include. |
| 8 | Saturn's rings were chosen by checking whether the planet's name starts with "S", which would also match the Sun. | Code review | A `hasRings` flag on each planet. |

To confirm the tests can actually fail, deliberate faults were introduced and the tests were run against them: stopping the DDA loop one step early failed five DDA tests, the old ellipse header failed ELL-03 and ELL-07, and shrinking Earth below Venus failed SCN-03.

## 5. Known Limitations and Observations

These are behaviours of the current algorithms that were not changed, because they are standard textbook behaviour or are design decisions for the owners of that area.

- **Sutherland-Hodgman and concave polygons (CLP-09).** When the window cuts a concave polygon into separate pieces, the result is one polygon joined by edges along the window boundary. The area is correct and filling looks right, but drawing its outline shows an extra line along the window edge.
- **Fill span convention (FIL-02).** Rows include the lower edge and exclude the upper edge, but each span includes both its left and right ends. Two polygons sharing a vertical edge would both fill that column of pixels.
- **DDA rounding.** Drawing a line in the opposite direction can round half-pixel ties differently, so the two directions may differ by one pixel in a few places. Both stay within half a pixel of the true line (DDA-06).
- **Scene highlight.** Each planet's white highlight is drawn on the scene's white background, so it is not visible. A dark scene background would fix this; that is a scene design decision.
- **Orbit centre.** Orbits are centred on the Sun. In the real Solar System the Sun sits at one focus of each elliptical orbit, not at the centre. This matters only if the orbit maths aims for real orbital shapes.

## 6. Future Testing

The current tests cover the 2D raster pipeline. As the planned features are added, the same framework can test them:

- **Transformations and camera:** matrix multiplication, rotation about each axis, view and projection matrices against hand-worked results.
- **Animation:** planet positions after a known time step, and orbit speed ordering (inner planets move faster).
- **Integration:** the main `stellar` application rendering a frame and saving it as evidence.
