# Popover

> An interactive floating native card surface anchored to a target widget with an animated pointer arrow tail, supporting rich form controls, buttons, and programmatic controllers.

- **Header File**: `#include "enki/widgets/popover.hpp"`
- **C++ Class**: `enki::Popover` (inherits from `enki::StatefulWidget`)
- **Factory Helper**: `enki::popover(child, popover_builder, options, controller)`
- **Props Struct**: `enki::PopoverProps`
- **Controller**: `enki::PopoverController`
- **Enums**: `enki::PopoverDirection`, `enki::PopoverAlignment`, `enki::PopoverTrigger`

---

## Overview

Unlike a simple `Tooltip` (which is typically transient and informational), a **`Popover`** is designed for rich interactive user workflows. It displays an anchored card with an arrow pointer pointing directly at the anchor widget. Users can interact with buttons, text fields, sliders, and toggles inside the popover without it prematurely dismissing.

---

## C++ API Definition

### Direction & Alignment Enums
```cpp
namespace enki {

enum class PopoverDirection {
    Top,
    Bottom,
    Left,
    Right,
    Auto
};

enum class PopoverAlignment {
    Start,
    Center,
    End
};

enum class PopoverTrigger {
    Click,
    Hover,
    Manual
};

} // namespace enki
```

### Controller (`PopoverController`)
```cpp
namespace enki {

class PopoverController {
public:
    void show();
    void hide();
    void toggle();
    [[nodiscard]] bool isOpen() const;
};

} // namespace enki
```

### Configuration Props (`PopoverProps`)
```cpp
namespace enki {

struct PopoverProps {
    Key              key              = Key::none();
    PopoverDirection direction        = PopoverDirection::Top;
    PopoverAlignment alignment        = PopoverAlignment::Center;
    PopoverTrigger   trigger          = PopoverTrigger::Click;

    Color            background_color = 0xFA1F242C; ///< ARGB translucent slate
    Color            border_color     = 0xFF363B42;
    float            border_width     = 1.0f;
    float            border_radius    = 10.0f;

    float            arrow_size       = 10.0f;     ///< Triangular pointer tail size
    bool             show_arrow       = true;

    float            elevation        = 12.0f;
    Color            shadow_color     = 0x60000000;

    EdgeInsets       padding          = EdgeInsets::all(12.0f);
    Size             content_size     = Size{260.0f, 180.0f};
    bool             auto_dismiss     = true;      ///< Dismiss on click outside
    std::string      custom_shader    = "";        ///< Optional SkSL shader code
};

} // namespace enki
```

### Factory Helper Functions
```cpp
namespace enki {

inline WidgetPtr popover(
    WidgetPtr                               child,
    std::function<WidgetPtr(BuildContext&)> popover_builder,
    PopoverProps                            options    = PopoverProps(),
    std::shared_ptr<PopoverController>      controller = nullptr
);

struct PopoverDeclarativeProps {
    Key                                     key = Key::none();
    WidgetPtr                               child;
    std::function<WidgetPtr(BuildContext&)> popover_builder;
    PopoverProps                            options    = PopoverProps();
    std::shared_ptr<PopoverController>      controller = nullptr;
};

inline WidgetPtr popover(PopoverDeclarativeProps props);

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `key` | `Key` | `Key::none()` | Identifier for widget diffing and state preservation. |
| `child` | `WidgetPtr` | `nullptr` | Target anchor widget. |
| `popover_builder` | `std::function<WidgetPtr(BuildContext&)>` | `nullptr` | Callback returning interactive card contents. |
| `controller` | `std::shared_ptr<PopoverController>` | `nullptr` | Programmatic handle to show, hide, or toggle popover. |
| `options.direction` | `PopoverDirection` | `PopoverDirection::Top` | Preferred side to open (`Top`, `Bottom`, `Left`, `Right`, `Auto`). |
| `options.alignment` | `PopoverAlignment` | `PopoverAlignment::Center` | Alignment along anchor's edge (`Start`, `Center`, `End`). |
| `options.trigger` | `PopoverTrigger` | `PopoverTrigger::Click` | Interaction triggering popover (`Click`, `Hover`, `Manual`). |
| `options.show_arrow` | `bool` | `true` | Draws a triangular indicator arrow pointing to the anchor. |
| `options.arrow_size` | `float` | `10.0f` | Dimension of the arrow pointer in pixels. |
| `options.content_size` | `Size` | `Size{260.0f, 180.0f}` | Dimensions of the floating popup surface. |
| `options.background_color` | `Color` | `0xFA1F242C` | Surface background color. |
| `options.border_color` | `Color` | `0xFF363B42` | Surface outline border color. |
| `options.border_width` | `float` | `1.0f` | Surface outline border thickness. |
| `options.border_radius` | `float` | `10.0f` | Corner rounding radius. |
| `options.elevation` | `float` | `12.0f` | Drop shadow elevation depth. |
| `options.shadow_color` | `Color` | `0x60000000` | Drop shadow tint color. |
| `options.padding` | `EdgeInsets` | `EdgeInsets::all(12.0f)` | Interior padding within popover card. |
| `options.auto_dismiss` | `bool` | `true` | Automatically dismisses when clicking outside surface. |
| `options.custom_shader` | `std::string` | `""` | Optional SkSL shader code applied to popover surface. |

---

## Code Examples (From `widgets_demo/popover_demo/main.cpp`)

### 1. Interactive User Card Popover
```cpp
#include "enki/widgets/popover.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/flexbox.hpp"

using namespace enki;

WidgetPtr buildProfilePopover() {
    auto profileButton = button(text("👤 Alexander Wright"), nullptr);

    PopoverProps options;
    options.direction = PopoverDirection::Bottom;
    options.content_size = Size{260.0f, 160.0f};
    options.border_color = 0xFF38BDF8;

    return popover(profileButton, [](BuildContext& ctx) -> WidgetPtr {
        return column({
            .align_items = Align::Start,
            .gap = 8_px,
            .children = {
                text("Alexander Wright", { .font_weight = FontWeight::Bold, .font_size = 15.0f }),
                text("Lead Architect · Engineering", { .color = 0xFF94A3B8, .font_size = 12.0f }),
                button(text("View Full Profile"), []{ /* Navigate */ }),
            }
        });
    }, options);
}
```

### 2. Programmatic Controller Toggle
```cpp
auto controller = std::make_shared<PopoverController>();

auto controlledPopover = popover(
    myTriggerWidget,
    [](BuildContext&) { return text("Programmatically Opened!"); },
    { .direction = PopoverDirection::Right },
    controller
);

// Elsewhere in code:
controller->toggle();
```

---

## See Also
- [**Tooltip**](./tooltip.md) — Non-interactive informational hover tooltips.
- [**Popup**](./popup.md) — Generalized native floating surface engine.
- [**DropdownMenu**](../Overlays/dropdown_menu.md) — Selection picker dropdown menu.
