/// @file timeline.cpp
/// @brief Implementation of Advanced Timeline widget.

#include "enki/widgets/timeline.hpp"
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/flexbox.hpp"
#include "enki/widgets/text.hpp"
#include "enki/platform/platform.hpp"
#include "enki/state/state.hpp"
#include "enki/tree/element.hpp"
#include "enki/tree/build_context.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/app/app.hpp"

#include "enki/rendering/paint.hpp"
#include "enki/rendering/path.hpp"
#include "enki/rendering/paragraph.hpp"

#include <algorithm>
#include <iostream>
#include <cmath>

namespace enki {

static std::unique_ptr<TextParagraph> buildTimelineParagraph(std::string_view text,
                                                             float font_size,
                                                             Color color,
                                                             float layout_w,
                                                             ParagraphAlign align = ParagraphAlign::Start) {
    ParagraphStyle p_style;
    p_style.text_align = align;
    auto builder = TextParagraphBuilder::make(p_style);
    if (!builder) return nullptr;
    ParagraphTextStyle t_style;
    t_style.font_size = font_size;
    t_style.color = color;
    builder->pushStyle(t_style);
    builder->addText(text);
    auto para = builder->build();
    if (para) {
        para->layout(layout_w);
    }
    return para;
}

// ════════════════════════════════════════════════════════════════
// Custom RenderBox for Timeline View
// ════════════════════════════════════════════════════════════════

class RenderTimelineBox : public RenderBox {
public:
    std::shared_ptr<TimelineController> controller;
    TimelineProps options;
    int hovered_item_index = -1;

    struct ItemLayoutInfo {
        Rect card_rect;
        Point node_center;
        
        float card_w = -1.0f;
        std::string title;
        std::string desc;
        std::string time;
        std::string details;
        std::string badge;
        std::string icon;
        bool is_expanded = false;
        Color title_color = 0;
        Color desc_color = 0;
        Color time_color = 0;

        std::unique_ptr<TextParagraph> p_title;
        std::unique_ptr<TextParagraph> p_time;
        std::unique_ptr<TextParagraph> p_desc;
        std::unique_ptr<TextParagraph> p_det;
        std::unique_ptr<TextParagraph> p_badge;
        std::unique_ptr<TextParagraph> p_icon;
        std::unique_ptr<TextParagraph> p_num;
    };
    mutable std::vector<ItemLayoutInfo> layout_cache_;

    RenderTimelineBox(std::shared_ptr<TimelineController> ctrl, TimelineProps opt)
        : controller(std::move(ctrl)), options(std::move(opt)) {
        updateFlexboxStyle();
    }

    Color getStatusColor(TimelineItemStatus status, Color custom_color) const {
        if (custom_color != 0) return custom_color;
        switch (status) {
            case TimelineItemStatus::Completed: return options.completed_color;
            case TimelineItemStatus::Active:    return options.active_color;
            case TimelineItemStatus::Pending:   return options.pending_color;
            case TimelineItemStatus::Warning:   return options.warning_color;
            case TimelineItemStatus::Failed:    return options.failed_color;
        }
        return options.pending_color;
    }

