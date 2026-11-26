#pragma once
/// @file raster_surface.hpp
/// @brief Backend-agnostic offscreen CPU raster surface for testing and snapshots.

#include "enki/rendering/canvas.hpp"

#include <memory>
#include <string>

namespace enki {

class RasterSurface {
public:
    ~RasterSurface();
    RasterSurface(const RasterSurface&) = delete;
    RasterSurface& operator=(const RasterSurface&) = delete;

    static std::unique_ptr<RasterSurface> create(int width, int height);

    [[nodiscard]] Canvas& canvas();
    [[nodiscard]] int width() const;
    [[nodiscard]] int height() const;

    [[nodiscard]] Color readPixel(int x, int y) const;
    bool savePng(const std::string& path) const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    explicit RasterSurface(std::unique_ptr<Impl> impl);
};

} // namespace enki

