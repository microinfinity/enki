# CommandPalette

> An in-window keyboard-driven fuzzy-search command launcher overlay (Ctrl+K / Cmd+K style) featuring intelligent subsequence scoring with character match highlighting, category section grouping, recent commands MRU history, keyboard navigation (Arrow Up/Down, Enter, Esc), danger action badges, and animated entrance/exit transitions.

- **Header File**: `#include "enki/widgets/command_palette.hpp"`
- **C++ Class**: `enki::CommandPaletteWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::CommandPalette` (converts implicitly to `WidgetPtr`)
- **Options Struct**: `enki::CommandPaletteOptions`
- **Controller**: `enki::CommandPaletteController`
- **Data Struct**: `enki::CommandItem`
- **Helper Function**: `enki::commandPalette(const CommandPaletteProps&)`

---

## Overview

`CommandPalette` provides a modern, high-performance command launcher and quick-search overlay inspired by industry-standard developer tools (VS Code, Raycast, Sublime Text, Linear, and macOS Spotlight). Rendered within Enki's **Container-Wrapping Architecture**, it wraps the underlying viewport `body` inside an unclipped `100% × 100%` `Stack`, displaying an animated dark obsidian backdrop scrim and a centered floating command card when invoked.

### Key Capabilities

- **Intelligent Fuzzy Match & Highlight Engine**:
  - Exact match, prefix match, and subsequence matching.
  - Bonus weights for word boundaries (spaces, underscores, dashes, slashes) and camelCase capital letters.
  - Character match highlighting: visual emphasis (e.g. Sky 400 bold text runs) on matched characters in item titles.
  - Secondary match support in subtitles and keywords.
- **Full Keyboard Navigation**:
  - Global `Ctrl+K` / `Cmd+K` shortcut listener to open or toggle the palette from anywhere in the window.
  - `ArrowDown` / `ArrowUp` to cycle through items smoothly, automatically skipping disabled commands.
  - `Home` / `End` to jump directly to first or last available command.
  - `Enter` / `Return` to execute the active command and invoke its `on_execute` callback.
  - `Escape` or clicking the backdrop scrim to dismiss the palette.
- **Categorized Sections & Recent History**:
  - Automatically groups commands under distinct category headers (e.g., "Navigation", "Git", "Editor", "System").
  - Tracks executed commands in a Most-Recently-Used (MRU) history cache and surfaces them under a "Recent" section when the search box is empty.
- **Visual Styling & Danger States**:
  - Badges for keyboard shortcuts (`Ctrl+Shift+P`, `Alt+F`, etc.) and tags (`Pro`, `Danger`, etc.).
  - Highlighting for destructive actions (`is_danger = true`) with red danger accents.
- **Animated Physical Transitions**:
  - Hardware-accelerated scale-and-fade in/out driven by `AnimationController` and `Ticker`.

---

## C++ API Definition

### Data Structures

