# MouseRegion & Cursor Control

> Platform mouse pointer icon switching, custom cursor states, wheel scrolling routing, and pointer hit-test configuration in Enki.

- **Header File**: `#include "enki/widgets/gesture_detector.hpp"`
- **Host Widget**: `enki::GestureDetector`
- **Cursor Enum**: `enki::SystemCursor`
- **Scroll Callback**: `GestureScrollCallback` (`std::function<void(float dx, float dy)>`)

---

## Overview

In desktop and workstation environments, mouse cursor visual states inform users when an element is clickable, resizable, selectable, or loading. Enki handles mouse cursor management directly through `GestureDetector`:
- Setting `cursor_type` changes the operating system's hardware mouse pointer when the cursor hovers over the widget.
- Setting `on_scroll` routes two-dimensional mouse wheel / trackpad scroll deltas (`dx`, `dy`) to custom viewports.

---

## C++ API Definition

### `SystemCursor` Enum
```cpp
namespace enki {

enum class SystemCursor {
    Default,
    Arrow,
    Pointer,          // Link / Hand
    Text,             // I-Beam
    Crosshair,
    Move,
    NotAllowed,
    ResizeHorizontal,
    ResizeVertical,
    ResizeTopLeft,     // NW-SE / Top-Left corner
    ResizeTopRight,    // NE-SW / Top-Right corner
    ResizeBottomLeft,  // SW-NE / Bottom-Left corner
    ResizeBottomRight, // SE-NW / Bottom-Right corner
    Wait
};

} // namespace enki
```

### `GestureDetector` Configuration
```cpp
namespace enki {

struct GestureDetectorProps {
    // ...
    SystemCursor          cursor_type       = SystemCursor::Default;
    HitTestBehavior       hit_test_behavior = HitTestBehavior::DeferToChild;
    GestureScrollCallback on_scroll         = nullptr; ///< (float dx, float dy)
};

} // namespace enki
```

---

## Code Examples (From `widgets_demo/gesture_demo/main.cpp`)

### 1. Resizable Splitter Handle with Resize Cursor
```cpp
#include "enki/widgets/gesture_detector.hpp"
#include "enki/widgets/container.hpp"

using namespace enki;

WidgetPtr buildSplitterHandle(std::function<void(float dy)> onResizeDrag) {
    return gestureDetector({
        // Switch hardware cursor to vertical resize arrows
        .cursor_type = SystemCursor::ResizeVertical,
        .hit_test_behavior = HitTestBehavior::Opaque,
        .on_pan_update = [onResizeDrag](const DragUpdateDetails& d) {
            onResizeDrag(d.delta.y);
        },
        .child = container({
            .color = 0xFF334155, // Slate border color
            .height = StyleValue::point(6.0f),
            .width = StyleValue::percent(100.0f)
        })
    });
}
```

### 2. Custom Zoom / Scroll Surface
```cpp
auto zoomSurface = gestureDetector({
    .cursor_type = SystemCursor::Crosshair,
    .hit_test_behavior = HitTestBehavior::Opaque,
    .on_scroll = [](float dx, float dy) {
        std::cout << "Mouse wheel delta: X=" << dx << " Y=" << dy << "\n";
        // Apply zoom or horizontal scroll
    },
    .child = myMapCanvas
});
```

---

## See Also
- [**HoverRegion**](./hover_region.md) — Cursor entry, continuous move, and exit events.
- [**GestureDetector**](./gesture_detector.md) — Comprehensive gesture detection widget.
- [**Draggable**](./draggable.md) — Floating drag-and-drop source widgets.
