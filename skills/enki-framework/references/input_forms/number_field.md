# NumberField

> A high-precision numeric input widget with steppers, horizontal mouse drag scrubbing, math expression evaluation, unit prefixes/suffixes, and currency formatting.

- **Header File**: `#include "enki/widgets/number_field.hpp"`
- **C++ Class**: `enki::NumberFieldWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::NumberField` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::NumberFieldProps`
- **Controller**: `enki::NumberFieldController`
- **Enums**: `enki::NumberFieldStepperPosition`, `enki::NumberFieldSize`, `enki::NumberFieldWrapMode`

---

## Overview

`NumberField` provides specialized numeric handling beyond generic text boxes. It supports:
1. **Interactive Stepper Buttons**: Stacked (▲/▼), split sides (−/+), or horizontal buttons.
2. **Mouse Drag Scrubbing**: Dragging horizontally adjusts values rapidly (Blender/Figma style).
3. **In-place Math Expressions**: Evaluates expressions typed by the user (e.g. `1920 / 2`, `100 + 25`).
4. **Units & Currencies**: Pre-configured prefix (`"$ "`, `"€ "`) and suffix (`" px"`, `" %"`, `" MB"`) decorations.
5. **Precision & Clamping**: Strict min/max bounds and decimal precision controls.

---

## C++ API Definition

### Controller (`NumberFieldController`)
```cpp
namespace enki {

class NumberFieldController {
public:
    NumberFieldController(double initial_val = 0.0);

    [[nodiscard]] double getValue() const;
    void setValue(double val, bool save_undo = true);

    [[nodiscard]] bool canUndo() const;
    [[nodiscard]] bool canRedo() const;
    bool undo();
    bool redo();
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

enum class NumberFieldStepperPosition {
    RightVertical,   ///< Stacked ▲ / ▼ arrows on the right
    Sides,           ///< − on the left, + on the right
    RightHorizontal, ///< − + side-by-side on the right
    None             ///< Clean input without stepper buttons
};

struct NumberField {
    Key                                    key                      = Key::none();
    std::shared_ptr<NumberFieldController> controller               = nullptr;
    double                                 initial_value            = 0.0;

    // Bounds & Step Sizes
    double                                 min_value                = -std::numeric_limits<double>::infinity();
    double                                 max_value                = std::numeric_limits<double>::infinity();
    double                                 step                     = 1.0;
    double                                 large_step               = 10.0; // With Shift key
    double                                 fine_step                = 0.1;  // With Alt key
    int                                    precision                = 0;    // 0 for int, > 0 for decimals

    // Feature Flags
    bool                                   allow_decimals           = true;
    bool                                   allow_negative           = true;
    bool                                   allow_expressions        = true; // Evaluates "100+50"
    bool                                   enable_scrubbing         = true; // Drag to adjust
    bool                                   enable_auto_repeat       = true; // Auto-repeat stepping on hold
    bool                                   show_thousands_separator = false;

    NumberFieldStepperPosition             stepper_position         = NumberFieldStepperPosition::RightVertical;
    NumberFieldSize                        size                     = NumberFieldSize::Medium;
    NumberFieldWrapMode                    wrap_mode                = NumberFieldWrapMode::Clamp;

    // Prefixes & Suffixes
    std::string                            prefix_text              = ""; // e.g. "$ "
    std::string                            suffix_text              = ""; // e.g. " px"

    // Styling
    TextStyle                              style;
    Color                                  background_color         = 0xFF0F172A;
    Color                                  border_color             = 0xFF334155;
    Color                                  focus_border_color       = 0xFF38BDF8;
    Color                                  button_color             = 0xFF1E293B;
    Color                                  button_hover_color       = 0xFF334155;
    Color                                  button_icon_color        = 0xFF94A3B8;
    Color                                  prefix_suffix_color      = 0xFF64748B;
    Color                                  cursor_color             = 0xFF38BDF8;
    Color                                  selection_color          = 0x4D38BDF8;

    float                                  border_radius            = 8.0f;
    EdgeInsets                             padding                  = EdgeInsets::symmetric(8.0f, 10.0f);

    bool                                   read_only                = false;
    bool                                   disabled                 = false;
    bool                                   auto_focus               = false;

