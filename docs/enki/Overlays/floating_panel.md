# FloatingPanel

> An in-window freeform draggable and resizable floating window overlay widget (MDI / tool palette / HUD inspector) featuring 8-direction multi-edge resize handles, title bar dragging, window state toggling (minimize, maximize, restore), active focus elevation, and magnetic edge snapping.

- **Header File**: `#include "enki/widgets/floating_panel.hpp"`
- **C++ Class**: `enki::FloatingPanelWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::FloatingPanel` (converts implicitly to `WidgetPtr`)
- **Options Struct**: `enki::FloatingPanelOptions`
- **Controller**: `enki::FloatingPanelController`
- **Enums**: `enki::FloatingPanelDisplayState`
- **Helper Function**: `enki::floatingPanel(const FloatingPanelProps&)`

---

## Overview

`FloatingPanel` provides a full-featured in-window floating tool window, inspector palette, or audio HUD. Rendered within Enki's **Container-Wrapping Architecture**, it sits above the main viewport `body` in a top-level `Stack`, allowing users to drag it around freely, resize it from any corner or edge, and toggle between normal, minimized, and maximized display states.

### Key Capabilities
- **Title Bar & Window Chrome**:
  - Interactive header with custom icon, title, and standard window buttons (minimize, maximize/restore, close).
  - Drag-to-move by dragging anywhere across the title bar with viewport boundary clamping.
- **8-Direction Resizing**:
  - Invisible edge and corner resize handles: N, S, E, W, NW, NE, SW, SE.
  - Interactive cursor switching (`ResizeHorizontal`, `ResizeVertical`, `ResizeTopLeft`, etc.).
  - Size constraints enforcement (`min_width`, `min_height`, `max_width`, `max_height`).
- **Window State Management**:
  - `Normal`: standard floating window dimensions.
  - `Minimized`: collapses to title bar only (38px height) to conserve workspace screen real estate.
  - `Maximized`: fills the application viewport; clicking restore returns back to previous floating bounds.
- **Elevation & Focus**:
  - Clicking on the panel brings it to the front and activates its glow border highlight (`0xFF38BDF8`).
- **Magnetic Edge Snapping**:
  - Automatically snaps to window boundaries when dragging within a configurable threshold distance (e.g. 16px).

---

## C++ API Definition

### Data Structures & Enums