    float computeItemHeight(const TimelineItem& item, float card_w, size_t index) const {
        if (layout_cache_.size() <= index) {
            layout_cache_.resize(index + 1);
        }
        auto& cache = layout_cache_[index];

        bool dirty = cache.card_w != card_w || cache.title != item.title ||
                     cache.desc != item.description || cache.time != item.timestamp ||
                     cache.details != item.details || cache.badge != item.badge_text ||
                     cache.icon != item.icon || cache.is_expanded != item.is_expanded ||
                     cache.title_color != options.title_color ||
                     cache.desc_color != options.desc_color ||
                     cache.time_color != options.timestamp_color;

        if (dirty) {
            cache.card_w = card_w;
            cache.title = item.title;
            cache.desc = item.description;
            cache.time = item.timestamp;
            cache.details = item.details;
            cache.badge = item.badge_text;
            cache.icon = item.icon;
            cache.is_expanded = item.is_expanded;
            cache.title_color = options.title_color;
            cache.desc_color = options.desc_color;
            cache.time_color = options.timestamp_color;

            // Badge
            if (!item.badge_text.empty()) {
                cache.p_badge = buildTimelineParagraph(
                    item.badge_text, 10.5f, item.badge_fg != 0 ? item.badge_fg : 0xFFFFFFFF, 100.0f);
            } else {
                cache.p_badge = nullptr;
            }
            float badge_w = cache.p_badge ? cache.p_badge->getMaxIntrinsicWidth() + 14.0f : 0.0f;

            // Title
            if (!item.title.empty()) {
                cache.p_title = buildTimelineParagraph(
                    item.title, 13.0f, options.title_color, std::max(40.0f, card_w - 24.0f - badge_w));
            } else {
                cache.p_title = nullptr;
            }

            // Time
            if (!item.timestamp.empty()) {
                cache.p_time = buildTimelineParagraph(
                    item.timestamp, 11.0f, options.timestamp_color, card_w - 24.0f);
            } else {
                cache.p_time = nullptr;
            }

            // Desc
            if (!item.description.empty()) {
                cache.p_desc = buildTimelineParagraph(
                    item.description, 12.0f, options.desc_color, card_w - 24.0f);
            } else {
                cache.p_desc = nullptr;
            }

            // Details
            if (item.is_expanded && !item.details.empty()) {
                cache.p_det = buildTimelineParagraph(
                    item.details, 11.5f, 0xFF94A3B8, std::max(40.0f, card_w - 40.0f));
            } else {
                cache.p_det = nullptr;
            }
            
            // Icon
            if (!item.icon.empty()) {
                cache.p_icon = buildTimelineParagraph(
                    item.icon, options.node_size * 0.55f, 0xFFFFFFFF, options.node_size, ParagraphAlign::Center);
            } else {
                cache.p_icon = nullptr;
            }

            // Number
            cache.p_num = nullptr;
            if (item.icon.empty() && (options.is_stepper || item.node_shape == TimelineNodeShape::Number)) {
                std::string num_str = std::to_string(index + 1);
                Color num_col = item.status == TimelineItemStatus::Pending ? options.pending_color : 0xFF0F172A;
                cache.p_num = buildTimelineParagraph(
                    num_str, 11.0f, num_col, options.node_size, ParagraphAlign::Center);
            }
        }

        float h = 12.0f; // top padding
        if (cache.p_title) h += cache.p_title->getHeight() + 4.0f;
        if (cache.p_time) h += cache.p_time->getHeight() + 4.0f;
        if (cache.p_desc) h += cache.p_desc->getHeight() + 6.0f;
        if (cache.p_det) h += cache.p_det->getHeight() + 20.0f;
        h += 12.0f; // bottom padding
        
        return std::max(h, options.node_size + 16.0f);
    }

    float computeTotalHeight() const {
        const auto& items = controller->getItems();
        if (items.empty()) return 60.0f;

        if (layout_cache_.size() != items.size()) {
            layout_cache_.resize(items.size());
        }

        if (options.orientation == TimelineOrientation::Horizontal) {
            float max_card_h = 100.0f;
            for (size_t i = 0; i < items.size(); ++i) {
                max_card_h = std::max(max_card_h, computeItemHeight(items[i], options.card_width, i));
            }
            return options.node_size + 20.0f + max_card_h + 20.0f;
        }

        float total_h = 0.0f;
        float card_w = (options.alignment == TimelineAlignment::Alternate) ? options.card_width : 480.0f;

        for (size_t i = 0; i < items.size(); ++i) {
            float item_h = computeItemHeight(items[i], card_w, i);
            total_h += item_h;
            if (i + 1 < items.size()) total_h += options.item_spacing;
        }

        return total_h + 20.0f;
    }

    void updateFlexboxStyle() {
        FlexboxStyle st;
        st.height = StyleValue::point(computeTotalHeight());
        st.width = StyleValue::percent(100.0f);
        applyFlexboxStyle(anuNode(), st);
    }

    void paint(PaintContext& ctx) override {
        auto& canvas = ctx.canvas;
        if (size_.width <= 0 || size_.height <= 0) return;

        const auto& items = controller->getItems();
        if (items.empty()) return;

        if (layout_cache_.size() != items.size()) {
            layout_cache_.resize(items.size());
        }

        if (options.orientation == TimelineOrientation::Horizontal) {
            paintHorizontal(canvas, ctx.offset);
        } else if (options.alignment == TimelineAlignment::Alternate) {
            paintVerticalAlternate(canvas, ctx.offset);
        } else {
            paintVerticalStandard(canvas, ctx.offset);
        }
    }

