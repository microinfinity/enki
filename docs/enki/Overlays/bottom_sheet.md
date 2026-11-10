# BottomSheet

> An in-window slide-up bottom sheet overlay supporting multi-detent stage snapping (Peek, Half, Full), smooth finger/mouse dragging physics, drag handles, and semi-transparent scrims.

- **Header File**: `#include "enki/widgets/bottom_sheet.hpp"`
- **C++ Class**: `enki::BottomSheetWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::BottomSheet` (converts implicitly to `WidgetPtr`)
- **Options Struct**: `enki::BottomSheetOptions`
- **Controller**: `enki::BottomSheetController`
- **Enums**: `enki::BottomSheetType`, `enki::BottomSheetDetent`

---

## Overview

`BottomSheet` slides up from the lower edge of the window. Designed for responsive workflows, detail inspectors, media player trays, and contextual action panels, it features **multi-detent height snapping**. Users can drag the sheet up or down with physics interpolation, or trigger programmatic snapping between predefined height thresholds.

---

## C++ API Definition

### Detents & Types
```cpp
namespace enki {

enum class BottomSheetType {
    Modal,        ///< Overlaid above a darkened scrim backdrop
    Persistent    ///< Embedded within layout (mini-player, tool trays)
};

enum class BottomSheetDetent {
    Hidden,       ///< 0.0 fraction (dismissed)
    Peek,         ///< Mini-player / summary bar (approx 90px / 0.15)
    Half,         ///< 0.50 fraction (half screen height)
    Full          ///< 0.88 fraction (near full screen height)
};

} // namespace enki
```

