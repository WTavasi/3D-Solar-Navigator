
#include <iostream>
#include <vector>

#include "algorithms/lines/bresenham_line_drawing.h"

using namespace std;

int main() {
    int startingX = 2;
    int startingY = 3;
    int endingX = 10;
    int endingY = 7;

    vector<Pixel> points = drawBresenhamLine(
        startingX, startingY, endingX, endingY
    );

    cout << "Bresenham's Line Drawing Algorithm\n";
    cout << "Starting point: (" << startingX
         << ", " << startingY << ")\n";

    cout << "Ending point: (" << endingX
         << ", " << endingY << ")\n\n";

    cout << "Generated points:\n";

    for (const Pixel& point : points) {
        cout << "(" << point.x << ", "
             << point.y << ")\n";
    }

    return 0;
}