```cpp
namespace enki {

struct CommandItem {
    std::string id = "";
    std::string title = "";
    std::string subtitle = "";
    std::string category = "Commands";
    std::string icon = "⚡";
    std::string shortcut = "";
    std::string badge = "";
    std::vector<std::string> keywords = {};

    bool disabled = false;
    bool is_danger = false;

    std::function<void()> on_execute = nullptr;

    CommandItem() = default;
    CommandItem(std::string id_, std::string title_, std::string category_ = "Commands",
                std::string shortcut_ = "", std::string icon_ = "⚡",
                std::function<void()> on_exec = nullptr);
};

struct CommandPaletteOptions {
    Color overlay_color         = 0xCC080C14; ///< Deep dark backdrop scrim (80% obsidian)
    Color card_bg_color         = 0xF80F172A; ///< Slate 900 palette card
    Color border_color          = 0xFF334155; ///< Slate 700 card border
    Color input_bg_color        = 0xFF0B0F19; ///< Slate 950 search field background
    Color input_text_color      = 0xFFF8FAFC; ///< Slate 50 query text
    Color placeholder_color     = 0xFF64748B; ///< Slate 500 placeholder text
    Color item_hover_bg         = 0x3338BDF8; ///< Sky 500/20% hover highlight
    Color item_selected_bg      = 0xFF0284C7; ///< Sky 600 active item selection
    Color item_title_color      = 0xFFF8FAFC; ///< Slate 50 item text
    Color item_subtitle_color   = 0xFF94A3B8; ///< Slate 400 description text
    Color highlight_match_color = 0xFF38BDF8; ///< Sky 400 fuzzy matched characters highlight
    Color shortcut_badge_bg    = 0xFF1E293B; ///< Slate 800 shortcut tag container
    Color shortcut_text_color   = 0xFF94A3B8; ///< Slate 400 shortcut label
    Color section_header_color  = 0xFF64748B; ///< Slate 500 category header text

    float card_width            = 620.0f;     ///< Palette card width
    float max_list_height       = 400.0f;     ///< Max height of results viewport
    float card_border_radius    = 14.0f;      ///< Outer card rounded corners
    float top_margin            = 90.0f;      ///< Top offset distance from viewport top

    std::string placeholder     = "Type a command or search...";
    std::string empty_text      = "No matching commands found";

    bool enable_global_shortcut = true;      ///< Listen for Ctrl+K / Cmd+K to toggle
    bool auto_close_on_select   = true;      ///< Auto-dismiss overlay after executing command
    bool barrier_dismissible    = true;      ///< Close when clicking outside on backdrop scrim
    bool show_recent            = true;      ///< Show recent commands when query is empty
    size_t max_results          = 20;        ///< Max items to display in list

    // Callbacks
    std::function<void()> on_open;
    std::function<void()> on_close;
    std::function<void(const std::string&)> on_query_change;
    std::function<void(const CommandItem&)> on_item_selected;
};

class CommandPaletteController {
public:
    void open();
    void close();
    void toggle();
    [[nodiscard]] bool isOpen() const;

    void setQuery(std::string q);
    [[nodiscard]] std::string getQuery() const;

    void setItems(std::vector<CommandItem> items);
    void selectNext();
    void selectPrevious();
    void executeActive();
    void clearRecent();
};

struct CommandPalette {
    Key                                       key          = Key::none();
    WidgetPtr                                 body         = nullptr;
    WidgetPtr                                 child        = nullptr; ///< Alias for body
    std::vector<CommandItem>                  items        = {};
    CommandPaletteOptions                     options      = {};
    std::shared_ptr<CommandPaletteController> controller   = nullptr;
    bool                                      initial_open = false;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `key` | `Key` | `Key::none()` | Identifier for widget diffing and state preservation. |
| `body` / `child` | `WidgetPtr` | `nullptr` | Application page content wrapped by the overlay. |
| `items` | `std::vector<CommandItem>` | `{}` | Initial collection of searchable command items. |
| `options` | `CommandPaletteOptions` | `{}` | Visual customization, keyboard shortcuts, and dimensions. |
| `controller` | `std::shared_ptr<CommandPaletteController>` | `nullptr` | Controller to open, close, query, or execute commands. |
| `initial_open` | `bool` | `false` | Whether palette is open initially upon creation. |

### `CommandPaletteOptions` Reference

| Option | Type | Default | Description |
|---|---|---|---|
| `overlay_color` | `Color` | `0xCC080C14` | Deep dark backdrop scrim color. |
| `card_bg_color` | `Color` | `0xF80F172A` | Floating search card background color. |
| `border_color` | `Color` | `0xFF334155` | Search card border outline color. |
| `input_bg_color` | `Color` | `0xFF0B0F19` | Search input field background. |
| `input_text_color` | `Color` | `0xFFF8FAFC` | Search input query text color. |
| `placeholder_color` | `Color` | `0xFF64748B` | Search input placeholder text color. |
| `item_hover_bg` | `Color` | `0x3338BDF8` | Item background highlight on mouse hover. |
| `item_selected_bg` | `Color` | `0xFF0284C7` | Active selected item background highlight. |
| `item_title_color` | `Color` | `0xFFF8FAFC` | Primary title font color. |
| `item_subtitle_color` | `Color` | `0xFF94A3B8` | Subtitle description font color. |
| `highlight_match_color`| `Color` | `0xFF38BDF8` | Highlight color for matched character substrings. |
| `shortcut_badge_bg` | `Color` | `0xFF1E293B` | Shortcut pill background badge color. |
| `shortcut_text_color` | `Color` | `0xFF94A3B8` | Shortcut pill font color. |
| `section_header_color` | `Color` | `0xFF64748B` | Category section header font color. |
| `card_width` | `float` | `620.0f` | Palette card width in pixels. |
| `max_list_height` | `float` | `400.0f` | Maximum results list viewport height. |
| `card_border_radius` | `float` | `14.0f` | Card corner rounding radius. |
| `top_margin` | `float` | `90.0f` | Distance from window top edge. |
| `placeholder` | `std::string` | `"Type a command or search..."` | Search box placeholder text. |
| `empty_text` | `std::string` | `"No matching commands found"` | Message when no commands match query. |
| `enable_global_shortcut`| `bool` | `true` | Listen for `Ctrl+K` / `Cmd+K` global keyboard shortcut. |
| `auto_close_on_select` | `bool` | `true` | Close palette automatically when an action executes. |
| `barrier_dismissible` | `bool` | `true` | Dismiss palette when clicking outside on backdrop scrim. |
| `show_recent` | `bool` | `true` | Display recently executed items when query is empty. |
| `max_results` | `size_t` | `20` | Maximum results displayed simultaneously. |
| `on_open` | `std::function<void()>` | `nullptr` | Callback when palette opens. |
| `on_close` | `std::function<void()>` | `nullptr` | Callback when palette closes. |
| `on_query_change` | `std::function<void(const std::string&)>` | `nullptr` | Callback when search input text changes. |
| `on_item_selected` | `std::function<void(const CommandItem&)>` | `nullptr` | Callback when an item is selected/executed. |

} // namespace enki
```

