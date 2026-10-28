
#include <iostream>
#include <vector>

#include "algorithms/polygons/polygon_clipping.h"

using namespace std;

void displayPolygon(const vector<Point>& polygon) {
    if (polygon.empty()) {
        cout << "No polygon remains inside the clipping window.\n";
        return;
    }

    for (size_t index = 0; index < polygon.size(); index++) {
        cout << "(" << polygon[index].x
             << ", " << polygon[index].y << ")";

        if (index + 1 < polygon.size()) {
            cout << " -> ";
        }
    }

    cout << " -> (" << polygon[0].x
         << ", " << polygon[0].y << ")\n";
}

int main() {
    vector<Point> polygon = {
        {2, 2},
        {8, 2},
        {8, 7},
        {5, 10},
        {2, 7}
    };

    double minimumX = 3;
    double minimumY = 3;
    double maximumX = 7;
    double maximumY = 8;

    cout << "SUTHERLAND-HODGMAN POLYGON CLIPPING\n\n";

    cout << "Original polygon:\n";
    displayPolygon(polygon);

    cout << "\nClipping window:\n";
    cout << "Minimum X: " << minimumX << "\n";
    cout << "Minimum Y: " << minimumY << "\n";
    cout << "Maximum X: " << maximumX << "\n";
    cout << "Maximum Y: " << maximumY << "\n\n";

    vector<Point> clippedPolygon = sutherlandHodgmanClip(
        polygon, minimumX, minimumY, maximumX, maximumY
    );

    cout << "Clipped polygon:\n";
    displayPolygon(clippedPolygon);

    return 0;
}