    void paintHorizontal(Canvas& canvas, Point offset) {
        const auto& items = controller->getItems();
        size_t n = items.size();
        float step_x = size_.width / static_cast<float>(n);
        float line_y = offset.y + options.node_size * 0.8f + 10.0f;

        Paint line_paint;
        line_paint.setAntiAlias(true);
        line_paint.setStrokeWidth(options.line_thickness);

        // 1. Connecting Track Lines
        for (size_t i = 0; i + 1 < n; ++i) {
            float x1 = offset.x + (static_cast<float>(i) + 0.5f) * step_x + options.node_size * 0.5f;
            float x2 = offset.x + (static_cast<float>(i + 1) + 0.5f) * step_x - options.node_size * 0.5f;

            line_paint.setColor(items[i].status == TimelineItemStatus::Completed ? options.completed_color : options.line_color);
            if (options.line_style == TimelineLineStyle::Dashed) {
                canvas.drawDashedLine(Point{x1, line_y}, Point{x2, line_y}, line_paint, 6.0f, 4.0f);
            } else {
                canvas.drawLine(Point{x1, line_y}, Point{x2, line_y}, line_paint);
            }
        }

        // 2. Nodes & Cards
        for (size_t i = 0; i < n; ++i) {
            const auto& item = items[i];
            float cx = offset.x + (static_cast<float>(i) + 0.5f) * step_x;
            float cy = line_y;

            layout_cache_[i].node_center = Point{cx, cy};

            paintNode(canvas, cx, cy, item, i);

            // Content Card Below Node
            float card_w = std::min(step_x - 12.0f, options.card_width);
            float card_x = cx - card_w * 0.5f;
            float card_y = cy + options.node_size * 0.5f + 12.0f;
            float card_h = computeItemHeight(item, card_w, i);

            layout_cache_[i].card_rect = Rect{card_x, card_y, card_w, card_h};
            paintCard(canvas, card_x, card_y, card_w, card_h, item, (hovered_item_index == static_cast<int>(i)), i);
        }
    }

    void paintVerticalStandard(Canvas& canvas, Point offset) {
        const auto& items = controller->getItems();
        size_t n = items.size();

        float node_x = offset.x + options.node_size * 0.5f + 16.0f;
        float cur_y = offset.y + 10.0f;
        float card_x = node_x + options.node_size * 0.5f + 16.0f;
        float card_w = std::max(200.0f, size_.width - card_x - 16.0f);

        // Precompute centers
        for (size_t i = 0; i < n; ++i) {
            float card_h = computeItemHeight(items[i], card_w, i);
            float node_y = cur_y + options.node_size * 0.5f + 6.0f;

            layout_cache_[i].node_center = Point{node_x, node_y};
            layout_cache_[i].card_rect = Rect{card_x, cur_y, card_w, card_h};

            cur_y += card_h + options.item_spacing;
        }

        Paint line_paint;
        line_paint.setAntiAlias(true);
        line_paint.setStrokeWidth(options.line_thickness);

        // 1. Draw Connecting Track Lines
        for (size_t i = 0; i + 1 < n; ++i) {
            float y1 = layout_cache_[i].node_center.y + options.node_size * 0.5f;
            float y2 = layout_cache_[i + 1].node_center.y - options.node_size * 0.5f;

            line_paint.setColor(items[i].status == TimelineItemStatus::Completed ? options.completed_color : options.line_color);
            if (options.line_style == TimelineLineStyle::Dashed) {
                canvas.drawDashedLine(Point{node_x, y1}, Point{node_x, y2}, line_paint, 6.0f, 4.0f);
            } else {
                canvas.drawLine(Point{node_x, y1}, Point{node_x, y2}, line_paint);
            }
        }

        // 2. Draw Nodes & Cards
        for (size_t i = 0; i < n; ++i) {
            const auto& item = items[i];
            const auto& cache = layout_cache_[i];

            paintNode(canvas, cache.node_center.x, cache.node_center.y, item, i);
            paintCard(canvas, cache.card_rect.x, cache.card_rect.y, cache.card_rect.width, cache.card_rect.height,
                      item, (hovered_item_index == static_cast<int>(i)), i);
        }
    }

