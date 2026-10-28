
#include <iomanip>
#include <iostream>
#include <vector>

#include "algorithms/curves/bezier_curve.h"

using namespace std;

int main() {
    Point startingPoint = {0, 0};
    Point firstControlPoint = {2, 8};
    Point secondControlPoint = {8, 8};
    Point endingPoint = {10, 0};

    vector<Point> curvePoints = generateBezierCurve(
        startingPoint,
        firstControlPoint,
        secondControlPoint,
        endingPoint,
        20
    );

    cout << fixed << setprecision(2);
    cout << "CUBIC BEZIER CURVE\n\n";

    cout << "Control points:\n";
    cout << "P0: (" << startingPoint.x
         << ", " << startingPoint.y << ")\n";

    cout << "P1: (" << firstControlPoint.x
         << ", " << firstControlPoint.y << ")\n";

    cout << "P2: (" << secondControlPoint.x
         << ", " << secondControlPoint.y << ")\n";

    cout << "P3: (" << endingPoint.x
         << ", " << endingPoint.y << ")\n\n";

    cout << "Generated curve points:\n";

    for (const Point& point : curvePoints) {
        cout << "(" << point.x << ", "
             << point.y << ")\n";
    }

    cout << "\nTotal points: " << curvePoints.size() << "\n";

    return 0;
}
