#include <iostream>
#include <vector>
#include <utility>
#include <cmath>
#include "raster/raster_canvas.h"
 
using Point = std::pair<double, double>;
 
Point calculateCatmullRom(
   const Point& p0,
   const Point& p1,
   const Point& p2,
   const Point& p3,
   double t
) {
   double t2 = t * t;
   double t3 = t2 * t;
 
   double x = 0.5 * (
       2 * p1.first +
       (-p0.first + p2.first) * t +
       (2 * p0.first - 5 * p1.first +
        4 * p2.first - p3.first) * t2 +
       (-p0.first + 3 * p1.first -
        3 * p2.first + p3.first) * t3
   );
 
   double y = 0.5 * (
       2 * p1.second +
       (-p0.second + p2.second) * t +
       (2 * p0.second - 5 * p1.second +
        4 * p2.second - p3.second) * t2 +
       (-p0.second + 3 * p1.second -
        3 * p2.second + p3.second) * t3
   );
 
   return {x, y};
}
 
int main() {
   RasterCanvas canvas(800, 600);
 
   std::vector<Point> points = {
       {80, 350},
       {180, 150},
       {300, 250},
       {420, 100},
       {550, 300},
       {700, 180}
   };
 
   // Draw the control points' connecting lines.
   for (size_t i = 0; i + 1 < points.size(); i++) {
       canvas.drawLine(
           static_cast<int>(points[i].first),
           static_cast<int>(points[i].second),
           static_cast<int>(points[i + 1].first),
           static_cast<int>(points[i + 1].second),
           100, 100, 100
       );
   }
 
   // Draw the spline one segment at a time.
   for (size_t i = 0; i + 3 < points.size(); i++) {
       Point previous = points[i + 1];
 
       for (int step = 1; step <= 100; step++) {
           double t = step / 100.0;
 
           Point current = calculateCatmullRom(
               points[i],
               points[i + 1],
               points[i + 2],
               points[i + 3],
               t
           );
 
           canvas.drawLine(
               static_cast<int>(std::round(previous.first)),
               static_cast<int>(std::round(previous.second)),
               static_cast<int>(std::round(current.first)),
               static_cast<int>(std::round(current.second)),
               255, 255, 0
           );
 
           previous = current;
       }
   }
 
   canvas.saveBMP("output/catmull_rom_output.bmp");
 
   std::cout << "Catmull-Rom spline test completed.\n";
   return 0;
}
