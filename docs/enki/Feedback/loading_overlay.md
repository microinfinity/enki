# LoadingOverlay

> A full-screen or scoped modal loading overlay widget that prevents user interactions during long-running background tasks, featuring 4 indicator styles, live progress updates, and cancel callbacks.

- **Header File**: `#include "enki/widgets/loading_overlay.hpp"`
- **C++ Class**: `enki::LoadingOverlayWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::LoadingOverlay` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::LoadingOverlayProps`
- **Controller**: `enki::LoadingOverlayController`
- **Indicator Enum**: `enki::LoadingIndicatorStyle` (`Spinner`, `ProgressRing`, `ProgressBar`, `DotsPulse`, `Custom`)

---

## Overview

`LoadingOverlay` blocks user interaction across a view or entire window while an asynchronous operation is in flight (e.g. database migration, compilation, file upload). It wraps the page `body` inside an unclipped stack, displays a darkened scrim, renders the selected indicator style, and allows updating the progress percentage and status message in real time.

---

## C++ API Definition

### `LoadingIndicatorStyle` Enum
```cpp
namespace enki {

enum class LoadingIndicatorStyle {
    Spinner,        ///< Rotating arc/spoke spinner
    ProgressRing,   ///< Circular progress ring with % center label
    ProgressBar,    ///< Linear progress bar with track
    DotsPulse,      ///< 3 pulsing/bouncing dots
    Custom          ///< Custom developer-provided child widget
};

struct LoadingOverlayProps {
    Key                   key             = Key::none();
    LoadingIndicatorStyle indicator_style = LoadingIndicatorStyle::Spinner;

    std::string           title           = "Processing...";
    std::string           message         = "Please wait while operation completes.";
    float                 progress        = 0.0f;
    bool                  is_determinate  = false;

    bool                  allow_cancel    = false;
    std::string           cancel_label    = "Cancel Operation";

    float                 width           = 360.0f;
    float                 border_radius   = 12.0f;

    Color                 background_color= 0xFF1E293B;
    Color                 border_color    = 0xFF334155;
    Color                 overlay_color   = 0xB30B1120;
    Color                 title_color     = 0xFFFFFFFF;
    Color                 message_color   = 0xFF94A3B8;
    Color                 accent_color    = 0xFF38BDF8;

    WidgetPtr             custom_indicator= nullptr;

    std::function<void()> on_cancel       = nullptr;
    std::function<void()> on_shown        = nullptr;
    std::function<void()> on_hidden       = nullptr;
};

} // namespace enki
```

### Controller (`LoadingOverlayController`)
```cpp
namespace enki {

class LoadingOverlayController {
public:
    void show(const LoadingOverlayProps& opts);
    void hide();
    void setProgress(float p, const std::string& msg = "");
    void setMessage(const std::string& msg);
    [[nodiscard]] bool isLoading() const;

    // Semantic Convenience Helpers
    void showSpinner(std::string title = "Loading...", std::string msg = "Please wait...",
                     bool cancelable = false, std::function<void()> on_cancel = nullptr);

    void showProgressRing(float p, std::string title = "Uploading...", std::string msg = "",
                          bool cancelable = false, std::function<void()> on_cancel = nullptr);

    void showProgressBar(float p, std::string title = "Compiling...", std::string msg = "",
                         bool cancelable = false, std::function<void()> on_cancel = nullptr);

    void showDots(std::string title = "Syncing...", std::string msg = "Connecting...");
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct LoadingOverlay {
    Key                                       key             = Key::none();
    WidgetPtr                                 body            = nullptr; ///< Content wrapped
    std::shared_ptr<LoadingOverlayController> controller      = nullptr;
    bool                                      initial_loading = false;
    LoadingOverlayProps                       initial_options = {};

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
| `controller` | `std::shared_ptr<LoadingOverlayController>` | `nullptr` | Controller to show/hide the busy overlay. |
| `initial_loading` | `bool` | `false` | Whether overlay is visible initially upon mount. |
| `initial_options` | `LoadingOverlayProps` | `{}` | Initial loader options when mounted. |

### `LoadingOverlayProps` Reference

| Option | Type | Default | Description |
|---|---|---|---|
| `indicator_style` | `LoadingIndicatorStyle` | `Spinner` | Visual loader variant (`Spinner`, `ProgressRing`, `ProgressBar`, `DotsPulse`, `Custom`). |
| `title` | `std::string` | `"Processing..."` | Main bold header label. |
| `message` | `std::string` | `"Please wait..."` | Descriptive subtitle or current status message. |
| `progress` | `float` | `0.0f` | Current fraction completed (`0.0f` to `1.0f`). |
| `is_determinate` | `bool` | `false` | If true, renders progress rings or bars according to `progress`. |
| `allow_cancel` | `bool` | `false` | Displays an interactive cancel button when true. |
| `cancel_label` | `std::string` | `"Cancel Operation"` | Text shown on the cancel button. |
| `width` | `float` | `360.0f` | Modal card width in pixels. |
| `border_radius` | `float` | `12.0f` | Modal card corner rounding radius. |
| `background_color` | `Color` | `0xFF1E293B` | Modal card surface background color. |
| `border_color` | `Color` | `0xFF334155` | Modal card outline border color. |
| `overlay_color` | `Color` | `0xB30B1120` | Dimmed backdrop scrim color. |
| `title_color` | `Color` | `0xFFFFFFFF` | Title text font color. |
| `message_color` | `Color` | `0xFF94A3B8` | Subtitle text font color. |
| `accent_color` | `Color` | `0xFF38BDF8` | Progress bar and spinner accent color. |
| `custom_indicator` | `WidgetPtr` | `nullptr` | Custom loader widget when `indicator_style == Custom`. |
| `on_cancel` | `std::function<void()>` | `nullptr` | Callback executed when user cancels the operation. |
| `on_shown` | `std::function<void()>` | `nullptr` | Callback executed when loading overlay becomes visible. |
| `on_hidden` | `std::function<void()>` | `nullptr` | Callback executed when loading overlay is dismissed. |

---

## Code Examples (From `widgets_demo/loading_overlay_demo/main.cpp`)

### 1. File Upload with Determinate ProgressRing
```cpp
#include "enki/widgets/loading_overlay.hpp"
#include "enki/widgets/button.hpp"

using namespace enki;

class UploadScreenState : public State {
    std::shared_ptr<LoadingOverlayController> loader_ = std::make_shared<LoadingOverlayController>();

public:
    WidgetPtr build(BuildContext& ctx) override {
        auto mainContent = button(text("Upload Firmware"), [this]() {
            // Show initial ring at 0%
            loader_->showProgressRing(0.0f, "Uploading Firmware", "Connecting to device...", true, [this]{
                std::cout << "Upload canceled by user.\n";
                loader_->hide();
            });

            // Simulate progress update
            loader_->setProgress(0.45f, "Transferred 45% (11.2 MB / 24.8 MB)");
        });

        return LoadingOverlay {
            .body = mainContent,
            .controller = loader_
        };
    }
};
```

### 2. Indeterminate Spinner with Cancel Option
```cpp
loader->showSpinner("Building Project", "Running Meson build...", true, []{
    std::cout << "Build terminated.\n";
});
```

---

## See Also
- [**Spinner**](./spinner.md) — Unconstrained standalone spinner widgets.
- [**ProgressBar**](./progress_bar.md) — Horizontal linear progress bars.
- [**ProgressRing**](./progress_ring.md) — Circular arc progress rings.
