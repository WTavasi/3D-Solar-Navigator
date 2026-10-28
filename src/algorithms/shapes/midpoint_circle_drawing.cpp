
#include <iostream>
#include <set>
#include <utility>

#include "algorithms/shapes/midpoint_circle.h"

using namespace std;

int main() {
    int centerX = 10;
    int centerY = 10;
    int radius = 5;

    set<pair<int, int>> circlePoints =
        drawMidpointCircle(centerX, centerY, radius);

    cout << "MIDPOINT CIRCLE DRAWING ALGORITHM\n";
    cout << "Center: (" << centerX << ", " << centerY << ")\n";
    cout << "Radius: " << radius << "\n\n";

    cout << "Generated circle coordinates:\n";

    for (const auto& point : circlePoints) {
        cout << "(" << point.first << ", "
             << point.second << ")\n";
    }

    cout << "\nTotal unique points: " << circlePoints.size() << "\n";

    return 0;
}
