# NavigationBar

> A multi-style navigation bar widget supporting Material 3 bottom bars, macOS floating glassmorphism capsule docks, desktop top header bars, and segmented tabs with 60+ to 240+ FPS sliding indicator physics.

- **Header File**: `#include "enki/widgets/navigation_bar.hpp"`
- **C++ Class**: `enki::NavigationBarWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::NavigationBar` (converts implicitly to `WidgetPtr`)
- **Options Struct**: `enki::NavigationBarOptions`
- **Item Descriptor**: `enki::NavigationBarItem`
- **Enums**: `enki::NavigationBarStyle`, `enki::NavIndicatorStyle`, `enki::NavItemLayout`

---

## Overview

`NavigationBar` is a modern navigation component supporting four distinct architectural styles:
1. **`BottomStandard`**: Standard mobile/tablet bottom navigation bar.
2. **`FloatingPill`**: Floating capsule island with multi-layer glassmorphism shadows (macOS Dock / iOS 18 style).
3. **`TopHeader`**: Full desktop/web top navigation bar supporting leading brand logos, search bar placeholders, and trailing action buttons.
4. **`SegmentedCapsule`**: Compact segmented pill switcher.

---

## C++ API Definition

### Enums
```cpp
namespace enki {

enum class NavigationBarStyle {
    BottomStandard,   ///< Material 3 style bottom bar
    FloatingPill,     ///< macOS Dock / iOS 18 floating capsule island with glassmorphism
    TopHeader,        ///< Desktop top header bar with brand, search & actions
    SegmentedCapsule  ///< Compact pill-shaped segmented tabs
};

enum class NavIndicatorStyle {
    Pill,             ///< Rounded capsule pill enclosing active item
    Underline,        ///< Glowing underline beneath active item
    Dot,              ///< Radiant glowing dot beneath active item
    Glow,             ///< Ambient gradient aura behind active item
    None              ///< Color change only without dedicated shape
};

enum class NavItemLayout {
    Vertical,         ///< Icon on top, label on bottom (Mobile standard)
    Horizontal,       ///< Icon leading on left, label on right (Desktop standard)
    IconOnly,         ///< Icon only (Compact dock)
    LabelOnly         ///< Text label only (Tab strip)
};

} // namespace enki
```

