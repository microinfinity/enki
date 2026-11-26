#include "enki/rendering/paragraph.hpp"
#include "enki/rendering/font_manager.hpp"

#include <include/core/SkColor.h>
#include <include/core/SkFont.h>
#include <include/core/SkFontMetrics.h>
#include <include/core/SkFontMgr.h>
#include <include/core/SkFontStyle.h>
#include <include/core/SkString.h>
#include <include/core/SkTypeface.h>

#if defined(__ANDROID__)
#include <include/ports/SkFontMgr_android.h>
#elif defined(_WIN32)
#include <include/ports/SkTypeface_win.h>
#elif !defined(__EMSCRIPTEN__)
#include <include/ports/SkFontMgr_fontconfig.h>
#endif

#if !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)
#include <modules/skparagraph/include/DartTypes.h>
#include <modules/skparagraph/include/FontCollection.h>
#include <modules/skparagraph/include/Paragraph.h>
#include <modules/skparagraph/include/ParagraphBuilder.h>
#include <modules/skparagraph/include/ParagraphStyle.h>
#include <modules/skparagraph/include/TextShadow.h>
#include <modules/skparagraph/include/TextStyle.h>
#endif

#include <algorithm>
#include <sstream>
#include <utility>

namespace enki {

#if !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)

namespace {

skia::textlayout::TextAlign toSkTextAlign(ParagraphAlign align) {
    switch (align) {
        case ParagraphAlign::Left:    return skia::textlayout::TextAlign::kLeft;
        case ParagraphAlign::Right:   return skia::textlayout::TextAlign::kRight;
        case ParagraphAlign::Center:  return skia::textlayout::TextAlign::kCenter;
        case ParagraphAlign::Justify: return skia::textlayout::TextAlign::kJustify;
        case ParagraphAlign::Start:   return skia::textlayout::TextAlign::kStart;
        case ParagraphAlign::End:     return skia::textlayout::TextAlign::kEnd;
    }
    return skia::textlayout::TextAlign::kStart;
}

skia::textlayout::TextDirection toSkTextDirection(ParagraphDirection dir) {
    return dir == ParagraphDirection::RTL
        ? skia::textlayout::TextDirection::kRtl
        : skia::textlayout::TextDirection::kLtr;
}

skia::textlayout::TextDecoration toSkTextDecoration(uint32_t dec) {
    unsigned int res = skia::textlayout::TextDecoration::kNoDecoration;
    if (dec & ParagraphDecoration::Underline)   res |= skia::textlayout::TextDecoration::kUnderline;
    if (dec & ParagraphDecoration::Overline)    res |= skia::textlayout::TextDecoration::kOverline;
    if (dec & ParagraphDecoration::LineThrough) res |= skia::textlayout::TextDecoration::kLineThrough;
    return static_cast<skia::textlayout::TextDecoration>(res);
}

skia::textlayout::TextDecorationStyle toSkTextDecorationStyle(ParagraphDecorationStyle style) {
    switch (style) {
        case ParagraphDecorationStyle::Solid:  return skia::textlayout::TextDecorationStyle::kSolid;
        case ParagraphDecorationStyle::Double: return skia::textlayout::TextDecorationStyle::kDouble;
        case ParagraphDecorationStyle::Dotted: return skia::textlayout::TextDecorationStyle::kDotted;
        case ParagraphDecorationStyle::Dashed: return skia::textlayout::TextDecorationStyle::kDashed;
        case ParagraphDecorationStyle::Wavy:   return skia::textlayout::TextDecorationStyle::kWavy;
    }
    return skia::textlayout::TextDecorationStyle::kSolid;
}

skia::textlayout::TextStyle toSkTextStyle(const ParagraphTextStyle& s) {
    skia::textlayout::TextStyle sk;
    sk.setColor(static_cast<SkColor>(s.color));
    sk.setFontSize(s.font_size);

    SkFontStyle::Slant slant = s.italic ? SkFontStyle::kItalic_Slant : SkFontStyle::kUpright_Slant;
    sk.setFontStyle(SkFontStyle(s.font_weight, SkFontStyle::kNormal_Width, slant));

    if (!s.font_families.empty()) {
        std::vector<SkString> families;
        families.reserve(s.font_families.size());
        for (const auto& f : s.font_families) {
            families.emplace_back(f.c_str(), f.size());
        }
        sk.setFontFamilies(families);
    }

    sk.setLetterSpacing(s.letter_spacing);
    sk.setWordSpacing(s.word_spacing);

    if (s.height.has_value()) {
        sk.setHeight(*s.height);
        sk.setHeightOverride(true);
    }

    sk.setDecoration(toSkTextDecoration(s.decoration));
    sk.setDecorationColor(static_cast<SkColor>(s.decoration_color));
    sk.setDecorationStyle(toSkTextDecorationStyle(s.decoration_style));
    sk.setDecorationThicknessMultiplier(s.decoration_thickness);

    for (const auto& shadow : s.shadows) {
        sk.addShadow(skia::textlayout::TextShadow(
            static_cast<SkColor>(shadow.color),
            SkPoint::Make(shadow.offset.x, shadow.offset.y),
            shadow.blur_sigma
        ));
    }

    return sk;
}

skia::textlayout::FontCollection* getParagraphFontCollection() {
    return static_cast<skia::textlayout::FontCollection*>(FontManager::getNativeFontCollection());
}

} // namespace

struct TextParagraph::Impl {
    std::unique_ptr<skia::textlayout::Paragraph> paragraph;
};

TextParagraph::TextParagraph(std::unique_ptr<Impl> impl) : impl_(std::move(impl)) {}
TextParagraph::~TextParagraph() = default;

void TextParagraph::layout(float available_width) {
    if (impl_ && impl_->paragraph) {
        impl_->paragraph->layout(available_width);
    }
}

float TextParagraph::getHeight() const {
    return (impl_ && impl_->paragraph) ? impl_->paragraph->getHeight() : 0.0f;
}

float TextParagraph::getMaxIntrinsicWidth() const {
    return (impl_ && impl_->paragraph) ? impl_->paragraph->getMaxIntrinsicWidth() : 0.0f;
}

float TextParagraph::getMinIntrinsicWidth() const {
    return (impl_ && impl_->paragraph) ? impl_->paragraph->getMinIntrinsicWidth() : 0.0f;
}

float TextParagraph::getLongestLine() const {
    return (impl_ && impl_->paragraph) ? impl_->paragraph->getLongestLine() : 0.0f;
}

std::vector<Rect> TextParagraph::getRectsForRange(size_t start, size_t end) const {
    std::vector<Rect> out;
    if (!impl_ || !impl_->paragraph) {
        return out;
    }
    auto boxes = impl_->paragraph->getRectsForRange(
        static_cast<unsigned>(start),
        static_cast<unsigned>(end),
        skia::textlayout::RectHeightStyle::kTight,
        skia::textlayout::RectWidthStyle::kTight
    );
    out.reserve(boxes.size());
    for (const auto& tb : boxes) {
        out.push_back(Rect{tb.rect.fLeft, tb.rect.fTop, tb.rect.width(), tb.rect.height()});
    }
    return out;
}

size_t TextParagraph::getGlyphIndexAtCoordinate(float x, float y) const {
    if (!impl_ || !impl_->paragraph) {
        return 0;
    }
    auto pos = impl_->paragraph->getGlyphPositionAtCoordinate(x, y);
    return static_cast<size_t>(std::max<int32_t>(0, pos.position));
}

std::pair<size_t, size_t> TextParagraph::getWordBoundary(size_t offset) const {
    if (!impl_ || !impl_->paragraph) {
        return {0, 0};
    }
    auto range = impl_->paragraph->getWordBoundary(static_cast<uint32_t>(offset));
    return {range.start, range.end};
}

void TextParagraph::paint(Canvas& canvas, float x, float y) const {
    if (impl_ && impl_->paragraph) {
        canvas.drawParagraph(impl_->paragraph.get(), x, y);
    }
}

void* TextParagraph::getNativeHandle() const {
    return (impl_ && impl_->paragraph) ? impl_->paragraph.get() : nullptr;
}

struct TextParagraphBuilder::Impl {
    std::unique_ptr<skia::textlayout::ParagraphBuilder> builder;
};

TextParagraphBuilder::TextParagraphBuilder(std::unique_ptr<Impl> impl) : impl_(std::move(impl)) {}
TextParagraphBuilder::~TextParagraphBuilder() = default;

std::unique_ptr<TextParagraphBuilder> TextParagraphBuilder::make(const ParagraphStyle& style) {
    auto fc = getParagraphFontCollection();
    if (!fc) {
        return nullptr;
    }

    skia::textlayout::ParagraphStyle p_style;
    p_style.setTextAlign(toSkTextAlign(style.text_align));
    p_style.setTextDirection(toSkTextDirection(style.text_direction));
    p_style.setTextStyle(toSkTextStyle(style.default_style));
    if (style.max_lines.has_value()) {
        p_style.setMaxLines(*style.max_lines);
    }
    if (!style.ellipsis.empty()) {
        p_style.setEllipsis(SkString(style.ellipsis.c_str(), style.ellipsis.size()));
    }

    auto sk_builder = skia::textlayout::ParagraphBuilder::make(p_style, sk_ref_sp(fc));
    if (!sk_builder) {
        return nullptr;
    }

    auto impl = std::make_unique<Impl>();
    impl->builder = std::move(sk_builder);
    return std::unique_ptr<TextParagraphBuilder>(new TextParagraphBuilder(std::move(impl)));
}

void TextParagraphBuilder::pushStyle(const ParagraphTextStyle& style) {
    if (impl_ && impl_->builder) {
        impl_->builder->pushStyle(toSkTextStyle(style));
    }
}

void TextParagraphBuilder::pop() {
    if (impl_ && impl_->builder) {
        impl_->builder->pop();
    }
}

void TextParagraphBuilder::addText(std::string_view text) {
    if (impl_ && impl_->builder && !text.empty()) {
        impl_->builder->addText(text.data(), text.size());
    }
}

std::unique_ptr<TextParagraph> TextParagraphBuilder::build() {
    if (!impl_ || !impl_->builder) {
        return nullptr;
    }
    auto p_impl = std::make_unique<TextParagraph::Impl>();
    p_impl->paragraph = impl_->builder->Build();
    return std::unique_ptr<TextParagraph>(new TextParagraph(std::move(p_impl)));
}

#else

// Fallback implementation for Android / Emscripten without SkParagraph
struct TextParagraph::Impl {
    ParagraphStyle style;
    std::string text;
    ParagraphTextStyle effective_style;

