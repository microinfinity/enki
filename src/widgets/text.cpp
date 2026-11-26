/// @file text.cpp
/// @brief Advanced Typography & Text Widgets implementation using TextParagraph and Anu Layout.

#include "enki/widgets/text.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/rendering/paint.hpp"
#include "enki/rendering/paragraph.hpp"
#include "enki/platform/platform.hpp"
#include <layout_engine/Anu.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <iostream>

namespace enki {

namespace {

// ════════════════════════════════════════════════════════════════
// Paragraph Style Conversion Helpers
// ════════════════════════════════════════════════════════════════

ParagraphAlign toParagraphAlign(TextAlign align) {
    switch (align) {
        case TextAlign::Left:    return ParagraphAlign::Left;
        case TextAlign::Right:   return ParagraphAlign::Right;
        case TextAlign::Center:  return ParagraphAlign::Center;
        case TextAlign::Justify: return ParagraphAlign::Justify;
        case TextAlign::Start:   return ParagraphAlign::Start;
        case TextAlign::End:     return ParagraphAlign::End;
    }
    return ParagraphAlign::Start;
}

ParagraphDirection toParagraphDirection(TextDirection dir) {
    return dir == TextDirection::RTL ? ParagraphDirection::RTL : ParagraphDirection::LTR;
}

uint32_t toParagraphDecoration(TextDecoration d) {
    uint32_t res = ParagraphDecoration::None;
    if (d & TextDecoration::Underline)   res |= ParagraphDecoration::Underline;
    if (d & TextDecoration::Overline)    res |= ParagraphDecoration::Overline;
    if (d & TextDecoration::LineThrough) res |= ParagraphDecoration::LineThrough;
    return res;
}

ParagraphDecorationStyle toParagraphDecorationStyle(TextDecorationStyle s) {
    switch (s) {
        case TextDecorationStyle::Solid:  return ParagraphDecorationStyle::Solid;
        case TextDecorationStyle::Double: return ParagraphDecorationStyle::Double;
        case TextDecorationStyle::Dotted: return ParagraphDecorationStyle::Dotted;
        case TextDecorationStyle::Dashed: return ParagraphDecorationStyle::Dashed;
        case TextDecorationStyle::Wavy:   return ParagraphDecorationStyle::Wavy;
    }
    return ParagraphDecorationStyle::Solid;
}

ParagraphTextStyle toParagraphTextStyle(const TextStyle& s) {
    ParagraphTextStyle pt;
    pt.color = s.color;
    pt.font_size = s.font_size;
    pt.font_weight = static_cast<int>(s.font_weight);
    pt.italic = (s.font_style == FontStyle::Italic);

    if (!s.font_family.empty() || !s.font_families.empty()) {
        if (!s.font_family.empty()) {
            pt.font_families.push_back(s.font_family);
        }
        for (const auto& fam : s.font_families) {
            pt.font_families.push_back(fam);
        }
#if defined(__EMSCRIPTEN__)
        pt.font_families.emplace_back("sans-serif");
#endif
    }
#if defined(_WIN32)
    else {
        pt.font_families.emplace_back("Segoe UI");
    }
#elif defined(__EMSCRIPTEN__)
    else {
        pt.font_families.emplace_back("sans-serif");
        pt.font_families.emplace_back("default");
    }
#endif

    pt.letter_spacing = s.letter_spacing;
    pt.word_spacing = s.word_spacing;
    pt.height = s.height;
    pt.decoration = toParagraphDecoration(s.decoration);
    pt.decoration_color = s.decoration_color;
    pt.decoration_style = toParagraphDecorationStyle(s.decoration_style);
    pt.decoration_thickness = s.decoration_thickness;

    for (const auto& shadow : s.shadows) {
        pt.shadows.push_back({shadow.color, shadow.offset, shadow.blur_radius * 0.5f});
    }

    return pt;
}

TextStyle mergeStyles(const TextStyle& parent, const std::optional<TextStyle>& child) {
    if (!child) return parent;
    TextStyle res = *child;
    if (res.font_family.empty()) res.font_family = parent.font_family;
    if (res.font_families.empty()) res.font_families = parent.font_families;
    return res;
}

} // namespace

// ════════════════════════════════════════════════════════════════
// ParagraphBuilderContext Implementation
// ════════════════════════════════════════════════════════════════

class ParagraphBuilderContext {
public:
    struct SpanRange {
        size_t start;
        size_t end;
        const TextSpan* span;
    };

