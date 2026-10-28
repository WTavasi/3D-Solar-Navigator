
#include <iostream>
#include <vector>

#include "algorithms/polygons/polygon_filling.h"

using namespace std;

void fillPolygon(const vector<Point>& polygon) {
    if (polygon.size() < 3) {
        cout << "A polygon must have at least three vertices.\n";
        return;
    }

    int minimumY = 0;
    int maximumY = 0;
    findScanlineRange(polygon, minimumY, maximumY);

    vector<FillSpan> spans = computeFillSpans(polygon);

    cout << "Filled scan-line spans:\n";

    for (int scanY = minimumY; scanY < maximumY; scanY++) {
        vector<FillSpan> row;

        for (const FillSpan& span : spans) {
            if (span.y == scanY) {
                row.push_back(span);
            }
        }

        cout << "Y = " << scanY << ": ";

        if (row.empty()) {
            cout << "No fill span\n";
            continue;
        }

        for (size_t index = 0; index < row.size(); index++) {
            cout << "X = " << row[index].startX
                 << " to " << row[index].endX;

            if (index + 1 < row.size()) {
                cout << ", ";
            }
        }

        cout << "\n";
    }
}

int main() {
    vector<Point> polygon = {
        {2, 2},
        {8, 2},
        {8, 7},
        {5, 10},
        {2, 7}
    };

    cout << "SCAN-LINE POLYGON FILLING ALGORITHM\n\n";

    cout << "Polygon vertices:\n";

    for (const Point& vertex : polygon) {
        cout << "(" << vertex.x << ", "
             << vertex.y << ")\n";
    }

    cout << "\n";

    fillPolygon(polygon);

    return 0;
}
