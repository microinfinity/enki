#include "enki/rendering/raster_surface.hpp"

#include <include/core/SkCanvas.h>
#include <include/core/SkPixmap.h>
#include <include/core/SkStream.h>
#include <include/core/SkSurface.h>
#include <include/encode/SkPngEncoder.h>

#include <utility>

namespace enki {

extern std::unique_ptr<Canvas> createCanvasWrapper(void* sk_canvas);

struct RasterSurface::Impl {
    int width = 0;
    int height = 0;
    sk_sp<SkSurface> surface;
    std::unique_ptr<Canvas> canvas_wrapper;
};

RasterSurface::RasterSurface(std::unique_ptr<Impl> impl) : impl_(std::move(impl)) {}
RasterSurface::~RasterSurface() = default;

std::unique_ptr<RasterSurface> RasterSurface::create(int width, int height) {
    if (width <= 0 || height <= 0) {
        return nullptr;
    }
    auto surface = SkSurface::MakeRasterN32Premul(width, height);
    if (!surface || !surface->getCanvas()) {
        return nullptr;
    }
    auto wrapper = createCanvasWrapper(surface->getCanvas());
    if (!wrapper) {
        return nullptr;
    }

    auto impl = std::make_unique<Impl>();
    impl->width = width;
    impl->height = height;
    impl->surface = std::move(surface);
    impl->canvas_wrapper = std::move(wrapper);
    return std::unique_ptr<RasterSurface>(new RasterSurface(std::move(impl)));
}

Canvas& RasterSurface::canvas() {
    return *impl_->canvas_wrapper;
}

int RasterSurface::width() const {
    return impl_ ? impl_->width : 0;
}

int RasterSurface::height() const {
    return impl_ ? impl_->height : 0;
}

Color RasterSurface::readPixel(int x, int y) const {
    if (!impl_ || !impl_->surface) {
        return 0;
    }
    SkPixmap pixmap;
    if (!impl_->surface->peekPixels(&pixmap)) {
        return 0;
    }
    if (x < 0 || y < 0 || x >= pixmap.width() || y >= pixmap.height()) {
        return 0;
    }
    return static_cast<Color>(pixmap.getColor(x, y));
}

bool RasterSurface::savePng(const std::string& path) const {
    if (!impl_ || !impl_->surface || path.empty()) {
        return false;
    }
    SkPixmap pixmap;
    if (!impl_->surface->peekPixels(&pixmap)) {
        return false;
    }
    SkFILEWStream stream(path.c_str());
    SkPngEncoder::Options opts;
    return SkPngEncoder::Encode(&stream, pixmap, opts);
}

} // namespace enki

