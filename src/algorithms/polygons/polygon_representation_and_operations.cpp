
#include <iomanip>
#include <iostream>
#include <vector>

#include "algorithms/polygons/polygon_representation_and_operations.h"

using namespace std;

void displayPolygon(const vector<Point>& vertices) {
    for (int index = 0; index < static_cast<int>(vertices.size()); index++) {
        cout << "("
             << vertices[index].x << ", "
             << vertices[index].y << ")";

        if (index < static_cast<int>(vertices.size()) - 1) {
            cout << " -> ";
        }
    }

    cout << " -> (" << vertices[0].x
         << ", " << vertices[0].y << ")\n";
}

int main() {
    vector<Point> triangle = {
        {2, 2},
        {6, 2},
        {4, 5}
    };

    cout << fixed << setprecision(2);

    cout << "POLYGON REPRESENTATION AND OPERATIONS\n\n";

    cout << "Original triangle:\n";
    displayPolygon(triangle);

    translatePolygon(triangle, 3, 2);

    cout << "\nAfter translation by (3, 2):\n";
    displayPolygon(triangle);

    scalePolygon(triangle, 2.0, {3, 2});

    cout << "\nAfter scaling by factor 2 around (3, 2):\n";
    displayPolygon(triangle);

    rotatePolygon(triangle, 90, {3, 2});

    cout << "\nAfter rotating 90 degrees around (3, 2):\n";
    displayPolygon(triangle);

    return 0;
}
