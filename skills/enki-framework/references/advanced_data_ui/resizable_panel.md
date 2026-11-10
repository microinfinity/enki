# ResizablePanel

> A floating tool window and resizable panel widget supporting multi-edge and corner drag resizing, drag-to-move header bars, min/max dimension constraints, and minimize/maximize window controls.

- **Header File**: `#include "enki/widgets/resizable_panel.hpp"`
- **C++ Class**: `enki::ResizablePanelWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::ResizablePanel` (converts implicitly to `WidgetPtr`)
- **Options Struct**: `enki::ResizablePanelOptions`
- **Controller**: `enki::ResizablePanelController`
- **Mode Enum**: `enki::ResizablePanelMode` (`Floating`, `Docked`)

---

## Overview

`ResizablePanel` provides in-window floating tool windows (such as property inspectors, debug consoles, HUD diagnostics, and detached tool palettes). When in **`Floating`** mode, users can grab the top title bar to drag the window across the workspace and drag any border or bottom-right corner grip (`◢`) to resize it with fluid 60+ FPS cursor feedback.

---

## C++ API Definition

### `ResizablePanelMode` Enum
```cpp
namespace enki {

enum class ResizablePanelMode {
    Floating,   ///< Freeform floating window with title bar and drag-to-move
    Docked      ///< Docked panel resizing along its boundary edge
};

} // namespace enki
```

### Controller (`ResizablePanelController`)
```cpp
namespace enki {

class ResizablePanelController {
public:
    void setSize(float width, float height);
    void setPosition(float x, float y);
    void setMinimized(bool min);
    void setMaximized(bool max);
    void close();
    void reset();

    [[nodiscard]] Point getPosition() const;
    [[nodiscard]] Size getSize() const;
};

} // namespace enki
```

### Options & Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct ResizablePanelOptions {
    ResizablePanelMode mode              = ResizablePanelMode::Floating;
    std::string        title             = "Tool Window";
    std::string        icon              = "🛠️";

    float              initial_x         = 240.0f;
    float              initial_y         = 120.0f;
    float              initial_width     = 460.0f;
    float              initial_height    = 320.0f;

    float              min_width         = 240.0f;
    float              min_height        = 160.0f;
    float              max_width         = 1000.0f;
    float              max_height        = 800.0f;

    bool               show_header       = true;
    bool               show_corner_grip  = true;  ///< Bottom-right corner resize handle
    bool               allow_drag_move   = true;  ///< Drag title bar to move
    bool               allow_minimize    = true;
    bool               allow_maximize    = true;
    bool               allow_close       = true;

    float              border_radius     = 10.0f;
    float              handle_thickness  = 6.0f;

    Color              background_color  = 0xFF1E293B; // Slate 800
    Color              border_color      = 0xFF334155; // Slate 700
    Color              active_border_col = 0xFF0284C7; // Blue 600
    Color              header_bg_color   = 0xFF0F172A; // Slate 900
    Color              title_color       = 0xFFFFFFFF; // White
    Color              grip_color        = 0xFF64748B; // Slate 500
    Color              grip_hover_color  = 0xFF38BDF8; // Sky 400

    // Callbacks
    std::function<void(float w, float h)> on_resized         = nullptr;
    std::function<void(float x, float y)> on_moved           = nullptr;
    std::function<void(bool is_min)>      on_minimize_toggled= nullptr;
    std::function<void(bool is_max)>      on_maximize_toggled= nullptr;
    std::function<void()>                 on_closed          = nullptr;
};