    // Callbacks
    std::function<void(double)>            on_changed               = nullptr;
    std::function<void(double)>            on_submitted             = nullptr;
    std::function<std::string(double)>     custom_formatter         = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `key` | `Key` | `Key::none()` | Optional widget identification key. |
| `controller` | `std::shared_ptr<NumberFieldController>` | `nullptr` | Shared controller managing the double numeric value and undo/redo history. |
| `initial_value` | `double` | `0.0` | Initial value if no controller is provided. |
| `min_value` | `double` | `-inf` | Minimum allowable value. |
| `max_value` | `double` | `+inf` | Maximum allowable value. |
| `step` | `double` | `1.0` | Normal delta increment on stepper click or Arrow keys. |
| `large_step` | `double` | `10.0` | Large delta increment used with Shift or PageUp/PageDown. |
| `fine_step` | `double` | `0.1` | Fine delta increment used with Alt or fine drag. |
| `precision` | `int` | `0` | Number of decimal places (0 for integers, >0 for fixed decimals, -1 for auto). |
| `allow_decimals` | `bool` | `true` | Whether floating-point decimal input is allowed. |
| `allow_negative` | `bool` | `true` | Whether negative numbers are accepted. |
| `allow_expressions`| `bool` | `true` | Evaluates inline math expressions (e.g. `1920 / 2`). |
| `enable_scrubbing` | `bool` | `true` | Enables horizontal mouse click-and-drag value adjustment. |
| `enable_auto_repeat` | `bool` | `true` | Continuously steps values when holding down stepper buttons. |
| `show_thousands_separator` | `bool` | `false` | Formats numbers with thousand comma separators. |
| `stepper_position` | `NumberFieldStepperPosition` | `RightVertical` | Stepper button placement (`RightVertical`, `Sides`, `RightHorizontal`, `None`). |
| `size` | `NumberFieldSize` | `Medium` | Predefined visual size preset (`Small`, `Medium`, `Large`). |
| `wrap_mode` | `NumberFieldWrapMode` | `Clamp` | Behavior when exceeding range (`Clamp` or `Wrap`). |
| `prefix_text` | `std::string` | `""` | Leading label inside the input field (e.g. `"$ "`). |
| `suffix_text` | `std::string` | `""` | Trailing unit label inside the input field (e.g. `" px"`, `" %"`). |
| `style` | `TextStyle` | `{}` | Text typography and styling for numeric value. |
| `background_color` | `Color` | `0xFF0F172A` | Background fill color of the input box. |
| `border_color` | `Color` | `0xFF334155` | Default outline border color. |
| `focus_border_color`| `Color` | `0xFF38BDF8` | Border color when focused. |
| `button_color` | `Color` | `0xFF1E293B` | Stepper button background color. |
| `button_hover_color`| `Color` | `0xFF334155` | Stepper button background color on hover. |
| `button_icon_color` | `Color` | `0xFF94A3B8` | Icon color for stepper buttons. |
| `prefix_suffix_color`| `Color`| `0xFF64748B` | Text color for prefix and suffix decorations. |
| `cursor_color` | `Color` | `0xFF38BDF8` | Color of text blinking insertion caret. |
| `selection_color` | `Color` | `0x4D38BDF8` | Color of highlighted text selection. |
| `border_radius` | `float` | `8.0f` | Corner rounding radius of the input field. |
| `padding` | `EdgeInsets` | `symmetric(8, 10)` | Inner padding for text and stepper layout. |
| `read_only` | `bool` | `false` | Disables keyboard text editing while keeping stepper interaction. |
| `disabled` | `bool` | `false` | Fully greys out and disables the input field. |
| `auto_focus` | `bool` | `false` | Automatically focuses the field upon mounting. |
| `on_changed` | `std::function<void(double)>` | `nullptr` | Triggered every time the numeric value changes. |
| `on_submitted` | `std::function<void(double)>` | `nullptr` | Triggered when Enter key is pressed. |
| `custom_formatter`| `std::function<std::string(double)>` | `nullptr` | Custom formatting function to override display string. |

---

## Code Examples (From `widgets_demo/number_field_demo/main.cpp`)

### 1. Currency Input with Thousands Separator
```cpp
#include "enki/widgets/number_field.hpp"

using namespace enki;

auto priceField = NumberField {
    .initial_value = 1250.0,
    .min_value = 0.0,
    .step = 50.0,
    .precision = 2,
    .show_thousands_separator = true,
    .prefix_text = "$ ",
    .on_changed = [](double val) {
        std::cout << "Price: " << val << "\n";
    }
};
```

### 2. Dimension Field with Math Expression Support
```cpp
auto widthField = NumberField {
    .initial_value = 1920.0,
    .min_value = 100.0,
    .max_value = 7680.0,
    .step = 10.0,
    .allow_expressions = true, // User can type "1920 / 2"
    .suffix_text = " px",
};
```

### 3. Percentage Scrubbing Slider
```cpp
auto opacityField = NumberField {
    .initial_value = 100.0,
    .min_value = 0.0,
    .max_value = 100.0,
    .step = 1.0,
    .enable_scrubbing = true, // Drag horizontally to adjust
    .suffix_text = " %",
};
```

---

## See Also
- [**Slider**](./slider.md) — Horizontal bar slider for visual numeric selection.
- [**TextField**](./text_field.md) — Standard alphanumeric input.