    explicit ParagraphBuilderContext(TextParagraphBuilder* b) : builder(b), current_index(0) {}

    void pushStyle(const TextStyle& style) {
        builder->pushStyle(toParagraphTextStyle(style));
    }

    void popStyle() {
        builder->pop();
    }

    void addText(const std::string& text, const TextSpan* span = nullptr) {
        if (!text.empty()) {
            builder->addText(text);
            accumulated_text += text;
            size_t end_index = current_index + text.size();
            if (span && (span->on_click || span->on_hover)) {
                interactive_spans.push_back({current_index, end_index, span});
            }
            current_index = end_index;
        }
    }

    TextParagraphBuilder* builder;
    size_t current_index;
    std::string accumulated_text;
    std::vector<SpanRange> interactive_spans;
};

void TextSpan::build(ParagraphBuilderContext& builder, const TextStyle& inheritedStyle) const {
    TextStyle effectiveStyle = mergeStyles(inheritedStyle, style);
    builder.pushStyle(effectiveStyle);

    if (!text.empty()) {
        builder.addText(text, this);
    }

    for (const auto& child : children) {
        if (child) {
            child->build(builder, effectiveStyle);
        }
    }

    builder.popStyle();
}

// ════════════════════════════════════════════════════════════════
// RenderParagraph Internal Implementation
// ════════════════════════════════════════════════════════════════

struct RenderParagraph::Impl {
    std::unique_ptr<TextParagraph> paragraph;
    float current_layout_width = -1.0f;
    std::string full_text;
    std::vector<ParagraphBuilderContext::SpanRange> interactive_spans;
    const TextSpan* hovered_span = nullptr;

    void build(const InlineSpan* rootSpan,
               const TextStyle& defaultStyle,
               TextAlign textAlign,
               TextDirection textDirection,
               TextOverflow overflow,
               std::optional<size_t> maxLines,
               bool softWrap) {
        ParagraphStyle pStyle;
        pStyle.text_align = toParagraphAlign(textAlign);
        pStyle.text_direction = toParagraphDirection(textDirection);
        pStyle.default_style = toParagraphTextStyle(defaultStyle);

        if (maxLines.has_value() && *maxLines > 0) {
            pStyle.max_lines = *maxLines;
        } else if (!softWrap) {
            pStyle.max_lines = 1;
        }

        if (overflow == TextOverflow::Ellipsis) {
            pStyle.ellipsis = "...";
        }

        auto builder = TextParagraphBuilder::make(pStyle);
        if (!builder) {
            paragraph = nullptr;
            full_text.clear();
            return;
        }

        ParagraphBuilderContext ctx(builder.get());
        if (rootSpan) {
            rootSpan->build(ctx, defaultStyle);
        }

        full_text = std::move(ctx.accumulated_text);
        interactive_spans = std::move(ctx.interactive_spans);
        paragraph = builder->build();
        current_layout_width = -1.0f;
    }

