
#include <iostream>
#include <set>
#include <utility>

#include "algorithms/shapes/midpoint_ellipse.h"

using namespace std;

int main() {
    int centerX = 15;
    int centerY = 10;
    int horizontalRadius = 8;
    int verticalRadius = 4;

    set<pair<int, int>> ellipsePoints = drawMidpointEllipse(
        centerX,
        centerY,
        horizontalRadius,
        verticalRadius
    );

    cout << "MIDPOINT ELLIPSE DRAWING ALGORITHM\n";
    cout << "Center: (" << centerX << ", " << centerY << ")\n";
    cout << "Horizontal radius: " << horizontalRadius << "\n";
    cout << "Vertical radius: " << verticalRadius << "\n\n";

    cout << "Generated ellipse coordinates:\n";

    for (const auto& point : ellipsePoints) {
        cout << "(" << point.first << ", "
             << point.second << ")\n";
    }

    cout << "\nTotal unique points: "
         << ellipsePoints.size() << "\n";

    return 0;
}