```cpp
namespace enki {

enum class FloatingPanelDisplayState {
    Normal,     ///< Standard floating window
    Minimized,  ///< Collapsed to title bar only
    Maximized   ///< Maximized to fill viewport
};

struct FloatingPanelOptions {
    std::string title = "Tool Inspector";
    std::string icon = "🎛️";

    float initial_x = 220.0f;
    float initial_y = 100.0f;
    float initial_width = 460.0f;
    float initial_height = 340.0f;

    float min_width = 240.0f;
    float min_height = 140.0f;
    float max_width = 1200.0f;
    float max_height = 900.0f;

    bool allow_drag = true;
    bool allow_resize = true;
    bool allow_minimize = true;
    bool allow_maximize = true;
    bool allow_close = true;

    bool snap_to_edges = true;
    float snap_threshold = 16.0f;

    float border_radius = 12.0f;
    float resize_handle_thickness = 8.0f;

    // Theme Colors
    Color background_color    = 0xF80F172A;
    Color border_color        = 0xFF334155;
    Color active_border_color = 0xFF38BDF8;
    Color titlebar_bg_color   = 0xFF0B0F19;
    Color title_color         = 0xFFF8FAFC;
    Color subtitle_color      = 0xFF94A3B8;

    std::function<void(float x, float y)> on_moved;
    std::function<void(float w, float h)> on_resized;
    std::function<void(float x, float y)> on_drag_update;
    std::function<void(float w, float h)> on_resize_update;
    std::function<void(FloatingPanelDisplayState state)> on_state_changed;
    std::function<void()> on_closed;
};

class FloatingPanelController {
public:
    void show();
    void hide();
    void toggle();
    [[nodiscard]] bool isOpen() const;

    void setPosition(float x, float y);
    [[nodiscard]] Point getPosition() const;

    void setSize(float w, float h);
    [[nodiscard]] Size getSize() const;

    void minimize();
    void maximize();
    void restore();
    [[nodiscard]] FloatingPanelDisplayState getState() const;

    void bringToFront();
};

struct FloatingPanel {
    Key                                      key          = Key::none();
    WidgetPtr                                content      = nullptr;
    WidgetPtr                                body         = nullptr;
    WidgetPtr                                child        = nullptr; ///< Alias for body
    FloatingPanelOptions                     options      = {};
    std::shared_ptr<FloatingPanelController> controller   = nullptr;
    bool                                     initial_open = true;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `key` | `Key` | `Key::none()` | Identifier for widget diffing and state preservation. |
| `content` | `WidgetPtr` | `nullptr` | Content inside the floating window panel. |
| `body` / `child` | `WidgetPtr` | `nullptr` | Application page content wrapped by the overlay stack. |
| `options` | `FloatingPanelOptions` | `{}` | Sizing, constraints, window buttons, and theming. |
| `controller` | `std::shared_ptr<FloatingPanelController>` | `nullptr` | Controller for imperative position, size, and display state changes. |
| `initial_open` | `bool` | `true` | Whether the floating panel starts opened. |

### `FloatingPanelOptions` Reference

| Option | Type | Default | Description |
|---|---|---|---|
| `title` | `std::string` | `"Tool Inspector"` | Window title bar header text. |
| `icon` | `std::string` | `"🎛️"` | Leading icon / emoji in title bar. |
| `initial_x` | `float` | `220.0f` | Starting X coordinate in viewport. |
| `initial_y` | `float` | `100.0f` | Starting Y coordinate in viewport. |
| `initial_width` | `float` | `460.0f` | Starting width in pixels. |
| `initial_height` | `float` | `340.0f` | Starting height in pixels. |
| `min_width` | `float` | `240.0f` | Minimum allowed resize width. |
| `min_height` | `float` | `140.0f` | Minimum allowed resize height. |
| `max_width` | `float` | `1200.0f` | Maximum allowed resize width. |
| `max_height` | `float` | `900.0f` | Maximum allowed resize height. |
| `allow_drag` | `bool` | `true` | Enables dragging window by title bar. |
| `allow_resize` | `bool` | `true` | Enables edge and corner resize handles. |
| `allow_minimize` | `bool` | `true` | Shows collapse to title bar button. |
| `allow_maximize` | `bool` | `true` | Shows expand to fill viewport button. |
| `allow_close` | `bool` | `true` | Shows close window button. |
| `snap_to_edges` | `bool` | `true` | Snaps to window edges when dragging nearby. |
| `snap_threshold` | `float` | `16.0f` | Distance in pixels triggering edge snapping. |
| `border_radius` | `float` | `12.0f` | Window container corner rounding radius. |
| `resize_handle_thickness`| `float` | `8.0f` | Thickness of invisible resize margin grab zone. |
| `background_color` | `Color` | `0xF80F172A` | Window surface background color. |
| `border_color` | `Color` | `0xFF334155` | Inactive window outline border color. |
| `active_border_color` | `Color` | `0xFF38BDF8` | Active focused window outline border color. |
| `titlebar_bg_color` | `Color` | `0xFF0B0F19` | Title bar background fill color. |
| `title_color` | `Color` | `0xFFF8FAFC` | Title header font color. |
| `subtitle_color` | `Color` | `0xFF94A3B8` | Subtitle description font color. |
| `on_moved` | `std::function<void(float, float)>` | `nullptr` | Callback when dragging completes. |
| `on_resized` | `std::function<void(float, float)>` | `nullptr` | Callback when resizing completes. |
| `on_drag_update` | `std::function<void(float, float)>` | `nullptr` | Real-time position callback during drag. |
| `on_resize_update` | `std::function<void(float, float)>` | `nullptr` | Real-time size callback during resize. |
| `on_state_changed` | `std::function<void(FloatingPanelDisplayState)>` | `nullptr` | Callback when minimized, maximized, or restored. |
| `on_closed` | `std::function<void()>` | `nullptr` | Callback when close button is clicked. |

} // namespace enki
```

---

## Declarative Usage Example

```cpp
#include "enki/widgets/floating_panel.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

auto panel_ctrl = std::make_shared<FloatingPanelController>();

WidgetPtr app_view = FloatingPanel {
    .content = container({
        .child = text("Inspector Tool Content")
    }),
    .body = container({
        .child = text("Main Application Workspace")
    }),
    .options = {
        .title = "DSP Audio Matrix",
        .icon = "🎛️",
        .initial_x = 240.0f,
        .initial_y = 120.0f,
        .initial_width = 460.0f,
        .initial_height = 320.0f
    },
    .controller = panel_ctrl
};
```

---

## See Also
- [**CommandPalette**](./command_palette.md) — Keyboard-driven fuzzy-search command overlay.
- [**Spotlight**](./spotlight.md) — Dimmed overlay with focused hole region.
- [**Dialog**](./dialog.md) — Centered modal prompt cards.
- [**Popup**](../NativePopups/popup.md) — OS-level multi-window floating surfaces.
