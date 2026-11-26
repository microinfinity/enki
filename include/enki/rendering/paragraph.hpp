#pragma once

#include "enki/core/types.hpp"
#include "enki/rendering/canvas.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace enki {

enum class ParagraphAlign {
    Left,
    Right,
    Center,
    Justify,
    Start,
    End
};

enum class ParagraphDirection {
    LTR,
    RTL
};

enum class ParagraphDecorationStyle {
    Solid,
    Double,
    Dotted,
    Dashed,
    Wavy
};

namespace ParagraphDecoration {
    inline constexpr uint32_t None        = 0;
    inline constexpr uint32_t Underline   = 1 << 0;
    inline constexpr uint32_t Overline    = 1 << 1;
    inline constexpr uint32_t LineThrough = 1 << 2;
}

struct ParagraphShadow {
    Color color = 0x80000000;
    Point offset{0.0f, 2.0f};
    float blur_sigma = 2.0f;
};

struct ParagraphTextStyle {
    Color color = 0xFFFFFFFF;
    float font_size = 14.0f;
    int font_weight = 400;
    bool italic = false;
    std::vector<std::string> font_families;
    float letter_spacing = 0.0f;
    float word_spacing = 0.0f;
    std::optional<float> height;
    uint32_t decoration = ParagraphDecoration::None;
    Color decoration_color = 0x00000000;
    ParagraphDecorationStyle decoration_style = ParagraphDecorationStyle::Solid;
    float decoration_thickness = 1.0f;
    std::vector<ParagraphShadow> shadows;
};

struct ParagraphStyle {
    ParagraphAlign text_align = ParagraphAlign::Start;
    ParagraphDirection text_direction = ParagraphDirection::LTR;
    std::optional<size_t> max_lines;
    std::string ellipsis;
    ParagraphTextStyle default_style;
};

class TextParagraph {
public:
    ~TextParagraph();
    TextParagraph(const TextParagraph&) = delete;
    TextParagraph& operator=(const TextParagraph&) = delete;

    void layout(float available_width);

    float getHeight() const;
    float getMaxIntrinsicWidth() const;
    float getMinIntrinsicWidth() const;
    float getLongestLine() const;

    std::vector<Rect> getRectsForRange(size_t start, size_t end) const;
    size_t getGlyphIndexAtCoordinate(float x, float y) const;
    std::pair<size_t, size_t> getWordBoundary(size_t offset) const;

    void paint(Canvas& canvas, float x, float y) const;

    void* getNativeHandle() const;

private:
    friend class TextParagraphBuilder;
    struct Impl;
    std::unique_ptr<Impl> impl_;
    explicit TextParagraph(std::unique_ptr<Impl> impl);
};

class TextParagraphBuilder {
public:
    ~TextParagraphBuilder();
    TextParagraphBuilder(const TextParagraphBuilder&) = delete;
    TextParagraphBuilder& operator=(const TextParagraphBuilder&) = delete;

    static std::unique_ptr<TextParagraphBuilder> make(const ParagraphStyle& style);

    void pushStyle(const ParagraphTextStyle& style);
    void pop();
    void addText(std::string_view text);
    std::unique_ptr<TextParagraph> build();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    explicit TextParagraphBuilder(std::unique_ptr<Impl> impl);
};

} // namespace enki