---

## Declarative Usage Example

```cpp
#include "enki/app/app.hpp"
#include "enki/widgets/command_palette.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

auto palette_ctrl = std::make_shared<CommandPaletteController>();

std::vector<CommandItem> commands = {
    CommandItem("nav_dash", "Go to Dashboard", "Navigation", "Ctrl+1", "📊", [] {
        std::cout << "Dashboard opened\n";
    }),
    CommandItem("git_pull", "Git: Pull Latest Origin", "Git", "Ctrl+Shift+P", "📥", [] {
        std::cout << "Git pull executed\n";
    }),
    CommandItem("edit_format", "Format Code Buffer", "Editor", "Shift+Alt+F", "✨", [] {
        std::cout << "Buffer formatted\n";
    })
};

WidgetPtr app_view = CommandPalette {
    .body = container({
        .color = 0xFF0B0F19,
        .child = text("Press Ctrl+K anytime to launch commands", { .color = 0xFF94A3B8 })
    }),
    .items = commands,
    .options = {
        .placeholder = "Type a command or search...",
        .enable_global_shortcut = true
    },
    .controller = palette_ctrl
};
```

---

## See Also
- [**Spotlight**](./spotlight.md) — Full-screen dimmed overlay with highlighted region.
- [**FloatingPanel**](./floating_panel.md) — Draggable and resizable floating window overlay.
- [**Dialog**](./dialog.md) — Centered modal prompt dialogs.
- [**BottomSheet**](./bottom_sheet.md) — Slide-up sheet with detent snapping.
