#pragma once
/// @file canvas_painter.hpp
/// @brief Lightweight pure-enki::Canvas RenderBox widget helper for internal widgets.

#include "enki/core/types.hpp"
#include "enki/rendering/canvas.hpp"
#include "enki/tree/render_object.hpp"
#include "enki/tree/widget.hpp"
#include <layout_engine/Anu.h>

#include <functional>
#include <memory>
#include <optional>
#include <utility>

namespace enki::detail {

struct CanvasPainterProps {
    std::function<void(Canvas&, Size)> painter;
    std::optional<StyleValue> width;
    std::optional<StyleValue> height;
    Key key = Key::none();
};

class RenderCanvasPainter : public RenderBox {
public:
    RenderCanvasPainter(std::function<void(Canvas&, Size)> painter,
                        std::optional<StyleValue> width,
                        std::optional<StyleValue> height)
        : painter_(std::move(painter)), width_(width), height_(height) {
        applySize();
    }

    void update(std::function<void(Canvas&, Size)> painter,
                std::optional<StyleValue> width,
                std::optional<StyleValue> height) {
        painter_ = std::move(painter);
        bool layout_changed = (width_ != width || height_ != height);
        width_ = width;
        height_ = height;
        if (layout_changed) {
            applySize();
            markNeedsLayout();
        }
        markNeedsPaint();
    }

    void paint(PaintContext& context) override {
        if (!painter_ || size_.width <= 0.0f || size_.height <= 0.0f) return;
        context.canvas.save();
        context.canvas.translate(context.offset.x, context.offset.y);
        painter_(context.canvas, size_);
        context.canvas.restore();
    }

private:
    void applySize() {
        if (!anu_node_) return;
        if (width_.has_value()) {
            if (width_->isPercent()) ANUNodeStyleSetWidthPercent(anu_node_, width_->value);
            else if (width_->isAuto()) ANUNodeStyleSetWidthAuto(anu_node_);
            else ANUNodeStyleSetWidth(anu_node_, width_->value);
        } else {
            ANUNodeStyleSetWidthAuto(anu_node_);
        }
        if (height_.has_value()) {
            if (height_->isPercent()) ANUNodeStyleSetHeightPercent(anu_node_, height_->value);
            else if (height_->isAuto()) ANUNodeStyleSetHeightAuto(anu_node_);
            else ANUNodeStyleSetHeight(anu_node_, height_->value);
        } else {
            ANUNodeStyleSetHeightAuto(anu_node_);
        }
    }

    std::function<void(Canvas&, Size)> painter_;
    std::optional<StyleValue> width_;
    std::optional<StyleValue> height_;
};

class CanvasPainterWidget : public SingleChildRenderObjectWidget {
public:
    CanvasPainterProps props;

    explicit CanvasPainterWidget(CanvasPainterProps p)
        : SingleChildRenderObjectWidget(p.key), props(std::move(p)) {}

    [[nodiscard]] std::string_view typeName() const override { return "CanvasPainter"; }

    [[nodiscard]] std::unique_ptr<RenderObject> createRenderObject(BuildContext&) override {
        return std::make_unique<RenderCanvasPainter>(props.painter, props.width, props.height);
    }

    void updateRenderObject(BuildContext&, RenderObject& renderObject) override {
        static_cast<RenderCanvasPainter&>(renderObject).update(props.painter, props.width, props.height);
    }
};

inline WidgetPtr canvasPainter(CanvasPainterProps props) {
    return std::make_shared<CanvasPainterWidget>(std::move(props));
}

} // namespace enki::detail