    void layout(float width) {
        float effective_width = std::max(0.0f, width);
        if (paragraph && current_layout_width != effective_width) {
            paragraph->layout(effective_width);
            current_layout_width = effective_width;
        }
    }
};

// ════════════════════════════════════════════════════════════════
// RenderParagraph Implementation
// ════════════════════════════════════════════════════════════════

RenderParagraph::RenderParagraph(std::shared_ptr<InlineSpan> span,
                                 TextStyle defaultStyle,
                                 TextAlign textAlign,
                                 TextDirection textDirection,
                                 TextOverflow overflow,
                                 std::optional<size_t> maxLines,
                                 bool softWrap)
    : span_(std::move(span)),
      default_style_(std::move(defaultStyle)),
      text_align_(textAlign),
      text_direction_(textDirection),
      overflow_(overflow),
      max_lines_(maxLines),
      soft_wrap_(softWrap),
      impl_(std::make_unique<Impl>()) {

    ANUNodeSetContext(anuNode(), this);
    ANUNodeSetMeasureFunc(anuNode(), &RenderParagraph::measureText);
    rebuildParagraph();
    if (impl_) {
        text_data_ = impl_->full_text;
    }
}

RenderParagraph::~RenderParagraph() {
    ANUNodeSetMeasureFunc(anuNode(), nullptr);
    ANUNodeSetContext(anuNode(), nullptr);
}

void RenderParagraph::setText(const std::string& data,
                              const TextStyle& style,
                              TextAlign align,
                              TextDirection dir,
                              TextOverflow overflow,
                              std::optional<size_t> maxLines,
                              bool softWrap) {
    if (text_data_ == data &&
        default_style_ == style &&
        text_align_ == align &&
        text_direction_ == dir &&
        overflow_ == overflow &&
        max_lines_ == maxLines &&
        soft_wrap_ == softWrap) {
        return; // Fast path: zero allocation, zero shaping, zero flexbox dirtying
    }

    if (impl_ && impl_->hovered_span) {
        if (impl_->hovered_span->on_hover) {
            impl_->hovered_span->on_hover(false);
        }
        impl_->hovered_span = nullptr;
    }

    bool layout_changed = (text_data_ != data ||
                           default_style_.font_size != style.font_size ||
                           default_style_.font_weight != style.font_weight ||
                           default_style_.font_family != style.font_family ||
                           default_style_.letter_spacing != style.letter_spacing ||
                           default_style_.height != style.height ||
                           text_direction_ != dir ||
                           max_lines_ != maxLines ||
                           soft_wrap_ != softWrap);

    text_data_ = data;
    default_style_ = style;
    text_align_ = align;
    text_direction_ = dir;
    overflow_ = overflow;
    max_lines_ = maxLines;
    soft_wrap_ = softWrap;

    span_ = std::make_shared<TextSpan>(text_data_, default_style_);
    rebuildParagraph();

    if (layout_changed) {
        ANUNodeMarkDirty(anuNode());
        markNeedsLayout();
    } else {
        markNeedsPaint();
    }
}

void RenderParagraph::setTextSpan(std::shared_ptr<InlineSpan> span) {
    if (span_ != span) {
        if (impl_ && impl_->hovered_span) {
            if (impl_->hovered_span->on_hover) {
                impl_->hovered_span->on_hover(false);
            }
            impl_->hovered_span = nullptr;
        }
        span_ = std::move(span);
        rebuildParagraph();
        ANUNodeMarkDirty(anuNode());
        markNeedsLayout();
    }
}

void RenderParagraph::setDefaultStyle(TextStyle style) {
    if (default_style_ != style) {
        default_style_ = std::move(style);
        rebuildParagraph();
        ANUNodeMarkDirty(anuNode());
        markNeedsLayout();
    }
}

void RenderParagraph::setTextAlign(TextAlign align) {
    if (text_align_ != align) {
        text_align_ = align;
        rebuildParagraph();
        markNeedsPaint();
    }
}

void RenderParagraph::setTextDirection(TextDirection dir) {
    if (text_direction_ != dir) {
        text_direction_ = dir;
        rebuildParagraph();
        ANUNodeMarkDirty(anuNode());
        markNeedsLayout();
    }
}

void RenderParagraph::setOverflow(TextOverflow overflow) {
    if (overflow_ != overflow) {
        overflow_ = overflow;
        rebuildParagraph();
        markNeedsPaint();
    }
}

void RenderParagraph::setMaxLines(std::optional<size_t> maxLines) {
    if (max_lines_ != maxLines) {
        max_lines_ = maxLines;
        rebuildParagraph();
        ANUNodeMarkDirty(anuNode());
        markNeedsLayout();
    }
}

void RenderParagraph::setSoftWrap(bool softWrap) {
    if (soft_wrap_ != softWrap) {
        soft_wrap_ = softWrap;
        rebuildParagraph();
        ANUNodeMarkDirty(anuNode());
        markNeedsLayout();
    }
}

void RenderParagraph::setSelectable(bool selectable) {
    if (selectable_ != selectable) {
        selectable_ = selectable;
        if (!selectable_) {
            clearSelection();
        }
        markNeedsPaint();
    }
}

void RenderParagraph::setSelectionColor(Color color) {
    if (selection_color_ != color) {
        selection_color_ = color;
        if (!selection_.isCollapsed()) {
            markNeedsPaint();
        }
    }
}

void RenderParagraph::setOnSelectionChanged(std::function<void(TextSelection)> callback) {
    on_selection_changed_ = std::move(callback);
}

void RenderParagraph::selectAll() {
    if (!impl_ || !impl_->paragraph) return;
    const std::string& all_text = !text_data_.empty() ? text_data_ : impl_->full_text;
    selection_ = TextSelection{0, all_text.length()};
    if (on_selection_changed_) {
        on_selection_changed_(selection_);
    }
    markNeedsPaint();
}

void RenderParagraph::clearSelection() {
    if (selection_.isValid() && !selection_.isCollapsed()) {
        selection_ = TextSelection::empty();
        if (on_selection_changed_) {
            on_selection_changed_(selection_);
        }
        markNeedsPaint();
    }
}

std::string RenderParagraph::getSelectedText() const {
    const std::string& all_text = !text_data_.empty() ? text_data_ : (impl_ ? impl_->full_text : "");
    if (!selection_.isValid() || selection_.isCollapsed() || all_text.empty()) {
        return "";
    }
    size_t s = selection_.start();
    size_t e = std::min(selection_.end(), all_text.length());
    if (s >= e) return "";
    return all_text.substr(s, e - s);
}

void RenderParagraph::copyToClipboard() {
    std::string sel = getSelectedText();
    if (!sel.empty()) {
        if (auto* p = Platform::instance()) {
            p->setClipboardText(sel);
        }
    }
}

void RenderParagraph::rebuildParagraph() {
    impl_->build(span_.get(), default_style_, text_align_, text_direction_, overflow_, max_lines_, soft_wrap_);
}

void RenderParagraph::layoutParagraph(float availableWidth) {
    impl_->layout(availableWidth);
}

TextParagraph* RenderParagraph::getParagraph() const {
    return impl_ ? impl_->paragraph.get() : nullptr;
}

ANUSize RenderParagraph::measureText(ANUNodeConstRef node,
                                     float width,
                                     ANUMeasureMode widthMode,
                                     float height,
                                     ANUMeasureMode heightMode) {
    auto* self = static_cast<RenderParagraph*>(ANUNodeGetContext(node));
    if (!self || !self->impl_ || !self->impl_->paragraph) {
        return {0.0f, 0.0f};
    }

    float constraintWidth = std::numeric_limits<float>::max();
    if (widthMode == ANUMeasureModeExactly || widthMode == ANUMeasureModeAtMost) {
        constraintWidth = width;
    }

    if (!self->soft_wrap_) {
        constraintWidth = std::numeric_limits<float>::max();
    }

    self->layoutParagraph(constraintWidth);

    float maxIntrinsicWidth = self->impl_->paragraph->getMaxIntrinsicWidth();
    float longestLineWidth = self->impl_->paragraph->getLongestLine();
    float measuredHeight = self->impl_->paragraph->getHeight();

    float contentWidth = (constraintWidth < maxIntrinsicWidth) ? longestLineWidth : maxIntrinsicWidth;

    float measuredWidth = contentWidth;
    if (widthMode == ANUMeasureModeExactly) {
        measuredWidth = width;
    } else if (widthMode == ANUMeasureModeAtMost) {
        measuredWidth = std::min(contentWidth, width);
    }

    if (heightMode == ANUMeasureModeExactly) {
        measuredHeight = height;
    } else if (heightMode == ANUMeasureModeAtMost) {
        measuredHeight = std::min(measuredHeight, height);
    }

    return {measuredWidth, measuredHeight};
}

void RenderParagraph::paint(PaintContext& ctx) {
    if (!impl_ || !impl_->paragraph) return;

    // Layout paragraph to the exact allocated box width
    layoutParagraph(size_.width);

    // Draw selection background if selectable and active
    if (selectable_ && selection_.isValid() && !selection_.isCollapsed()) {
        size_t s = selection_.start();
        size_t e = selection_.end();
        auto rects = impl_->paragraph->getRectsForRange(s, e);

        Paint sel_paint;
        sel_paint.setColor(selection_color_);

        for (const auto& box : rects) {
            Rect r = Rect::fromLTWH(
                ctx.offset.x + box.x,
                ctx.offset.y + box.y,
                box.width,
                box.height
            );
            ctx.canvas.drawRRect(r, BorderRadius::circular(3.0f), sel_paint);
        }
    }

    impl_->paragraph->paint(ctx.canvas, ctx.offset.x, ctx.offset.y);
}

const TextSpan* findSpanAtPosition(const std::vector<ParagraphBuilderContext::SpanRange>& spans, size_t position) {
    for (const auto& range : spans) {
        if (position >= range.start && position < range.end) {
            return range.span;
        }
    }
    return nullptr;
}

bool RenderParagraph::hitTestSelf(Point localPoint) const {
    if (localPoint.x >= 0 && localPoint.x <= size_.width &&
        localPoint.y >= 0 && localPoint.y <= size_.height) {
        if (selectable_) return true;
        if (impl_ && impl_->paragraph && !impl_->interactive_spans.empty()) {
            size_t pos = impl_->paragraph->getGlyphIndexAtCoordinate(localPoint.x, localPoint.y);
            const TextSpan* span = findSpanAtPosition(impl_->interactive_spans, pos);
            return span != nullptr;
        }
    }
    return false;
}

void RenderParagraph::handlePointerDown(const PointerEvent& e) {
    if (selectable_ && impl_ && impl_->paragraph) {
        layoutParagraph(size_.width);
        size_t char_pos = impl_->paragraph->getGlyphIndexAtCoordinate(e.localPosition.x, e.localPosition.y);

        auto now = std::chrono::steady_clock::now();
        float dist = std::hypot(e.localPosition.x - last_click_pos_.x, e.localPosition.y - last_click_pos_.y);
        auto dt_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_click_time_).count();

        if (dt_ms < 350 && dist < 10.0f) {
            click_count_++;
        } else {
            click_count_ = 1;
        }
        last_click_pos_ = e.localPosition;
        last_click_time_ = now;

        if (click_count_ == 2) {
            auto [start, end] = impl_->paragraph->getWordBoundary(char_pos);
            selection_ = TextSelection{start, end};
            selection_anchor_ = start;
            is_dragging_ = true;
        } else if (click_count_ >= 3) {
            selectAll();
            is_dragging_ = false;
        } else {
            selection_anchor_ = char_pos;
            selection_ = TextSelection::collapsed(char_pos);
            is_dragging_ = true;
        }

        if (on_selection_changed_) {
            on_selection_changed_(selection_);
        }
        markNeedsPaint();
    }
}

