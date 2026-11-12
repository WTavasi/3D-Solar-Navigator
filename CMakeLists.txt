cmake_minimum_required(VERSION 3.20)

project(ProjectStellar
    VERSION 1.0
    DESCRIPTION "Project Stellar - Solar System Computer Graphics"
    LANGUAGES CXX
)

# Use the C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Shared directory for header files
include_directories(${PROJECT_SOURCE_DIR}/include)

# Main application
add_executable(stellar
    src/main.cpp
    src/camera/camera.cpp
)

# Line drawing algorithms
add_executable(digital_differential_analyzer
    src/algorithms/lines/digital_differential_analyzer.cpp
)

add_executable(bresenham_line_drawing
    src/algorithms/lines/bresenham_line_drawing.cpp
)

# Polygon algorithms
add_executable(polygon_representation_and_operations
    src/algorithms/polygons/polygon_representation_and_operations.cpp
)

add_executable(polygon_clipping
    src/algorithms/polygons/polygon_clipping.cpp
)

add_executable(polygon_filling
    src/algorithms/polygons/polygon_filling.cpp
)

# Circle and ellipse algorithms
add_executable(midpoint_circle_drawing
    src/algorithms/shapes/midpoint_circle_drawing.cpp
)

add_executable(midpoint_ellipse_drawing
    src/algorithms/shapes/midpoint_ellipse_drawing.cpp
)

# Curve algorithms
add_executable(bezier_curve
    src/algorithms/curves/bezier_curve.cpp
)

add_executable(catmull_rom_spline
    src/algorithms/curves/catmull_rom_spline.cpp
)

# Tests (see docs/TESTING.md)
option(STELLAR_BUILD_TESTS "Build the unit, integration and visual tests" ON)

if(STELLAR_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Interactive 3D camera demonstration (Windows)
if(WIN32)
    add_executable(camera_interactive_demo WIN32
        src/camera/camera_interactive_demo.cpp
        src/camera/camera.cpp
    )

    target_include_directories(camera_interactive_demo PRIVATE
        ${PROJECT_SOURCE_DIR}/include
    )

    target_link_libraries(camera_interactive_demo PRIVATE
        user32
        gdi32
    )
endif()