### Controller (`BottomSheetController`)
```cpp
namespace enki {

class BottomSheetController {
public:
    void show(BottomSheetDetent detent = BottomSheetDetent::Half);
    void hide();
    void toggle();
    void snapTo(BottomSheetDetent detent);
    [[nodiscard]] bool isOpen() const;
    [[nodiscard]] BottomSheetDetent getDetent() const;
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct BottomSheet {
    Key                                    key               = Key::none();
    WidgetPtr                              sheet_content     = nullptr; ///< Content inside sheet
    WidgetPtr                              body              = nullptr; ///< Main page body content
    bool                                   initial_open      = false;

    BottomSheetType                        type              = BottomSheetType::Modal;
    BottomSheetDetent                      initial_detent    = BottomSheetDetent::Half;

    bool                                   show_drag_handle  = true;
    bool                                   show_close_button = true;
    bool                                   enable_drag       = true;
    bool                                   close_on_overlay  = true;

    float                                  peek_height       = 90.0f;
    float                                  half_fraction     = 0.50f;
    float                                  full_fraction     = 0.88f;
    float                                  border_radius     = 16.0f;

    // Styling Colors
    Color                                  background_color  = 0xFF1E293B;
    Color                                  border_color      = 0xFF334155;
    Color                                  handle_color      = 0xFF64748B;
    Color                                  handle_hover_col  = 0xFF94A3B8;
    Color                                  overlay_color     = 0x99000000;
    Color                                  title_color       = 0xFFFFFFFF;
    Color                                  subtitle_color    = 0xFF94A3B8;

    std::string                            title             = "";
    std::string                            subtitle          = "";

    // Callbacks
    std::function<void()>                  on_opened         = nullptr;
    std::function<void()>                  on_closed         = nullptr;
    std::function<void(BottomSheetDetent)> on_detent_changed = nullptr;
    std::function<void(float fraction)>    on_drag_progress  = nullptr;

    std::shared_ptr<BottomSheetController> controller        = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `key` | `Key` | `Key::none()` | Identifier for widget diffing and state preservation. |
| `body` | `WidgetPtr` | `nullptr` | Application page content wrapped by the overlay. |
| `sheet_content` | `WidgetPtr` | `nullptr` | Widgets displayed inside the sliding sheet panel. |
| `initial_open` | `bool` | `false` | Whether the sheet is presented initially upon creation. |
| `controller` | `std::shared_ptr<BottomSheetController>` | `nullptr` | Controller to open, hide, and snap detents programmatically. |
| `type` | `BottomSheetType` | `BottomSheetType::Modal` | Modal (over darkened scrim) or Persistent (embedded tool tray). |
| `initial_detent` | `BottomSheetDetent` | `BottomSheetDetent::Half` | Starting height stage when opened (`Hidden`, `Peek`, `Half`, `Full`). |
| `show_drag_handle` | `bool` | `true` | Renders a pill grab handle centered along the top edge. |
| `show_close_button` | `bool` | `true` | Shows standard close button in header. |
| `enable_drag` | `bool` | `true` | Allows dragging the sheet with cursor or touch gestures. |
| `close_on_overlay` | `bool` | `true` | Dismisses the sheet when clicking the dimmed scrim backdrop. |
| `peek_height` | `float` | `90.0f` | Height in pixels for the `Peek` detent. |
| `half_fraction` | `float` | `0.50f` | Height viewport fraction for the `Half` detent. |
| `full_fraction` | `float` | `0.88f` | Height viewport fraction for the `Full` detent. |
| `border_radius` | `float` | `16.0f` | Top corner rounding radius. |
| `background_color` | `Color` | `0xFF1E293B` | Sheet container background fill color. |
| `border_color` | `Color` | `0xFF334155` | Sheet outline border color. |
| `handle_color` | `Color` | `0xFF64748B` | Drag handle bar idle color. |
| `handle_hover_col` | `Color` | `0xFF94A3B8` | Drag handle bar hovered color. |
| `overlay_color` | `Color` | `0x99000000` | Dimmed scrim backdrop color. |
| `title` | `std::string` | `""` | Optional header title text. |
| `title_color` | `Color` | `0xFFFFFFFF` | Header title font color. |
| `subtitle` | `std::string` | `""` | Optional header subtitle text. |
| `subtitle_color` | `Color` | `0xFF94A3B8` | Header subtitle font color. |
| `on_opened` | `std::function<void()>` | `nullptr` | Callback fired when sheet becomes fully visible. |
| `on_closed` | `std::function<void()>` | `nullptr` | Callback fired when sheet completes dismissal. |
| `on_detent_changed` | `std::function<void(BottomSheetDetent)>` | `nullptr` | Callback fired when the active detent stage changes. |
| `on_drag_progress` | `std::function<void(float)>` | `nullptr` | Callback fired during dragging with normalized height fraction. |

---

## Code Examples (From `widgets_demo/bottom_sheet_demo/main.cpp`)

### 1. Modal Share Sheet with Detent Snapping
```cpp
#include "enki/widgets/bottom_sheet.hpp"
#include "enki/widgets/button.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

class DocumentScreenState : public State {
    std::shared_ptr<BottomSheetController> sheet_ = std::make_shared<BottomSheetController>();

public:
    WidgetPtr build(BuildContext& ctx) override {
        auto mainContent = button(text("Share Document"), [this]() {
            sheet_->show(BottomSheetDetent::Half);
        });

        auto sharePanel = column({
            .gap = 12_px,
            .children = {
                text("Share with Team", { .font_size = 16.0f, .font_weight = FontWeight::Bold }),
                button(text("Copy Public Link"), [this]() { sheet_->hide(); }),
                button(text("Email Invite"),     [this]() { sheet_->hide(); }),
            }
        });

        return BottomSheet {
            .sheet_content = sharePanel,
            .body = mainContent,
            .controller = sheet_,
            .type = BottomSheetType::Modal,
            .initial_detent = BottomSheetDetent::Half,
            .show_drag_handle = true,
            .border_radius = 16.0f,
        };
    }
};
```

---

## See Also
- [**Drawer**](../Navigation/drawer.md) — Lateral slide-in navigation panel.
- [**Dialog**](./dialog.md) — Centered modal prompt cards.
- [**Snackbar**](./snackbar.md) — Brief toast notification overlay.
