
#include <iomanip>
#include <iostream>
#include <vector>

#include "algorithms/curves/catmull_rom_spline.h"

using namespace std;

int main() {
    vector<Point> controlPoints = {
        {0, 0},
        {2, 5},
        {5, 3},
        {8, 7},
        {10, 2}
    };

    vector<Point> curvePoints =
        generateCatmullRomSpline(controlPoints, 10);

    cout << fixed << setprecision(2);
    cout << "CATMULL-ROM SPLINE\n\n";

    cout << "Control points:\n";

    for (const Point& point : controlPoints) {
        cout << "(" << point.x << ", "
             << point.y << ")\n";
    }

    cout << "\nGenerated spline points:\n";

    for (const Point& point : curvePoints) {
        cout << "(" << point.x << ", "
             << point.y << ")\n";
    }

    cout << "\nTotal generated points: "
         << curvePoints.size() << "\n";

    return 0;
}
