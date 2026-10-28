
#ifndef RASTER_CANVAS_H
#define RASTER_CANVAS_H

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

class RasterCanvas {
private:
    int width;
    int height;
    std::vector<std::uint8_t> pixels;

public:
    RasterCanvas(int canvasWidth, int canvasHeight)
        : width(canvasWidth),
          height(canvasHeight),
          pixels(canvasWidth * canvasHeight * 3, 255) {
    }

    int getWidth() const {
        return width;
    }

    int getHeight() const {
        return height;
    }

    void setPixel(int x, int y,
                  std::uint8_t red,
                  std::uint8_t green,
                  std::uint8_t blue) {
        if (x < 0 || x >= width ||
            y < 0 || y >= height) {
            return;
        }

        // Store the pixel in RGB order.
        int index = (y * width + x) * 3;

        pixels[index] = red;
        pixels[index + 1] = green;
        pixels[index + 2] = blue;
    }

    void drawLine(int x1, int y1, int x2, int y2,
                  std::uint8_t red = 0,
                  std::uint8_t green = 0,
                  std::uint8_t blue = 0) {
        int differenceX = std::abs(x2 - x1);
        int differenceY = std::abs(y2 - y1);

        int stepX = (x1 < x2) ? 1 : -1;
        int stepY = (y1 < y2) ? 1 : -1;

        int error = differenceX - differenceY;

        while (true) {
            setPixel(x1, y1, red, green, blue);

            if (x1 == x2 && y1 == y2) {
                break;
            }

            int doubledError = 2 * error;

            if (doubledError > -differenceY) {
                error -= differenceY;
                x1 += stepX;
            }

            if (doubledError < differenceX) {
                error += differenceX;
                y1 += stepY;
            }
        }
    }

    bool saveBMP(const std::string& filename) const {
        std::ofstream file(filename, std::ios::binary);

        if (!file) {
            return false;
        }

        const int rowSize = (width * 3 + 3) & ~3;
        const int imageSize = rowSize * height;
        const int fileSize = 54 + imageSize;

        auto write16 = [&file](std::uint16_t value) {
            file.put(static_cast<char>(value & 0xFF));
            file.put(static_cast<char>((value >> 8) & 0xFF));
        };

        auto write32 = [&file](std::uint32_t value) {
            file.put(static_cast<char>(value & 0xFF));
            file.put(static_cast<char>((value >> 8) & 0xFF));
            file.put(static_cast<char>((value >> 16) & 0xFF));
            file.put(static_cast<char>((value >> 24) & 0xFF));
        };

        // BMP file header
        file.put('B');
        file.put('M');
        write32(fileSize);
        write16(0);
        write16(0);
        write32(54);

        // DIB header
        write32(40);
        write32(width);
        write32(height);
        write16(1);
        write16(24);
        write32(0);
        write32(imageSize);
        write32(2835);
        write32(2835);
        write32(0);
        write32(0);

        // BMP stores rows from bottom to top.
        std::vector<char> row(rowSize, 0);

        for (int y = height - 1; y >= 0; --y) {
            for (int x = 0; x < width; ++x) {
                int index = (y * width + x) * 3;

                // BMP uses BGR order.
                row[x * 3] =
                    static_cast<char>(pixels[index + 2]);
                row[x * 3 + 1] =
                    static_cast<char>(pixels[index + 1]);
                row[x * 3 + 2] =
                    static_cast<char>(pixels[index]);
            }

            file.write(row.data(), rowSize);
        }

        return file.good();
    }
};

#endif
