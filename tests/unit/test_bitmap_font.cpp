// Unit tests for the application's 5 x 7 bitmap font.

#include <string>

#include "app/bitmap_font.h"
#include "raster/raster_canvas.h"
#include "test_framework.h"

namespace {

// The canvas starts white, so text is drawn in black.
const Color INK = {0, 0, 0};

int litPixels(const font::Glyph& glyph) {
    int count = 0;
    for (const char* row : glyph.rows) {
        for (int column = 0; column < font::GLYPH_WIDTH; column++) {
            if (row[column] == '#') {
                count++;
            }
        }
    }
    return count;
}

}  // namespace

TEST_CASE("FONT-01 every glyph is 5 columns by 7 rows") {
    int count = 0;
    const font::Glyph* glyphs = font::glyphTable(count);

    CHECK(count > 36);
    for (int i = 0; i < count; i++) {
        for (const char* row : glyphs[i].rows) {
            CHECK_EQ(std::string(row).size(), static_cast<std::size_t>(font::GLYPH_WIDTH));
        }
    }
}

TEST_CASE("FONT-02 has every capital letter and digit") {
    for (char c = 'A'; c <= 'Z'; c++) {
        CHECK_EQ(font::glyphFor(c).character, c);
    }
    for (char c = '0'; c <= '9'; c++) {
        CHECK_EQ(font::glyphFor(c).character, c);
    }
}

TEST_CASE("FONT-03 lowercase draws as capitals and unknown characters as '?'") {
    CHECK_EQ(font::glyphFor('g').character, 'G');
    CHECK_EQ(font::glyphFor('#').character, '?');
    CHECK_EQ(font::glyphFor('\n').character, '?');
}

TEST_CASE("FONT-04 every visible glyph lights pixels and space lights none") {
    int count = 0;
    const font::Glyph* glyphs = font::glyphTable(count);

    for (int i = 0; i < count; i++) {
        if (glyphs[i].character == ' ') {
            CHECK_EQ(litPixels(glyphs[i]), 0);
        } else {
            CHECK(litPixels(glyphs[i]) > 0);
        }
    }
}

TEST_CASE("FONT-05 no two glyphs share the same shape") {
    int count = 0;
    const font::Glyph* glyphs = font::glyphTable(count);

    for (int a = 0; a < count; a++) {
        for (int b = a + 1; b < count; b++) {
            bool same = true;
            for (int row = 0; row < font::GLYPH_HEIGHT && same; row++) {
                same = std::string(glyphs[a].rows[row]) == glyphs[b].rows[row];
            }
            CHECK(!same);
        }
    }
}

TEST_CASE("FONT-06 text width counts characters and spacing") {
    CHECK_EQ(font::textWidth(""), 0);
    CHECK_EQ(font::textWidth("A"), 5);
    CHECK_EQ(font::textWidth("AB"), 11);
    CHECK_EQ(font::textWidth("AB", 2), 22);
    CHECK_EQ(font::lineHeight(1), 10);
    CHECK_EQ(font::lineHeight(3), 30);
}

TEST_CASE("FONT-07 drawText sets exactly the glyph's pixels") {
    RasterCanvas canvas(20, 10);
    int drawn = font::drawText(canvas, 0, 0, "L", INK);

    CHECK_EQ(drawn, litPixels(font::glyphFor('L')));
    CHECK(canvas.getPixel(0, 0) == INK);    // top of the upright
    CHECK(canvas.getPixel(4, 6) == INK);    // end of the foot
    CHECK(canvas.getPixel(4, 0) != INK);    // empty top-right
}

TEST_CASE("FONT-08 scale turns each font pixel into a square block") {
    RasterCanvas small(40, 40);
    RasterCanvas large(40, 40);

    int one = font::drawText(small, 0, 0, "E", INK, 1);
    int three = font::drawText(large, 0, 0, "E", INK, 3);

    CHECK_EQ(three, one * 9);
    CHECK(large.getPixel(2, 2) == INK);
}

TEST_CASE("FONT-09 text off the canvas is clipped, not drawn out of range") {
    RasterCanvas canvas(10, 10);

    int inside = font::drawText(canvas, 0, 0, "W", INK);
    int partly = font::drawText(canvas, 7, 7, "W", INK);
    int outside = font::drawText(canvas, -100, -100, "W", INK);

    CHECK(partly < inside);
    CHECK_EQ(outside, 0);
}

int main() {
    return testing::runAllTests();
}
