# AspectRatio

> A layout widget and styling constraint that sizes a child to a specific width-to-height ratio.

- **Header Files**: `#include "enki/widgets/aspect_ratio.hpp"`, `#include "enki/widgets/container.hpp"`, and `#include "enki/widgets/flexbox.hpp"`
- **C++ Class**: `enki::AspectRatioWidget` (inherits from `enki::StatelessWidget`)
- **Props Struct**: `enki::AspectRatioProps`
- **Factory Helpers**: `enki::aspectRatio(float ratio, WidgetPtr child, Key key = Key::none())`, `enki::aspectRatio(const AspectRatioProps& props)`
- **Declarative Property**: `.aspect_ratio = <float>` on `Container`, `FlexItem`, and `Flexbox`
- **Underlying Engine**: Direct Anu Layout Engine integration (`ANUNodeStyleSetAspectRatio`)

---

## Overview

In Enki, aspect ratio constraints can be applied in two ways:
1. **Dedicated Widget (`aspectRatio`)**: A lightweight `StatelessWidget` wrapper that forwards to `container()` with `aspect_ratio` configured.
2. **Direct Property**: Applied directly via `.aspect_ratio` on any `Container`, `FlexItem`, or `Flexbox`.

It enforces a strict mathematical proportion between the width and height of the box:

$$\text{aspect\_ratio} = \frac{\text{width}}{\text{height}}$$

When either dimension is constrained (e.g., width is set to `320_px` or `100_pct`), the Anu Layout Engine calculates the dependent dimension automatically.

---

## C++ API Definition

### Dedicated Widget & Props (`aspect_ratio.hpp`)
```cpp
namespace enki {

struct AspectRatioProps {
    Key       key   = Key::none();
    float     ratio = 1.0f;
    WidgetPtr child = nullptr;
};

class AspectRatioWidget : public StatelessWidget {
public:
    float     ratio = 1.0f;   ///< Width / Height ratio (e.g. 1.77 for 16:9).
    WidgetPtr child = nullptr;

    explicit AspectRatioWidget(Key key = Key::none());
    [[nodiscard]] WidgetPtr build(BuildContext& ctx) override;
    [[nodiscard]] std::string_view typeName() const override { return "AspectRatio"; }
};

inline WidgetPtr aspectRatio(float ratio, WidgetPtr child, Key key = Key::none());
inline WidgetPtr aspectRatio(const AspectRatioProps& props);

} // namespace enki
```

### Property Definition on Container / Flexbox
```cpp
// Within ContainerProps, FlexboxProps, and FlexItemProps:
std::optional<float> aspect_ratio;
```

---

## Properties Reference (`AspectRatioProps`)

| Property | Type | Default | Description |
|---|---|---|---|
| `key` | `Key` | `Key::none()` | Unique widget identifier for tree reconciliation. |
| `ratio` | `float` | `1.0f` | Aspect ratio expressed as width / height (e.g. `16.0f / 9.0f`). |
| `child` | `WidgetPtr` | `nullptr` | Child widget constrained by this aspect ratio. |

---

## Standard Aspect Ratio Values

| Ratio | Value | Common Application |
|---|---|---|
| `1.0f` | `1.0f` | Square avatar, icon button, profile picture. |
| `16.0f / 9.0f` | `1.7778f` | Widescreen video players, YouTube previews, hero banners. |
| `4.0f / 3.0f` | `1.3333f` | Classic camera photos, standard preview cards. |
| `21.0f / 9.0f` | `2.3333f` | Ultrawide cinematic banners. |

---

## Code Examples

### 1. Using the Dedicated `aspectRatio` Helper
```cpp
#include "enki/widgets/aspect_ratio.hpp"
#include "enki/widgets/image.hpp"

using namespace enki;

// Constrains a video or banner image to standard 16:9 widescreen
WidgetPtr buildBanner() {
    return aspectRatio(16.0f / 9.0f, image("assets/hero.png"));
}
```

### 2. Video Player Container via Direct Property (16:9 Widescreen)
```cpp
#include "enki/widgets/container.hpp"
#include "enki/widgets/text.hpp"

using namespace enki;

WidgetPtr buildVideoPreview() {
    return container({
        .width = 320_px,
        .aspect_ratio = 16.0f / 9.0f, // Height automatically computes to 180px
        .color = 0xFF000000,
        .border_radius = BorderRadius::circular(8.0f),
        .align = Alignment::Center,
        .child = text("▶ Video Stream", { .color = 0xFFFFFFFF }),
    });
}
```

### 2. Percentage Width with Aspect Ratio in a Flex Row (From `tests/widgets/test_flexbox.cpp`)
```cpp
#include "enki/widgets/flexbox.hpp"

auto mediaItem = flexItem({
    .width = 50_pct,               // Takes 50% of parent width
    .aspect_ratio = 2.0f,           // width / height = 2.0 -> height = width / 2
    .child = imageThumbnailWidget,
});
```

---

## See Also
- [**Container**](./container.md) — Visual container supporting aspect ratio.
- [**ConstrainedBox**](./constrained_box.md) — Min/max size bounds.
- [**FractionallySizedBox**](./fractionally_sized_box.md) — Percentage dimensions.
