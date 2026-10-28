# Test targets for Project Stellar.
#
#   unit/         assertion-based tests for one algorithm or component
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

stellar_add_unit_test(test_raster_canvas)
stellar_add_unit_test(test_dda)
stellar_add_unit_test(test_bresenham)
stellar_add_unit_test(test_polygon_operations)
stellar_add_unit_test(test_polygon_clipping)
stellar_add_unit_test(test_polygon_filling)
stellar_add_unit_test(test_midpoint_circle)
stellar_add_unit_test(test_midpoint_ellipse)
stellar_add_unit_test(test_bezier_curve)