void RenderParagraph::handlePointerUp(const PointerEvent& e) {
    if (selectable_) {
        is_dragging_ = false;
    }
    if (!impl_ || !impl_->paragraph) return;
    size_t pos = impl_->paragraph->getGlyphIndexAtCoordinate(e.localPosition.x, e.localPosition.y);
    const TextSpan* span = findSpanAtPosition(impl_->interactive_spans, pos);
    if (span && span->on_click) {
        span->on_click();
    }
}

void RenderParagraph::handlePointerMove(const PointerEvent& e) {
    if (!impl_ || !impl_->paragraph) return;

    if (selectable_ && is_dragging_) {
        layoutParagraph(size_.width);
        size_t char_pos = impl_->paragraph->getGlyphIndexAtCoordinate(e.localPosition.x, e.localPosition.y);

        TextSelection new_sel{selection_anchor_, char_pos};
        if (new_sel != selection_) {
            selection_ = new_sel;
            if (on_selection_changed_) {
                on_selection_changed_(selection_);
            }
            markNeedsPaint();
        }
    }

    bool is_in_bounds = (e.localPosition.x >= 0 && e.localPosition.x <= size_.width &&
                         e.localPosition.y >= 0 && e.localPosition.y <= size_.height);
    
    const TextSpan* new_hovered = nullptr;
    if (is_in_bounds && !impl_->interactive_spans.empty()) {
        size_t pos = impl_->paragraph->getGlyphIndexAtCoordinate(e.localPosition.x, e.localPosition.y);
        new_hovered = findSpanAtPosition(impl_->interactive_spans, pos);
    }
    
    if (impl_->hovered_span != new_hovered) {
        if (impl_->hovered_span && impl_->hovered_span->on_hover) {
            impl_->hovered_span->on_hover(false);
        }
        impl_->hovered_span = new_hovered;
        if (impl_->hovered_span && impl_->hovered_span->on_hover) {
            impl_->hovered_span->on_hover(true);
        }
    }
}

