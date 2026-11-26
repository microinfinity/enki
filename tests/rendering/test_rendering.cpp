/// @file test_rendering.cpp
/// @brief Unit tests for ENKI Rendering wrappers.

#include "enki/rendering/canvas.hpp"
#include "enki/rendering/raster_surface.hpp"
#include "enki/rendering/path.hpp"
#include "enki/rendering/color.hpp"
#include <cassert>
#include <cstdio>

using namespace enki;

void test_colors() {
    assert(Colors::White == 0xFFFFFFFF);
    assert(Colors::Black == 0xFF000000);
    assert(makeColorRGB(255, 0, 0) == Colors::Red);
    assert(makeColorARGB(128, 0, 255, 0) == 0x8000FF00);
    printf("  [PASS] Colors\n");
}

void test_path() {
    Path p;
    p.moveTo(0, 0);
    p.lineTo(100, 0);
    p.lineTo(100, 100);
    p.close();

    Rect bounds = p.getBounds();
    assert(bounds.width == 100 && bounds.height == 100);
    assert(bounds.x == 0 && bounds.y == 0);

    printf("  [PASS] Path\n");
}

void test_canvas() {
    // Create an offscreen raster surface
    auto surface = RasterSurface::create(100, 100);
    assert(surface != nullptr);
    Canvas& canvas = surface->canvas();

    // Test basic drawing commands (no crash expected)
    canvas.clear(Colors::White);

    Paint paint;
    paint.setColor(Colors::Red);
    canvas.drawRect(Rect{10, 10, 50, 50}, paint);

    Path p;
    p.addCircle(50, 50, 20);
    canvas.drawPath(p, paint);

    // Verify a pixel
    Color c = surface->readPixel(15, 15);
    assert(c == Colors::Red);

    printf("  [PASS] Canvas drawing\n");
}

int main() {
    printf("=== Rendering Tests ===\n");
    test_colors();
    test_path();
    test_canvas();
    printf("=== All Rendering tests passed! ===\n");
    return 0;
}
