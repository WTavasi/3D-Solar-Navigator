#include <iostream>
#include <vector>
#include <utility>
#include <cmath>
#include "raster/raster_canvas.h"
 
using Point = std::pair<double, double>;
 
Point calculateBezier(
   const Point& p0,
   const Point& p1,
   const Point& p2,
   const Point& p3,
   double t
) {
   double u = 1.0 - t;
 
   double x =
       u * u * u * p0.first +
       3 * u * u * t * p1.first +
       3 * u * t * t * p2.first +
       t * t * t * p3.first;
 
   double y =
       u * u * u * p0.second +
       3 * u * u * t * p1.second +
       3 * u * t * t * p2.second +
       t * t * t * p3.second;
 
   return {x, y};
}
 
int main() {
   RasterCanvas canvas(800, 600);
 
   Point p0 = {100, 450};
   Point p1 = {200, 100};
   Point p2 = {600, 100};
   Point p3 = {700, 450};
 
   // Draw the control polygon.
   canvas.drawLine(100, 450, 200, 100, 100, 100, 100);
   canvas.drawLine(200, 100, 600, 100, 100, 100, 100);
   canvas.drawLine(600, 100, 700, 450, 100, 100, 100);
 
   Point previous = p0;
 
   for (int i = 1; i <= 1000; i++) {
       double t = i / 1000.0;
 
       Point current = calculateBezier(p0, p1, p2, p3, t);
 
       canvas.drawLine(
           static_cast<int>(std::round(previous.first)),
           static_cast<int>(std::round(previous.second)),
           static_cast<int>(std::round(current.first)),
           static_cast<int>(std::round(current.second)),
           0, 255, 255
       );
 
       previous = current;
   }
 
   canvas.saveBMP("output/bezier_output.bmp");
 
   std::cout << "Bezier curve test completed.\n";
   return 0;
}
