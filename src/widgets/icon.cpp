/// @file icon.cpp
/// @brief Implementation of the Icon widget.

#include "enki/widgets/icon.hpp"
#include "enki/core/string_utils.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/rendering/font_manager.hpp"
#include "enki/rendering/paint.hpp"
#include "enki/rendering/path.hpp"
#include <algorithm>
#include <iostream>
#include <unordered_set>

namespace enki {

namespace {

std::string encodeCodepointUtf8(uint32_t cp) {
    std::string out;
    if (cp <= 0x7F) {
        out.push_back(static_cast<char>(cp));
    } else if (cp <= 0x7FF) {
        out.push_back(static_cast<char>(0xC0 | ((cp >> 6) & 0x1F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0xFFFF) {
        out.push_back(static_cast<char>(0xE0 | ((cp >> 12) & 0x0F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0x10FFFF) {
        out.push_back(static_cast<char>(0xF0 | ((cp >> 18) & 0x07)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
    return out;
}

void ensureIconFontLoaded(const std::string& family) {
    if (family.empty()) return;
    static std::unordered_set<std::string> s_loaded;
    if (!s_loaded.insert(family).second) return;

    if (family == "Material Icons") {
        std::string font_path = resolveAssetPath("assets/fonts/MaterialIcons-Regular.ttf");
        if (!FontManager::loadFont(font_path, "Material Icons")) {
            std::cerr << "[RenderIcon] Warning: Could not load Material Icons from: " << font_path << "\n";
        }
    } else {
        std::string candidate = "assets/fonts/" + family + ".ttf";
        std::string font_path = resolveAssetPath(candidate);
        (void)FontManager::loadFont(font_path, family);
    }
}

} // namespace

struct RenderIcon::IconCache {
    Path        cached_svg_path;
    std::string cached_utf8;
    float       cached_dx = 0.0f;
    float       cached_dy = 0.0f;
    bool        font_cache_dirty = true;
};

RenderIcon::RenderIcon(IconData data, float size, Color color) 
    : data_(std::move(data)), size_(size), color_(color),
      cache_(std::make_unique<IconCache>()) {
    
    // Completely rely on Anu Layout Engine to enforce size.
    // This removes any need for custom layout computation overrides.
    ANUNodeStyleSetWidth(getAnuNode(), size_);
    ANUNodeStyleSetHeight(getAnuNode(), size_);
    
    if (data_.isSvg()) {
        rebuildSvgPath();
    }
}

RenderIcon::~RenderIcon() = default;

void RenderIcon::setIconData(const IconData& data) {
    if (data_ == data) return;
    data_ = data;
    if (cache_) cache_->font_cache_dirty = true;
    if (data_.isSvg()) {
        rebuildSvgPath();
    }
    markNeedsPaint(); // IconData change only affects painting
}

void RenderIcon::setSize(float size) {
    if (size_ == size) return;
    size_ = size;
    
    // Update Anu node styling
    ANUNodeStyleSetWidth(getAnuNode(), size_);
    ANUNodeStyleSetHeight(getAnuNode(), size_);
    
    if (cache_) cache_->font_cache_dirty = true;
    if (data_.isSvg()) {
        rebuildSvgPath();
    }
    markNeedsLayout(); 
}

void RenderIcon::setColor(Color color) {
    if (color_ == color) return;
    color_ = color;
    markNeedsPaint();
}

void RenderIcon::rebuildSvgPath() {
    if (!cache_) return;
    if (auto parsed = Path::fromSvgString(data_.svg_path)) {
        Rect bounds = parsed->computeTightBounds();
        float max_dim = std::max(bounds.width, bounds.height);
        if (max_dim > 0.0f) {
            float scale = size_ / max_dim;
            float dx = -bounds.x * scale + (size_ - bounds.width * scale) / 2.0f;
            float dy = -bounds.y * scale + (size_ - bounds.height * scale) / 2.0f;
            parsed->transform(scale, scale, dx, dy);
        }
        cache_->cached_svg_path = std::move(*parsed);
    } else {
        std::cerr << "[RenderIcon] Failed to parse SVG path: " << data_.svg_path << std::endl;
        cache_->cached_svg_path.reset();
    }
}

void RenderIcon::rebuildFontCache() {
    if (!cache_) return;
    ensureIconFontLoaded(data_.font_family);
    cache_->cached_utf8 = encodeCodepointUtf8(data_.codepoint);
    cache_->font_cache_dirty = true;
}

void RenderIcon::paint(PaintContext& context) {
    if (!cache_) return;

    Paint paint;
    paint.setColor(color_);
    paint.setAntiAlias(true);

    if (data_.isSvg()) {
        paint.setStyle(PaintStyle::Fill);
        context.canvas.save();
        context.canvas.translate(context.offset.x, context.offset.y);
        context.canvas.drawPath(cache_->cached_svg_path, paint);
        context.canvas.restore();
    } else {
        if (cache_->font_cache_dirty) {
            ensureIconFontLoaded(data_.font_family);
            cache_->cached_utf8 = encodeCodepointUtf8(data_.codepoint);
            const char* fam = data_.font_family.empty() ? nullptr : data_.font_family.c_str();
            Rect bounds = context.canvas.measureTextBounds(cache_->cached_utf8, size_, fam, false);
            cache_->cached_dx = (size_ - bounds.width) / 2.0f - bounds.x;
            cache_->cached_dy = (size_ - bounds.height) / 2.0f - bounds.y;
            cache_->font_cache_dirty = false;
        }
        if (!cache_->cached_utf8.empty()) {
            const char* fam = data_.font_family.empty() ? nullptr : data_.font_family.c_str();
            context.canvas.drawText(
                cache_->cached_utf8,
                Point{context.offset.x + cache_->cached_dx, context.offset.y + cache_->cached_dy},
                paint,
                size_,
                fam,
                false
            );
        }
    }
}

} // namespace enki
