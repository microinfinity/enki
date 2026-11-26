#pragma once
/// @file path.hpp
/// @brief Abstract 2D vector path for drawing.

#include "enki/core/types.hpp"
#include <memory>
#include <optional>
#include <string_view>

namespace enki {

/// Represents a 2D path (lines, curves, etc.).
class Path {
public:
    Path();
    ~Path();

    Path(const Path&) = delete;
    Path& operator=(const Path&) = delete;
    Path(Path&&) noexcept;
    Path& operator=(Path&&) noexcept;

    /// Parse an SVG path data string ("M... L... Z") into a Path.
    [[nodiscard]] static std::optional<Path> fromSvgString(std::string_view svg_d);

    void moveTo(float x, float y);
    void lineTo(float x, float y);
    void quadTo(float x1, float y1, float x2, float y2);
    void cubicTo(float x1, float y1, float x2, float y2, float x3, float y3);
    void close();
    void reset();

    void addRect(const Rect& rect);
    void addCircle(float x, float y, float radius);
    void addRoundRect(const Rect& rect, const BorderRadius& radius);
    inline void addRRect(const Rect& rect, const BorderRadius& radius) { addRoundRect(rect, radius); }

    /// Apply scale then translation in-place.
    void transform(float sx, float sy, float tx = 0.0f, float ty = 0.0f);

    [[nodiscard]] bool isEmpty() const;
    [[nodiscard]] Rect getBounds() const;
    [[nodiscard]] Rect computeTightBounds() const;

    /// Internal: Get native Skia path handle
    void* getNativeHandle() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace enki
