// Unit tests for RasterCanvas: pixel storage, Bresenham drawLine and the
// BMP writer that every graphics test depends on.

#include <cstdint>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include "raster/raster_canvas.h"
#include "test_framework.h"

namespace {

const Color WHITE = {255, 255, 255};
const Color RED = {255, 0, 0};
const Color BLUE = {0, 0, 255};
const Color SPACE = {10, 12, 28};

std::ostream& operator<<(std::ostream& stream, const Color& color) {
    return stream << "rgb(" << static_cast<int>(color.red) << ", "
                  << static_cast<int>(color.green) << ", "
                  << static_cast<int>(color.blue) << ")";
}

int countPixelsNotMatching(const RasterCanvas& canvas, const Color& color) {
    int count = 0;

    for (int y = 0; y < canvas.getHeight(); y++) {
        for (int x = 0; x < canvas.getWidth(); x++) {
            if (canvas.getPixel(x, y) != color) {
                count++;
            }
        }
    }

    return count;
}

std::vector<std::uint8_t> readFile(const std::string& fileName) {
    std::ifstream file(fileName, std::ios::binary);

    return std::vector<std::uint8_t>(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>());
}

std::uint32_t read32(const std::vector<std::uint8_t>& bytes,
                     std::size_t offset) {
    return static_cast<std::uint32_t>(bytes[offset]) |
           (static_cast<std::uint32_t>(bytes[offset + 1]) << 8) |
           (static_cast<std::uint32_t>(bytes[offset + 2]) << 16) |
           (static_cast<std::uint32_t>(bytes[offset + 3]) << 24);
}

std::uint16_t read16(const std::vector<std::uint8_t>& bytes,
                     std::size_t offset) {
    return static_cast<std::uint16_t>(
        bytes[offset] | (bytes[offset + 1] << 8));
}

}  // namespace

TEST_CASE("RC-01 a new canvas is white by default") {
    RasterCanvas canvas(40, 30);

    CHECK_EQ(canvas.getWidth(), 40);
    CHECK_EQ(canvas.getHeight(), 30);
    CHECK_EQ(countPixelsNotMatching(canvas, WHITE), 0);
}

TEST_CASE("RC-02 the background colour fills every pixel") {
    RasterCanvas canvas(17, 9, SPACE.red, SPACE.green, SPACE.blue);

    CHECK_EQ(countPixelsNotMatching(canvas, SPACE), 0);
}

TEST_CASE("RC-03 setPixel stores the colour that getPixel reads back") {
    RasterCanvas canvas(10, 10);

    canvas.setPixel(3, 7, 12, 34, 56);
    canvas.setPixel(9, 0, BLUE);

    CHECK_EQ(canvas.getPixel(3, 7), (Color{12, 34, 56}));
    CHECK_EQ(canvas.getPixel(9, 0), BLUE);
    CHECK_EQ(countPixelsNotMatching(canvas, WHITE), 2);
}

TEST_CASE("RC-04 pixels outside the canvas are ignored safely") {
    RasterCanvas canvas(8, 6);

    canvas.setPixel(-1, 0, RED);
    canvas.setPixel(0, -1, RED);
    canvas.setPixel(8, 0, RED);
    canvas.setPixel(0, 6, RED);

    CHECK_EQ(countPixelsNotMatching(canvas, WHITE), 0);
    CHECK(canvas.contains(0, 0));
    CHECK(canvas.contains(7, 5));
    CHECK(!canvas.contains(8, 5));
    CHECK(!canvas.contains(7, 6));
    CHECK_EQ(canvas.getPixel(-5, 2), (Color{0, 0, 0}));
}

TEST_CASE("RC-05 drawLine colours both endpoints and nothing else") {
    RasterCanvas canvas(20, 10);

    canvas.drawLine(2, 4, 12, 4, RED);

    CHECK_EQ(canvas.getPixel(2, 4), RED);
    CHECK_EQ(canvas.getPixel(12, 4), RED);
    CHECK_EQ(canvas.getPixel(1, 4), WHITE);
    CHECK_EQ(canvas.getPixel(13, 4), WHITE);
    CHECK_EQ(countPixelsNotMatching(canvas, WHITE), 11);
}