### `NavigationBarItem` Descriptor
```cpp
namespace enki {

struct NavigationBarItem {
    std::string label;
    IconData    icon;
    IconData    selected_icon;      ///< Optional active state icon
    std::string badge;              ///< Numeric count ("5", "99+") or tag text ("NEW")
    bool        dot_badge = false;  ///< Glowing notification dot
    bool        enabled   = true;
    std::string sublabel;
    std::string tooltip;
    std::string id;

    NavigationBarItem(std::string lbl, IconData ic, std::string bdg = "", bool dot = false);
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct NavigationBar {
    Key                                   key               = Key::none();
    std::vector<NavigationBarItem>        items;
    int                                   selected_index    = 0;

    std::function<void(int)>              on_item_selected  = nullptr;
    std::function<void(int)>              on_item_reselect  = nullptr;
    std::function<void(std::string_view)> on_action_clicked = nullptr;

    NavigationBarOptions                  options           = {};

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `key` | `Key` | `Key::none()` | Optional widget identification key. |
| `items` | `std::vector<NavigationBarItem>` | `{}` | Selectable navigation items. |
| `selected_index` | `int` | `0` | 0-indexed active item. |
| `on_item_selected` | `std::function<void(int)>` | `nullptr` | Callback fired when selecting an item. |
| `on_item_reselect` | `std::function<void(int)>` | `nullptr` | Callback fired when re-clicking the already-active item (e.g. scroll to top). |
| `on_action_clicked`| `std::function<void(std::string_view)>` | `nullptr` | Callback fired when desktop top header trailing actions are clicked. |
| `options.style` | `NavigationBarStyle` | `BottomStandard` | Overall navigation bar layout paradigm (`BottomStandard`, `FloatingPill`, `TopHeader`, `SegmentedCapsule`). |
| `options.indicator_style`| `NavIndicatorStyle` | `Pill` | Shape of the sliding indicator (`Pill`, `Underline`, `Dot`, `Glow`, `None`). |
| `options.item_layout`| `NavItemLayout` | `Vertical` | Internal arrangement of icon and text (`Vertical`, `Horizontal`, `IconOnly`, `LabelOnly`). |
| `options.background_color` | `Color` | `0xFF1E293B` | Primary surface background color. |
| `options.border_color` | `Color` | `0xFF334155` | Border or separator line color. |
| `options.active_color` | `Color` | `0xFF38BDF8` | Active icon & label tint. |
| `options.inactive_color` | `Color` | `0xFF94A3B8` | Inactive icon & label tint. |
| `options.indicator_color` | `Color` | `0x2638BDF8` | Active indicator fill/highlight color. |
| `options.indicator_border` | `Color` | `0x4D38BDF8` | Active indicator outline border color. |
| `options.badge_color` | `Color` | `0xFFEF4444` | Notification badge background color. |
| `options.badge_text_color` | `Color` | `0xFFFFFFFF` | Notification badge text color. |
| `options.height` | `float` | `68.0f` | Total bar height in pixels. |
| `options.width` | `float` | `0.0f` | Bar width (0.0 for 100% full width, >0 for fixed width). |
| `options.corner_radius` | `float` | `0.0f` | Outer bar corner radius (e.g. 24px for floating pill). |
| `options.enable_glassmorphism` | `bool` | `false` | Enables multi-layer glass backdrop effect. |
| `options.leading_title` | `std::string` | `""` | Brand or application title for top header mode. |
| `options.show_search_placeholder` | `bool` | `false` | Displays inline search box mock for top header mode. |
| `options.trailing_actions` | `std::vector<std::string>` | `{}` | Action button labels in top header mode. |
| `options.enable_animations` | `bool` | `true` | Enables animated indicator sliding transitions. |

---

## Code Examples (From `widgets_demo/navigation_bar_demo/main.cpp`)

### 1. Desktop Top Header Bar with Brand & Actions
```cpp
#include "enki/widgets/navigation_bar.hpp"
#include "enki/widgets/icon.hpp"

using namespace enki;

WidgetPtr buildDesktopHeader(int activeIndex, auto onSelect) {
    return NavigationBar {
        .items = {
            {"Dashboard", Icons::Material::dashboard()},
            {"Projects",  Icons::Material::folder()},
            {"Team",      Icons::Material::people(), "3"},
            {"Settings",  Icons::Material::settings()},
        },
        .selected_index = activeIndex,
        .on_item_selected = onSelect,
        .options = {
            .style = NavigationBarStyle::TopHeader,
            .item_layout = NavItemLayout::Horizontal,
            .indicator_style = NavIndicatorStyle::Underline,
            .leading_title = "ENKI Studio",
            .show_search_placeholder = true,
            .trailing_actions = {"Docs", "GitHub"},
        }
    };
}
```

### 2. Floating macOS Dock / iOS 18 Capsule Island
```cpp
WidgetPtr buildFloatingDock(int activeIndex, auto onSelect) {
    return NavigationBar {
        .items = {
            {"Home",     Icons::Material::home()},
            {"Search",   Icons::Material::search()},
            {"Inbox",    Icons::Material::mail(), "12"},
            {"Terminal", Icons::Material::terminal()},
        },
        .selected_index = activeIndex,
        .on_item_selected = onSelect,
        .options = {
            .style = NavigationBarStyle::FloatingPill,
            .item_layout = NavItemLayout::IconOnly,
            .indicator_style = NavIndicatorStyle::Pill,
            .enable_glassmorphism = true,
            .corner_radius = 28.0f,
            .height = 56.0f,
            .width = 340.0f,
        }
    };
}
```

---

## See Also
- [**NavigationRail**](./navigation_rail.md) — Vertical side rail navigation.
- [**TabBar**](./tab_bar.md) — Lightweight tab strip.
- [**Breadcrumb**](./breadcrumb.md) — Path breadcrumb indicators.