    struct Line {
        std::string text;
        float width = 0.0f;
        float x_offset = 0.0f;
        float baseline_y = 0.0f;
    };
    std::vector<Line> lines;
    float max_intrinsic_width = 0.0f;
    float min_intrinsic_width = 0.0f;
    float longest_line = 0.0f;
    float calculated_height = 0.0f;

    static sk_sp<SkFontMgr> getFallbackMgr() {
        static sk_sp<SkFontMgr> s_mgr = []() {
#if defined(__ANDROID__)
            auto m = SkFontMgr_New_Android(nullptr);
            return m ? m : SkFontMgr::RefDefault();
#else
            return SkFontMgr::RefDefault();
#endif
        }();
        return s_mgr;
    }

    SkFont makeFont() const {
        sk_sp<SkTypeface> typeface;
        SkFontStyle::Slant slant = effective_style.italic ? SkFontStyle::kItalic_Slant : SkFontStyle::kUpright_Slant;
        SkFontStyle fs(effective_style.font_weight, SkFontStyle::kNormal_Width, slant);

        if (auto* fc = static_cast<skia::textlayout::FontCollection*>(FontManager::getNativeFontCollection())) {
            std::vector<SkString> fams;
            for (const auto& f : effective_style.font_families) {
                fams.emplace_back(f.c_str(), f.size());
            }
            auto matches = fc->findTypefaces(fams, fs);
            if (!matches.empty()) {
                typeface = matches.front();
            }
        }
        if (!typeface) {
            auto mgr = getFallbackMgr();
            if (mgr) {
                for (const auto& fam : effective_style.font_families) {
                    typeface = mgr->matchFamilyStyle(fam.c_str(), fs);
                    if (typeface) break;
                }
                if (!typeface) {
                    typeface = mgr->legacyMakeTypeface(nullptr, fs);
                }
            }
        }
        SkFont font(typeface, effective_style.font_size);
        font.setEdging(SkFont::Edging::kAntiAlias);
        return font;
    }
};

TextParagraph::TextParagraph(std::unique_ptr<Impl> impl) : impl_(std::move(impl)) {}
TextParagraph::~TextParagraph() = default;

void TextParagraph::layout(float available_width) {
    if (!impl_) return;
    impl_->lines.clear();
    impl_->max_intrinsic_width = 0.0f;
    impl_->min_intrinsic_width = 0.0f;
    impl_->longest_line = 0.0f;
    impl_->calculated_height = 0.0f;

    SkFont font = impl_->makeFont();
    SkFontMetrics metrics;
    font.getMetrics(&metrics);
    float raw_line_h = (-metrics.fAscent + metrics.fDescent + metrics.fLeading);
    if (raw_line_h <= 0.0f) raw_line_h = impl_->effective_style.font_size * 1.2f;
    float line_h = impl_->effective_style.height.has_value()
        ? (*impl_->effective_style.height * impl_->effective_style.font_size)
        : raw_line_h;
    float ascent = -metrics.fAscent;
    if (ascent <= 0.0f) ascent = impl_->effective_style.font_size * 0.8f;

    float full_w = impl_->text.empty()
        ? 0.0f
        : font.measureText(impl_->text.data(), impl_->text.size(), SkTextEncoding::kUTF8);
    impl_->max_intrinsic_width = full_w;
    impl_->min_intrinsic_width = std::min(full_w, impl_->effective_style.font_size);

    float effective_width = std::isinf(available_width) ? 0.0f : available_width;
    if (impl_->text.empty()) {
        impl_->calculated_height = line_h;
        return;
    }

    if (effective_width <= 0.0f || full_w <= effective_width) {
        impl_->lines.push_back({impl_->text, full_w, 0.0f, ascent});
    } else {
        std::istringstream iss(impl_->text);
        std::string word;
        std::string current_line;
        float current_w = 0.0f;
        while (iss >> word) {
            std::string candidate = current_line.empty() ? word : (current_line + " " + word);
            float cand_w = font.measureText(candidate.data(), candidate.size(), SkTextEncoding::kUTF8);
            if (cand_w <= effective_width || current_line.empty()) {
                current_line = std::move(candidate);
                current_w = cand_w;
            } else {
                impl_->lines.push_back({std::move(current_line), current_w, 0.0f, 0.0f});
                current_line = word;
                current_w = font.measureText(word.data(), word.size(), SkTextEncoding::kUTF8);
            }
        }
        if (!current_line.empty()) {
            impl_->lines.push_back({std::move(current_line), current_w, 0.0f, 0.0f});
        }
    }

    if (impl_->style.max_lines.has_value() && impl_->lines.size() > *impl_->style.max_lines) {
        impl_->lines.resize(*impl_->style.max_lines);
        if (!impl_->style.ellipsis.empty() && !impl_->lines.empty() && effective_width > 0.0f) {
            auto& last = impl_->lines.back();
            const std::string& ell = impl_->style.ellipsis;
            while (!last.text.empty() &&
                   font.measureText((last.text + ell).data(), (last.text + ell).size(), SkTextEncoding::kUTF8) > effective_width) {
                last.text.pop_back();
            }
            last.text += ell;
            last.width = font.measureText(last.text.data(), last.text.size(), SkTextEncoding::kUTF8);
        }
    }

    for (size_t i = 0; i < impl_->lines.size(); ++i) {
        auto& l = impl_->lines[i];
        impl_->longest_line = std::max(impl_->longest_line, l.width);
        l.baseline_y = static_cast<float>(i) * line_h + ascent;
        float avail_w = effective_width > 0.0f ? effective_width : l.width;
        if (impl_->style.text_align == ParagraphAlign::Center) {
            l.x_offset = std::max(0.0f, (avail_w - l.width) * 0.5f);
        } else if (impl_->style.text_align == ParagraphAlign::Right ||
                   (impl_->style.text_align == ParagraphAlign::End && impl_->style.text_direction == ParagraphDirection::LTR)) {
            l.x_offset = std::max(0.0f, avail_w - l.width);
        }
    }
    impl_->calculated_height = static_cast<float>(impl_->lines.size()) * line_h;
}

float TextParagraph::getHeight() const { return impl_ ? impl_->calculated_height : 0.0f; }
float TextParagraph::getMaxIntrinsicWidth() const { return impl_ ? impl_->max_intrinsic_width : 0.0f; }
float TextParagraph::getMinIntrinsicWidth() const { return impl_ ? impl_->min_intrinsic_width : 0.0f; }
float TextParagraph::getLongestLine() const { return impl_ ? impl_->longest_line : 0.0f; }

std::vector<Rect> TextParagraph::getRectsForRange(size_t, size_t) const { return {}; }
size_t TextParagraph::getGlyphIndexAtCoordinate(float, float) const { return 0; }
std::pair<size_t, size_t> TextParagraph::getWordBoundary(size_t) const { return {0, 0}; }

void TextParagraph::paint(Canvas& canvas, float offset_x, float offset_y) const {
    if (!impl_) return;
    const auto& s = impl_->effective_style;
    Paint paint;
    paint.setColor(s.color);
    paint.setAntiAlias(true);
    const char* fam = s.font_families.empty() ? "" : s.font_families.front().c_str();

    for (const auto& line : impl_->lines) {
        if (line.text.empty()) continue;
        float x = offset_x + line.x_offset;
        float y = offset_y + line.baseline_y;

        for (const auto& shadow : s.shadows) {
            Paint sp = paint;
            sp.setColor(shadow.color);
            canvas.drawText(line.text, Point{x + shadow.offset.x, y + shadow.offset.y}, sp, s.font_size, fam, s.font_weight >= 600);
        }
        canvas.drawText(line.text, Point{x, y}, paint, s.font_size, fam, s.font_weight >= 600);

        if (s.decoration != ParagraphDecoration::None) {
            Paint dp = paint;
            dp.setColor(s.decoration_color != 0 ? s.decoration_color : s.color);
            dp.setStrokeWidth(s.decoration_thickness > 0.0f ? s.decoration_thickness : 1.0f);
            if (s.decoration & ParagraphDecoration::Underline) {
                float uly = y + 2.0f;
                canvas.drawLine(Point{x, uly}, Point{x + line.width, uly}, dp);
            }
            if (s.decoration & ParagraphDecoration::LineThrough) {
                float lty = y - (s.font_size * 0.35f);
                canvas.drawLine(Point{x, lty}, Point{x + line.width, lty}, dp);
            }
        }
    }
}

void* TextParagraph::getNativeHandle() const { return nullptr; }

struct TextParagraphBuilder::Impl {
    ParagraphStyle style;
    std::vector<ParagraphTextStyle> stack;
    std::string text;
};

TextParagraphBuilder::TextParagraphBuilder(std::unique_ptr<Impl> impl) : impl_(std::move(impl)) {}
TextParagraphBuilder::~TextParagraphBuilder() = default;

std::unique_ptr<TextParagraphBuilder> TextParagraphBuilder::make(const ParagraphStyle& style) {
    auto impl = std::make_unique<Impl>();
    impl->style = style;
    impl->stack.push_back(style.default_style);
    return std::unique_ptr<TextParagraphBuilder>(new TextParagraphBuilder(std::move(impl)));
}

void TextParagraphBuilder::pushStyle(const ParagraphTextStyle& style) {
    if (impl_) {
        impl_->stack.push_back(style);
    }
}

void TextParagraphBuilder::pop() {
    if (impl_ && impl_->stack.size() > 1) {
        impl_->stack.pop_back();
    }
}

void TextParagraphBuilder::addText(std::string_view text) {
    if (impl_ && !text.empty()) {
        impl_->text.append(text.data(), text.size());
    }
}

std::unique_ptr<TextParagraph> TextParagraphBuilder::build() {
    if (!impl_) return nullptr;
    auto p_impl = std::make_unique<TextParagraph::Impl>();
    p_impl->style = impl_->style;
    p_impl->text = std::move(impl_->text);
    p_impl->effective_style = impl_->stack.empty() ? impl_->style.default_style : impl_->stack.back();
    return std::unique_ptr<TextParagraph>(new TextParagraph(std::move(p_impl)));
}

#endif

} // namespace enki