    void paintVerticalAlternate(Canvas& canvas, Point offset) {
        const auto& items = controller->getItems();
        size_t n = items.size();

        float center_x = offset.x + size_.width * 0.5f;
        float cur_y = offset.y + 10.0f;
        float card_w = std::min(options.card_width, size_.width * 0.45f);

        for (size_t i = 0; i < n; ++i) {
            float card_h = computeItemHeight(items[i], card_w, i);
            float node_y = cur_y + options.node_size * 0.5f + 6.0f;

            bool is_left = (i % 2 == 0);
            float card_x = is_left ? (center_x - options.node_size * 0.5f - 16.0f - card_w)
                                   : (center_x + options.node_size * 0.5f + 16.0f);

            layout_cache_[i].node_center = Point{center_x, node_y};
            layout_cache_[i].card_rect = Rect{card_x, cur_y, card_w, card_h};

            cur_y += card_h + options.item_spacing;
        }

        Paint line_paint;
        line_paint.setAntiAlias(true);
        line_paint.setStrokeWidth(options.line_thickness);

        // 1. Track line
        for (size_t i = 0; i + 1 < n; ++i) {
            float y1 = layout_cache_[i].node_center.y + options.node_size * 0.5f;
            float y2 = layout_cache_[i + 1].node_center.y - options.node_size * 0.5f;

            line_paint.setColor(items[i].status == TimelineItemStatus::Completed ? options.completed_color : options.line_color);
            if (options.line_style == TimelineLineStyle::Dashed) {
                canvas.drawDashedLine(Point{center_x, y1}, Point{center_x, y2}, line_paint, 6.0f, 4.0f);
            } else {
                canvas.drawLine(Point{center_x, y1}, Point{center_x, y2}, line_paint);
            }
        }

        // 2. Nodes & Cards
        for (size_t i = 0; i < n; ++i) {
            const auto& item = items[i];
            const auto& cache = layout_cache_[i];

            paintNode(canvas, cache.node_center.x, cache.node_center.y, item, i);
            paintCard(canvas, cache.card_rect.x, cache.card_rect.y, cache.card_rect.width, cache.card_rect.height,
                      item, (hovered_item_index == static_cast<int>(i)), i);
        }
    }

    void paintNode(Canvas& canvas, float cx, float cy, const TimelineItem& item, size_t index) {
        float r = options.node_size * 0.5f;
        Color node_col = getStatusColor(item.status, item.custom_node_color);

        // Active Glowing Outer Ring
        if (item.status == TimelineItemStatus::Active) {
            Paint glow_paint;
            glow_paint.setAntiAlias(true);
            glow_paint.setColor(0x3338BDF8);
            canvas.drawCircle(Point{cx, cy}, r + 5.0f, glow_paint);
        }

        // Base Circle / Square
        Paint node_paint;
        node_paint.setAntiAlias(true);
        node_paint.setColor(node_col);

        if (item.node_shape == TimelineNodeShape::Square) {
            canvas.drawRRect(Rect{cx - r, cy - r, options.node_size, options.node_size}, BorderRadius::all(4.0f), node_paint);
        } else {
            if (item.status == TimelineItemStatus::Pending) {
                node_paint.setStyle(PaintStyle::Stroke);
                node_paint.setStrokeWidth(2.0f);
                canvas.drawCircle(Point{cx, cy}, r - 1.0f, node_paint);
            } else {
                canvas.drawCircle(Point{cx, cy}, r, node_paint);
            }
        }

        // Glyphs & Content inside Node
        if (item.status == TimelineItemStatus::Completed) {
            // Emerald checkmark ✓
            Paint chk_paint;
            chk_paint.setAntiAlias(true);
            chk_paint.setColor(0xFFFFFFFF);
            chk_paint.setStyle(PaintStyle::Stroke);
            chk_paint.setStrokeWidth(2.0f);
            Path p;
            p.moveTo(cx - 4.5f, cy);
            p.lineTo(cx - 1.0f, cy + 3.5f);
            p.lineTo(cx + 4.5f, cy - 3.0f);
            canvas.drawPath(p, chk_paint);
        } else if (item.status == TimelineItemStatus::Failed) {
            // Red cross ✕
            Paint x_paint;
            x_paint.setAntiAlias(true);
            x_paint.setColor(0xFFFFFFFF);
            x_paint.setStrokeWidth(2.0f);
            float sz = 3.5f;
            canvas.drawLine(Point{cx - sz, cy - sz}, Point{cx + sz, cy + sz}, x_paint);
            canvas.drawLine(Point{cx + sz, cy - sz}, Point{cx - sz, cy + sz}, x_paint);
        } else {
            const auto& cache = layout_cache_[index];
            if (!item.icon.empty() && cache.p_icon) {
                cache.p_icon->paint(canvas, cx - options.node_size * 0.5f, cy - cache.p_icon->getHeight() * 0.5f);
            } else if (cache.p_num) {
                cache.p_num->paint(canvas, cx - options.node_size * 0.5f, cy - cache.p_num->getHeight() * 0.5f);
            }
        }
    }

