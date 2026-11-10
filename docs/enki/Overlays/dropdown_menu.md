# DropdownMenu

> An in-window overlay dropdown menu widget rendered above application content using an unclipped Stack layout, supporting action items, checkboxes, radios, headers, badges, and auto-flip boundary placement.

- **Header File**: `#include "enki/widgets/dropdown_menu.hpp"`
- **C++ Class**: `enki::DropdownMenuWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::DropdownMenu` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::DropdownMenuProps`
- **Item Descriptor**: `enki::DropdownMenuItem`
- **Controller**: `enki::DropdownMenuController`
- **Enums**: `enki::DropdownMenuItemType`, `enki::DropdownPlacement`

---

## Overview

`DropdownMenu` implements an in-window floating selection menu. Unlike native OS subsurfaces (`NativePopups`), it wraps the page `body` inside an unclipped `100% × 100%` `Stack`. When opened, it mounts a transparent click-catcher scrim behind a `Positioned` floating panel positioned accurately against anchor coordinates or custom triggers.

---

## C++ API Definition

### `DropdownMenuItem` Factory Methods
```cpp
namespace enki {

enum class DropdownMenuItemType {
    Standard,   ///< Regular clickable action item
    Checkbox,   ///< Toggleable item with checkmark (✓)
    Radio,      ///< Radio selection bullet (◉/○)
    Header,     ///< Non-clickable group section title
    Divider     ///< Horizontal separator line
};

enum class DropdownPlacement {
    Auto,       ///< Opens below if space allows; flips above if near bottom edge
    Bottom,     ///< Always opens downwards below trigger
    Top         ///< Always opens upwards above trigger
};

struct DropdownMenuItem {
    static DropdownMenuItem standard(std::string id, std::string label,
                                     std::string icon = "", std::string shortcut = "");

    static DropdownMenuItem checkbox(std::string id, std::string label,
                                     bool checked = false, std::string icon = "");

    static DropdownMenuItem radio(std::string id, std::string label,
                                  bool selected = false, std::string icon = "");

    static DropdownMenuItem header(std::string title);
    static DropdownMenuItem divider();