struct ResizablePanel {
    Key                                       key        = Key::none();
    WidgetPtr                                 child      = nullptr; ///< Content inside panel
    WidgetPtr                                 body       = nullptr; ///< Workspace page body to wrap
    ResizablePanelOptions                     options    = {};
    std::shared_ptr<ResizablePanelController> controller = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `key` | `Key` | `Key::none()` | Widget key identifier. |
| `child` | `WidgetPtr` | `nullptr` | Content rendered inside the floating tool window. |
| `body` | `WidgetPtr` | `nullptr` | Main workspace background wrapped by the window overlay. |
| `controller` | `std::shared_ptr<ResizablePanelController>`| `nullptr`| Handle for positioning, sizing, and minimizing programmatically. |
| `options.mode` | `ResizablePanelMode` | `Floating` | Floating window or docked boundary panel. |
| `options.title` | `std::string` | `"Tool Window"` | Title bar text label. |
| `options.icon` | `std::string` | `"🛠️"` | Leading emoji/icon in title bar. |
| `options.initial_x`, `initial_y` | `float` | `240.0f`, `120.0f` | Initial floating window position coordinates. |
| `options.initial_width`, `initial_height` | `float` | `460.0f`, `320.0f` | Initial window dimensions. |
| `options.min_width`, `min_height` | `float` | `240.0f`, `160.0f` | Minimum allowed dimensions during drag resize. |
| `options.max_width`, `max_height` | `float` | `1000.0f`, `800.0f` | Maximum allowed dimensions during drag resize. |
| `options.show_header` | `bool` | `true` | Shows top window title bar. |
| `options.show_corner_grip`| `bool` | `true` | Renders diagonal resize handle at bottom-right corner. |
| `options.allow_drag_move` | `bool` | `true` | Allows dragging by the title bar across the screen. |
| `options.allow_minimize` | `bool` | `true` | Enables window minimize control button. |
| `options.allow_maximize` | `bool` | `true` | Enables window maximize control button. |
| `options.allow_close` | `bool` | `true` | Enables window close control button. |
| `options.border_radius` | `float` | `10.0f` | Window container corner rounding radius. |
| `options.handle_thickness`| `float` | `6.0f` | Interactive drag hit-test thickness on edges. |
| `options.background_color`| `Color` | `0xFF1E293B` | Window content background fill color. |
| `options.border_color` | `Color` | `0xFF334155` | Inactive window border stroke color. |
| `options.active_border_col`| `Color` | `0xFF0284C7` | Active/focused window border stroke color. |
| `options.header_bg_color` | `Color` | `0xFF0F172A` | Title bar header background color. |
| `options.title_color` | `Color` | `0xFFFFFFFF` | Title bar text color. |
| `options.grip_color` | `Color` | `0xFF64748B` | Corner grip icon color. |
| `options.grip_hover_color`| `Color` | `0xFF38BDF8` | Corner grip color on mouse hover. |
| `options.on_resized` | `std::function<void(float, float)>` | `nullptr` | Callback fired on resize updates. |
| `options.on_moved` | `std::function<void(float, float)>` | `nullptr` | Callback fired when window is moved. |
| `options.on_minimize_toggled` | `std::function<void(bool)>` | `nullptr` | Callback fired when minimize button is clicked. |
| `options.on_maximize_toggled` | `std::function<void(bool)>` | `nullptr` | Callback fired when maximize button is clicked. |
| `options.on_closed` | `std::function<void()>` | `nullptr` | Callback fired when window close button is clicked. |

---

## Code Examples (From `widgets_demo/resizable_panel_demo/main.cpp`)

### 1. Floating Diagnostics Inspector Window
```cpp
#include "enki/widgets/resizable_panel.hpp"
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"
#include "enki/widgets/flexbox.hpp"

using namespace enki;

WidgetPtr buildWorkspaceWithFloatingInspector(WidgetPtr mainWorkspaceView) {
    auto panelCtrl = std::make_shared<ResizablePanelController>();

    auto inspectorContent = column({
        .gap = 8_px,
        .padding = EdgeInsets::all(16.0f),
        .children = {
            text("⚡ GPU Pipeline Status", { .color = 0xFF38BDF8, .font_weight = FontWeight::Bold }),
            text("FPS: 60 • Frame Time: 3.8ms", { .color = 0xFF10B981 }),
            text("Allocated VRAM: 240 MB / 8 GB", { .color = 0xFFCBD5E1 })
        }
    });

    return ResizablePanel {
        .child = inspectorContent,
        .body = mainWorkspaceView,
        .controller = panelCtrl,
        .options = {
            .mode = ResizablePanelMode::Floating,
            .title = "Pipeline Inspector",
            .icon = "🛠️",
            .initial_x = 200.0f,
            .initial_y = 100.0f,
            .initial_width = 440.0f,
            .initial_height = 280.0f,
            .min_width = 300.0f,
            .min_height = 180.0f,
            .allow_drag_move = true,
            .show_corner_grip = true
        }
    };
}
```

---

## See Also
- [**SplitView**](./split_view.md) — Two-pane layout with draggable divider.
- [**Dialog**](../Overlays/dialog.md) — Modal prompt cards.
- [**Popup**](../NativePopups/popup.md) — Native OS-level floating surfaces.