    void paintCard(Canvas& canvas, float x, float y, float w, float h, const TimelineItem& item,
                   bool is_hov, size_t index) {
        Rect rrect{x, y, w, h};
        BorderRadius rad = BorderRadius::all(options.card_border_radius);

        // Card Background
        Paint bg_paint;
        bg_paint.setAntiAlias(true);
        bg_paint.setColor(is_hov ? 0xFF24324D : options.card_bg_color);
        canvas.drawRRect(rrect, rad, bg_paint);

        // Card Border
        Paint b_paint;
        b_paint.setAntiAlias(true);
        b_paint.setStyle(PaintStyle::Stroke);
        b_paint.setStrokeWidth(1.0f);
        b_paint.setColor(is_hov ? 0xFF475569 : options.card_border_color);
        canvas.drawRRect(rrect, rad, b_paint);

        float cur_y = y + 12.0f;
        float pad_x = x + 12.0f;
        const auto& cache = layout_cache_[index];

        // 1. Title Row
        if (cache.p_title) {
            cache.p_title->paint(canvas, pad_x, cur_y);
            
            // Paint Badge on the right
            if (cache.p_badge) {
                float bw = cache.p_badge->getMaxIntrinsicWidth() + 10.0f;
                float bh = cache.p_badge->getHeight() + 2.0f;
                float bx = x + w - 12.0f - bw;
                float by = cur_y;

                Rect bd_rect{bx, by, bw, bh};
                Paint bd_paint;
                bd_paint.setColor(item.badge_bg != 0 ? item.badge_bg : 0x2E38BDF8);
                canvas.drawRRect(bd_rect, BorderRadius::all(3.0f), bd_paint);
                cache.p_badge->paint(canvas, bx + 5.0f, by + 1.0f);
            }

            cur_y += cache.p_title->getHeight() + 4.0f;
        }

        // 2. Timestamp
        if (cache.p_time) {
            cache.p_time->paint(canvas, pad_x, cur_y);
            cur_y += cache.p_time->getHeight() + 4.0f;
        }

        // 3. Description
        if (cache.p_desc) {
            cache.p_desc->paint(canvas, pad_x, cur_y);
            cur_y += cache.p_desc->getHeight() + 6.0f;
        }

        // 4. Expandable Details Box
        if (cache.p_det) {
            float det_h = cache.p_det->getHeight() + 12.0f;
            float avail_w = w - 24.0f;
            Rect det_box{pad_x, cur_y, avail_w, det_h};
            Paint det_bg;
            det_bg.setColor(options.details_bg_color);
            canvas.drawRRect(det_box, BorderRadius::all(4.0f), det_bg);

            cache.p_det->paint(canvas, pad_x + 8.0f, cur_y + 6.0f);
        }
    }

    [[nodiscard]] bool hitTestSelf(Point localPoint) const override {
        return localPoint.x >= 0 && localPoint.x <= size_.width &&
               localPoint.y >= 0 && localPoint.y <= size_.height;
    }

