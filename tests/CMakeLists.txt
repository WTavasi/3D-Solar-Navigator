# Test targets for Project Stellar.
#
#   unit/         assertion-based tests for one algorithm or component
#   integration/  tests for the Solar System scene, which combines them
#   visual/       programs that draw each algorithm into output/*.bmp
#
# Run everything from the project root with:
#   cmake -S . -B build
#   cmake --build build
#   ctest --test-dir build --output-on-failure

# Adds tests/unit/<name>.cpp as an executable and registers it with CTest.
function(stellar_add_unit_test name)
    add_executable(${name} unit/${name}.cpp)
    target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/support)

    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES
        LABELS "unit"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endfunction()

# Adds tests/integration/<name>.cpp as an executable and registers it.
function(stellar_add_integration_test name)
    add_executable(${name} integration/${name}.cpp)
    target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/support)

    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES
        LABELS "integration"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endfunction()

# Adds tests/visual/<name>.cpp. It saves a BMP into the project's output/
# folder as visual evidence and fails if nothing is drawn or saved.
function(stellar_add_visual_test name)
    add_executable(${name} visual/${name}.cpp)
    target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/support)
    target_compile_definitions(${name} PRIVATE
        STELLAR_OUTPUT_DIR="${PROJECT_SOURCE_DIR}/output"
    )

    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES LABELS "visual")
endfunction()

# Unit tests
stellar_add_unit_test(test_raster_canvas)
stellar_add_unit_test(test_dda)
stellar_add_unit_test(test_bresenham)
stellar_add_unit_test(test_polygon_operations)
stellar_add_unit_test(test_polygon_clipping)
stellar_add_unit_test(test_polygon_filling)
stellar_add_unit_test(test_midpoint_circle)
stellar_add_unit_test(test_midpoint_ellipse)
stellar_add_unit_test(test_bezier_curve)
stellar_add_unit_test(test_catmull_rom_spline)

# Integration tests
stellar_add_integration_test(test_solar_system_scene)

# Visual tests (write images into output/)
stellar_add_visual_test(raster_canvas_test)
stellar_add_visual_test(dda_graphics_test)
stellar_add_visual_test(bresenham_graphics_test)