void RenderParagraph::handlePointerExit(const PointerEvent& e) {
    if (impl_ && impl_->hovered_span) {
        if (impl_->hovered_span->on_hover) {
            impl_->hovered_span->on_hover(false);
        }
        impl_->hovered_span = nullptr;
    }
}

SystemCursor RenderParagraph::cursor() const {
    if (impl_ && impl_->hovered_span && (impl_->hovered_span->on_click || impl_->hovered_span->on_hover)) {
        return SystemCursor::Pointer;
    }
    if (selectable_) {
        return SystemCursor::Text;
    }
    return SystemCursor::Default;
}

// ════════════════════════════════════════════════════════════════
// Text Widget Implementation
// ════════════════════════════════════════════════════════════════

std::unique_ptr<RenderObject> Text::createRenderObject(BuildContext&) {
    auto span = std::make_shared<TextSpan>(data, style);
    auto rp = std::make_unique<RenderParagraph>(
        span,
        style,
        text_align,
        text_direction,
        overflow,
        max_lines,
        soft_wrap
    );
    rp->setText(data, style, text_align, text_direction, overflow, max_lines, soft_wrap);
    rp->setSelectable(selectable);
    rp->setSelectionColor(selection_color);
    rp->setOnSelectionChanged(on_selection_changed);
    return rp;
}