    // Fluent Setters
    DropdownMenuItem& setBadge(std::string text, Color bg = 0x2E38BDF8, Color fg = 0xFF38BDF8);
    DropdownMenuItem& setSubtitle(std::string sub);
    DropdownMenuItem& setDanger(bool d = true);
    DropdownMenuItem& setDisabled(bool d = true);
};

} // namespace enki
```

### Controller (`DropdownMenuController`)
```cpp
namespace enki {

class DropdownMenuController {
public:
    void open();
    void close();
    void toggle();
    void select(const std::string& id);
    [[nodiscard]] bool isOpen() const;
    [[nodiscard]] std::string getSelected() const;
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct DropdownMenu {
    Key                                           key             = Key::none();
    std::vector<DropdownMenuItem>                 items;
    WidgetPtr                                     body            = nullptr; ///< Underlying page body
    WidgetPtr                                     custom_trigger  = nullptr;
    std::shared_ptr<DropdownMenuController>       controller      = nullptr;

    float                                         menu_width      = 240.0f;
    float                                         max_menu_height = 340.0f;
    float                                         border_radius   = 8.0f;
    bool                                          close_on_select = true;
    DropdownPlacement                             placement       = DropdownPlacement::Bottom;

    std::string                                   placeholder     = "Select an option...";
    std::string                                   selected_id     = "";
    float                                         trigger_width   = 220.0f;
    float                                         trigger_height  = 38.0f;

    // Anchor Trigger Positioning
    float                                         anchor_x        = 0.0f;
    float                                         anchor_y        = 0.0f;

    // Theming Colors
    Color                                         background_color = 0xFF1E293B;
    Color                                         border_color     = 0xFF334155;
    Color                                         trigger_bg       = 0xFF0F172A;
    Color                                         trigger_border   = 0xFF334155;
    Color                                         hover_color      = 0x2238BDF8;
    Color                                         text_color       = 0xFFF1F5F9;
    Color                                         subtitle_color   = 0xFF94A3B8;
    Color                                         shortcut_color   = 0xFF64748B;
    Color                                         header_color     = 0xFF38BDF8;
    Color                                         divider_color    = 0xFF334155;
    Color                                         danger_color     = 0xFFEF4444;

    // Callbacks
    std::function<void(const DropdownMenuItem&)>  on_selected      = nullptr;
    std::function<void(const std::string&, bool)> on_toggle_checked= nullptr;
    std::function<void()>                         on_opened        = nullptr;
    std::function<void()>                         on_closed        = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `key` | `Key` | `Key::none()` | Identifier for widget diffing and state preservation. |
| `body` | `WidgetPtr` | `nullptr` | Application page content wrapped by the overlay stack. |
| `items` | `std::vector<DropdownMenuItem>` | `{}` | Collection of menu items, headers, and dividers. |
| `custom_trigger` | `WidgetPtr` | `nullptr` | Custom trigger widget replacing default select button. |
| `controller` | `std::shared_ptr<DropdownMenuController>` | `nullptr` | Controller to toggle menu visibility programmatically. |
| `menu_width` | `float` | `240.0f` | Width of the floating dropdown panel in pixels. |
| `max_menu_height` | `float` | `340.0f` | Maximum viewport height for scrollable items. |
| `border_radius` | `float` | `8.0f` | Menu container corner rounding radius. |
| `close_on_select` | `bool` | `true` | Closes the menu automatically when an item is selected. |
| `placement` | `DropdownPlacement` | `DropdownPlacement::Bottom` | Opening direction (`Auto`, `Bottom`, `Top`). |
| `placeholder` | `std::string` | `"Select an option..."` | Label shown when no item is selected. |
| `selected_id` | `std::string` | `""` | Currently selected item ID. |
| `trigger_width` | `float` | `220.0f` | Width of the default select trigger button. |
| `trigger_height` | `float` | `38.0f` | Height of the default select trigger button. |
| `anchor_x` | `float` | `0.0f` | Absolute X coordinate of trigger in window. |
| `anchor_y` | `float` | `0.0f` | Absolute Y coordinate of trigger in window. |
| `background_color` | `Color` | `0xFF1E293B` | Menu container background color. |
| `border_color` | `Color` | `0xFF334155` | Menu container outline border color. |
| `trigger_bg` | `Color` | `0xFF0F172A` | Default trigger button background color. |
| `trigger_border` | `Color` | `0xFF334155` | Default trigger button border color. |
| `hover_color` | `Color` | `0x2238BDF8` | Item hover background tint. |
| `text_color` | `Color` | `0xFFF1F5F9` | Primary item label font color. |
| `subtitle_color` | `Color` | `0xFF94A3B8` | Item subtitle font color. |
| `shortcut_color` | `Color` | `0xFF64748B` | Trailing shortcut font color. |
| `header_color` | `Color` | `0xFF38BDF8` | Section header font color. |
| `divider_color` | `Color` | `0xFF334155` | Horizontal separator divider color. |
| `danger_color` | `Color` | `0xFFEF4444` | Destructive item accent color. |
| `on_selected` | `std::function<void(const DropdownMenuItem&)>` | `nullptr` | Callback when any item is clicked/selected. |
| `on_toggle_checked` | `std::function<void(const std::string&, bool)>` | `nullptr` | Callback when checkbox/radio state toggles. |
| `on_opened` | `std::function<void()>` | `nullptr` | Callback when dropdown opens. |
| `on_closed` | `std::function<void()>` | `nullptr` | Callback when dropdown closes. |

---

## Code Examples (From `widgets_demo/dropdown_menu_demo/main.cpp`)

### 1. Multi-Section Dropdown with Checkboxes and Badges
```cpp
#include "enki/widgets/dropdown_menu.hpp"
#include "enki/widgets/button.hpp"

using namespace enki;

class SettingsViewState : public State {
    std::shared_ptr<DropdownMenuController> menu_ctrl_ = std::make_shared<DropdownMenuController>();
    bool auto_save_ = true;

public:
    WidgetPtr build(BuildContext& ctx) override {
        // Build items
        std::vector<DropdownMenuItem> items = {
            DropdownMenuItem::header("ENVIRONMENT"),
            DropdownMenuItem::radio("dev",  "Development Sandbox", false, "🛠"),
            DropdownMenuItem::radio("prod", "Production Cluster",   true,  "🌐")
                .setBadge("ACTIVE", 0x2E10B981, 0xFF10B981),
            DropdownMenuItem::divider(),
            DropdownMenuItem::header("PREFERENCES"),
            DropdownMenuItem::checkbox("autosave", "Auto-Save Changes", auto_save_, "💾"),
            DropdownMenuItem::divider(),
            DropdownMenuItem::standard("reset", "Reset to Factory Defaults", "🔄")
                .setDanger(true)
        };

        auto pageContent = button(text("⚙️ Settings Menu ▾"), [this]() {
            menu_ctrl_->toggle();
        });

        return DropdownMenu {
            .items = std::move(items),
            .body = pageContent,
            .controller = menu_ctrl_,
            .placement = DropdownPlacement::Bottom,
            .anchor_x = 40.0f,
            .anchor_y = 60.0f,
            .on_selected = [](const DropdownMenuItem& item) {
                std::cout << "Selected: " << item.label << "\n";
            }
        };
    }
};
```

---

## See Also
- [**ComboBox**](../Input%20Forms/combo_box.md) — Single and multi-select form input box.
- [**Menu**](../NativePopups/menu.md) — OS-level native menu bar and dropdowns.
- [**Dialog**](./dialog.md) — Centered modal overlay card.