TEST_CASE("RC-06 drawLine keeps only the visible part of an off-canvas line") {
    RasterCanvas canvas(20, 10);

    canvas.drawLine(-50, 5, 50, 5, RED);

    CHECK_EQ(countPixelsNotMatching(canvas, WHITE), 20);
    CHECK_EQ(canvas.getPixel(0, 5), RED);
    CHECK_EQ(canvas.getPixel(19, 5), RED);
}

TEST_CASE("RC-07 saveBMP writes a valid 24-bit BMP header") {
    RasterCanvas canvas(8, 4);
    const std::string fileName = "rc07_header.bmp";

    CHECK(canvas.saveBMP(fileName));

    std::vector<std::uint8_t> bytes = readFile(fileName);
    const std::size_t rowSize = 8 * 3;
    const std::size_t expectedSize = 54 + rowSize * 4;

    CHECK_EQ(bytes.size(), expectedSize);

    if (bytes.size() < 54) {
        return;
    }

    CHECK_EQ(bytes[0], 'B');
    CHECK_EQ(bytes[1], 'M');
    CHECK_EQ(read32(bytes, 2), expectedSize);   // file size
    CHECK_EQ(read32(bytes, 10), 54u);           // pixel data offset
    CHECK_EQ(read32(bytes, 14), 40u);           // DIB header size
    CHECK_EQ(read32(bytes, 18), 8u);            // width
    CHECK_EQ(read32(bytes, 22), 4u);            // height
    CHECK_EQ(read16(bytes, 26), 1);             // colour planes
    CHECK_EQ(read16(bytes, 28), 24);            // bits per pixel
}

TEST_CASE("RC-08 saveBMP pads every row to a multiple of 4 bytes") {
    // 5 pixels * 3 bytes = 15 bytes, so each row needs 1 byte of padding.
    RasterCanvas canvas(5, 3);
    const std::string fileName = "rc08_padding.bmp";

    CHECK(canvas.saveBMP(fileName));

    std::vector<std::uint8_t> bytes = readFile(fileName);

    CHECK_EQ(bytes.size(), static_cast<std::size_t>(54 + 16 * 3));

    if (bytes.size() != 54 + 16 * 3) {
        return;
    }

    for (int row = 0; row < 3; row++) {
        CHECK_EQ(static_cast<int>(bytes[54 + row * 16 + 15]), 0);
    }
}

TEST_CASE("RC-09 saveBMP stores rows bottom-up in BGR order") {
    RasterCanvas canvas(3, 2);
    const std::string fileName = "rc09_order.bmp";

    canvas.setPixel(0, 0, RED);    // top-left on screen
    canvas.setPixel(0, 1, BLUE);   // bottom-left on screen

    CHECK(canvas.saveBMP(fileName));

    std::vector<std::uint8_t> bytes = readFile(fileName);
    const std::size_t rowSize = 12;   // 9 bytes padded to 12

    if (bytes.size() != 54 + rowSize * 2) {
        CHECK_EQ(bytes.size(), 54 + rowSize * 2);
        return;
    }

    // The first stored row is the bottom of the image (y = 1): blue.
    CHECK_EQ(static_cast<int>(bytes[54]), 255);   // B
    CHECK_EQ(static_cast<int>(bytes[55]), 0);     // G
    CHECK_EQ(static_cast<int>(bytes[56]), 0);     // R

    // The second stored row is the top of the image (y = 0): red.
    CHECK_EQ(static_cast<int>(bytes[54 + rowSize]), 0);         // B
    CHECK_EQ(static_cast<int>(bytes[54 + rowSize + 1]), 0);     // G
    CHECK_EQ(static_cast<int>(bytes[54 + rowSize + 2]), 255);   // R
}

TEST_CASE("RC-10 saveBMP reports failure when the file cannot be created") {
    RasterCanvas canvas(4, 4);

    CHECK(!canvas.saveBMP("missing_directory/rc10.bmp"));
}

int main() {
    return testing::runAllTests();
}