void Text::updateRenderObject(BuildContext&, RenderObject& renderObject) {
    if (auto* rp = dynamic_cast<RenderParagraph*>(&renderObject)) {
        rp->setText(data, style, text_align, text_direction, overflow, max_lines, soft_wrap);
        rp->setSelectable(selectable);
        rp->setSelectionColor(selection_color);
        rp->setOnSelectionChanged(on_selection_changed);
    }
}

// ════════════════════════════════════════════════════════════════
// RichText Widget Implementation
// ════════════════════════════════════════════════════════════════

std::unique_ptr<RenderObject> RichText::createRenderObject(BuildContext&) {
    auto rp = std::make_unique<RenderParagraph>(
        text_span,
        default_style,
        text_align,
        text_direction,
        overflow,
        max_lines,
        soft_wrap
    );
    rp->setSelectable(selectable);
    rp->setSelectionColor(selection_color);
    rp->setOnSelectionChanged(on_selection_changed);
    return rp;
}

void RichText::updateRenderObject(BuildContext&, RenderObject& renderObject) {
    if (auto* rp = dynamic_cast<RenderParagraph*>(&renderObject)) {
        rp->setTextSpan(text_span);
        rp->setDefaultStyle(default_style);
        rp->setTextAlign(text_align);
        rp->setTextDirection(text_direction);
        rp->setOverflow(overflow);
        rp->setMaxLines(max_lines);
        rp->setSoftWrap(soft_wrap);
        rp->setSelectable(selectable);
        rp->setSelectionColor(selection_color);
        rp->setOnSelectionChanged(on_selection_changed);
    }
}

} // namespace enki
