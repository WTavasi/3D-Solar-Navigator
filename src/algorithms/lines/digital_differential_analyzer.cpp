
#include <iostream>
#include <vector>

#include "algorithms/lines/digital_differential_analyzer.h"

using namespace std;

int main() {
    int x1 = 2;
    int y1 = 3;
    int x2 = 10;
    int y2 = 7;

    vector<Pixel> points = drawDDA(x1, y1, x2, y2);

    cout << "DDA Line Drawing Algorithm\n";
    cout << "Start point: (" << x1 << ", " << y1 << ")\n";
    cout << "End point: (" << x2 << ", " << y2 << ")\n\n";

    cout << "Generated points:\n";

    for (const Pixel& point : points) {
        cout << "(" << point.x << ", " << point.y << ")\n";
    }

    return 0;
}
