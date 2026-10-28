#ifndef ALGORITHMS_GEOMETRY_H
#define ALGORITHMS_GEOMETRY_H

#include <ostream>

// Shared coordinate types for the graphics algorithms.

// A point with real-valued coordinates: polygon vertices, curve points.
struct Point {
    double x;
    double y;
};

// A whole-number position on the pixel grid: line drawing output.
struct Pixel {
    int x;
    int y;
};

inline bool operator==(const Pixel& first, const Pixel& second) {
    return first.x == second.x && first.y == second.y;
}

inline bool operator!=(const Pixel& first, const Pixel& second) {
    return !(first == second);
}

inline bool operator<(const Pixel& first, const Pixel& second) {
    return first.x < second.x ||
           (first.x == second.x && first.y < second.y);
}

inline std::ostream& operator<<(std::ostream& stream, const Point& point) {
    return stream << "(" << point.x << ", " << point.y << ")";
}

inline std::ostream& operator<<(std::ostream& stream, const Pixel& pixel) {
    return stream << "(" << pixel.x << ", " << pixel.y << ")";
}

#endif