    int getHitItemIndex(float local_x, float local_y) const {
        for (size_t i = 0; i < layout_cache_.size(); ++i) {
            const auto& cache = layout_cache_[i];
            if (local_x >= cache.card_rect.x && local_x <= cache.card_rect.x + cache.card_rect.width &&
                local_y >= cache.card_rect.y && local_y <= cache.card_rect.y + cache.card_rect.height) {
                return static_cast<int>(i);
            }
            float dx = local_x - cache.node_center.x;
            float dy = local_y - cache.node_center.y;
            if ((dx * dx + dy * dy) <= (options.node_size * options.node_size)) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }
};

static RenderTimelineBox* findTimelineBox(RenderObject* ro) {
    if (!ro) return nullptr;
    if (auto* box = dynamic_cast<RenderTimelineBox*>(ro)) return box;
    for (auto* child : ro->children()) {
        if (auto* found = findTimelineBox(child)) return found;
    }
    return nullptr;
}

class RenderTimelineWidget : public SingleChildRenderObjectWidget {
public:
    std::shared_ptr<TimelineController> controller;
    TimelineProps options;
    int hovered_item_index;

    RenderTimelineWidget(std::shared_ptr<TimelineController> ctrl, TimelineProps opt, int h_idx)
        : SingleChildRenderObjectWidget(Key::none()), controller(std::move(ctrl)),
          options(std::move(opt)), hovered_item_index(h_idx) {}

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        auto ro = std::make_unique<RenderTimelineBox>(controller, options);
        ro->hovered_item_index = hovered_item_index;
        return ro;
    }

    void updateRenderObject(BuildContext&, RenderObject& renderObject) override {
        if (auto* ro = dynamic_cast<RenderTimelineBox*>(&renderObject)) {
            ro->controller = controller;
            ro->options = options;
            ro->hovered_item_index = hovered_item_index;
            ro->updateFlexboxStyle();
            ro->markNeedsPaint();
        }
    }

    [[nodiscard]] std::string_view typeName() const override { return "RenderTimelineWidget"; }
};

// ════════════════════════════════════════════════════════════════
// Timeline State Implementation
// ════════════════════════════════════════════════════════════════

class TimelineState : public State {
private:
    std::shared_ptr<TimelineController> controller_;
    int hovered_item_index_ = -1;

public:
    void initState() override {
        State::initState();
        auto* tl = static_cast<const TimelineWidget*>(widget());
        controller_ = tl->props.controller;
    }

    void didUpdateWidget(const Widget& old_widget) override {
        State::didUpdateWidget(old_widget);
        auto* tl = static_cast<const TimelineWidget*>(widget());
        controller_ = tl->props.controller;
    }

    WidgetPtr build(BuildContext&) override {
        auto* tl = static_cast<const TimelineWidget*>(widget());

        auto timeline_render = std::make_shared<RenderTimelineWidget>(
            controller_, tl->props, hovered_item_index_
        );

        return gestureDetector({
            .child = timeline_render,
            .hit_test_behavior = HitTestBehavior::Opaque,
            .cursor_type = (hovered_item_index_ >= 0) ? SystemCursor::Pointer : SystemCursor::Default,
            .on_tap_down = [this, tl](const TapDownDetails& e) {
                if (auto* ro = context().element()->findRenderObject()) {
                    if (auto* box = findTimelineBox(ro)) {
                        int hit = box->getHitItemIndex(e.local_position.x, e.local_position.y);
                        if (hit >= 0 && static_cast<size_t>(hit) < controller_->getItems().size()) {
                            const auto& item = controller_->getItems()[hit];

                            if (tl->props.is_stepper) {
                                controller_->setActiveStep(hit);
                                if (tl->props.on_step_changed) {
                                    tl->props.on_step_changed(hit);
                                }
                            } else {
                                if (!item.details.empty()) {
                                    controller_->toggleExpand(item.id);
                                    if (tl->props.on_item_expanded) {
                                        tl->props.on_item_expanded(item.id, !item.is_expanded);
                                    }
                                }
                            }

                            if (tl->props.on_item_tap) {
                                tl->props.on_item_tap(item);
                            }
                            setState([] {});
                        }
                    }
                }
            },
            .on_hover_exit = [this](const PointerEvent&) {
                setState([this] { hovered_item_index_ = -1; });
            },
            .on_hover_move = [this](const PointerEvent& e) {
                if (auto* ro = context().element()->findRenderObject()) {
                    if (auto* box = findTimelineBox(ro)) {
                        int hit = box->getHitItemIndex(e.localPosition.x, e.localPosition.y);
                        if (hit != hovered_item_index_) {
                            hovered_item_index_ = hit;
                            setState([] {});
                        }
                    }
                }
            },
        });
    }
};

std::unique_ptr<State> TimelineWidget::createState() {
    return std::make_unique<TimelineState>();
}

} // namespace enki
