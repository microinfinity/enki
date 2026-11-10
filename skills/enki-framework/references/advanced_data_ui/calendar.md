# Calendar

> An interactive calendar and scheduling widget supporting single-date and date-range selection, event dot markers, agenda panel integration, month/year navigation, and today quick-jumping.

- **Header File**: `#include "enki/widgets/calendar.hpp"`
- **C++ Class**: `enki::CalendarWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::Calendar` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::CalendarProps`
- **Controller**: `enki::CalendarController`
- **Date Representation**: `enki::CalendarDate`
- **Event Model**: `enki::CalendarEvent`
- **Selection Enum**: `enki::CalendarSelectionMode` (`Single`, `Range`, `None`)

---

## Overview

`Calendar` provides a full-featured desktop calendar component. It computes month layout grids, highlights today's date, indicates events with colored dot badges under corresponding day numbers, and supports selecting single dates or continuous date ranges (useful for booking systems and date-range filtering).

---

## C++ API Definition

### `CalendarDate` Struct
```cpp
namespace enki {

struct CalendarDate {
    int year  = 2026;
    int month = 8;    ///< 1 to 12
    int day   = 19;   ///< 1 to 31

    constexpr CalendarDate() = default;
    constexpr CalendarDate(int y, int m, int d);

    constexpr bool operator==(const CalendarDate& o) const;
    constexpr bool operator!=(const CalendarDate& o) const;
    constexpr bool operator<(const CalendarDate& o) const;

    [[nodiscard]] std::string toString() const; ///< Formats as "YYYY-MM-DD"
};

enum class CalendarSelectionMode {
    Single,     ///< Pick a single date
    Range,      ///< Pick a start date and an end date
    None        ///< Read-only calendar
};

struct CalendarEvent {
    std::string  id          = "";
    std::string  title       = "";
    std::string  time_str    = "";   ///< e.g. "10:00 AM - 11:30 AM"
    std::string  description = "";
    CalendarDate date;
    Color        color       = 0xFF38BDF8;
    bool         is_all_day  = false;

    CalendarEvent() = default;
    CalendarEvent(std::string id, std::string title, CalendarDate date,
                  Color col = 0xFF38BDF8, std::string time = "", std::string desc = "");
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct Calendar {
    Key                                 key                = Key::none();
    std::vector<CalendarEvent>          events;
    std::shared_ptr<CalendarController> controller         = nullptr;

    CalendarSelectionMode               selection_mode     = CalendarSelectionMode::Single;
    CalendarDate                        initial_date       = {2026, 8, 19};
    int                                 first_day_of_week  = 1; ///< 0 = Sunday, 1 = Monday

    bool                                show_today_btn     = true;
    bool                                show_adjacent_days = true;
    bool                                highlight_today    = true;

    float                               width              = 360.0f;
    float                               border_radius      = 12.0f;

    Color                               background_color   = 0xFF1E293B; // Slate 800
    Color                               border_color       = 0xFF334155; // Slate 700
    Color                               header_bg_color    = 0xFF0F172A; // Slate 900
    Color                               title_color        = 0xFFFFFFFF; // White
    Color                               weekday_color      = 0xFF94A3B8; // Slate 400
    Color                               day_color          = 0xFFF1F5F9; // Slate 100
    Color                               adjacent_day_color = 0xFF475569; // Slate 600
    Color                               today_ring_color   = 0xFF38BDF8; // Sky 400
    Color                               selected_bg_color  = 0xFF0284C7; // Blue 600
    Color                               in_range_bg_color  = 0x330284C7; // Blue 20%
    Color                               hover_bg_color     = 0x33334155; // Slate 700 opacity

    // Callbacks
    std::function<void(CalendarDate)>                         on_date_selected  = nullptr;
    std::function<void(CalendarDate start, CalendarDate end)> on_range_selected = nullptr;
    std::function<void(int year, int month)>                  on_month_changed  = nullptr;
    std::function<void(const CalendarEvent&)>                 on_event_clicked  = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `key` | `Key` | `Key::none()` | Widget key identifier. |
| `events` | `std::vector<CalendarEvent>` | `{}` | Scheduled items rendered as colored indicator dots on days. |
| `controller` | `std::shared_ptr<CalendarController>` | `nullptr` | Programmatic controller for date selection and month switching. |
| `selection_mode` | `CalendarSelectionMode` | `Single` | Mode for picking dates (`Single`, `Range`, `None`). |
| `initial_date` | `CalendarDate` | `{2026, 8, 19}` | The initially viewed/selected calendar date. |
| `first_day_of_week`| `int` | `1` | Starting day column (0 = Sunday, 1 = Monday). |
| `show_today_btn` | `bool` | `true` | Renders quick-jump "Today" button in header. |
| `show_adjacent_days`| `bool` | `true` | Displays dimmed days belonging to adjacent months. |
| `highlight_today` | `bool` | `true` | Highlights current system date with a distinctive ring. |
| `width` | `float` | `360.0f` | Total width constraint of calendar container. |
| `border_radius` | `float` | `12.0f` | Corner rounding radius. |
| `background_color` | `Color` | `0xFF1E293B` | Base background fill color. |
| `border_color` | `Color` | `0xFF334155` | Outer container border color. |
| `header_bg_color` | `Color` | `0xFF0F172A` | Background color of month/year navigation header bar. |
| `title_color` | `Color` | `0xFFFFFFFF` | Month and year title font color. |
| `weekday_color` | `Color` | `0xFF94A3B8` | Column day names font color (Mon, Tue, etc.). |
| `day_color` | `Color` | `0xFFF1F5F9` | Standard day number text color. |
| `adjacent_day_color`| `Color` | `0xFF475569` | Dimmed day number text color for adjacent months. |
| `today_ring_color` | `Color` | `0xFF38BDF8` | Accent ring color outlining today's date. |
| `selected_bg_color`| `Color` | `0xFF0284C7` | Background highlight fill for selected day cell. |
| `in_range_bg_color`| `Color` | `0x330284C7` | Semi-transparent highlight fill for dates within a selected range. |
| `hover_bg_color` | `Color` | `0x33334155` | Cell background tint on cursor hover. |
| `on_date_selected` | `std::function<void(CalendarDate)>` | `nullptr` | Callback fired when a date is clicked in `Single` mode. |
| `on_range_selected`| `std::function<void(CalendarDate, CalendarDate)>` | `nullptr` | Callback fired when start and end range dates are established. |
| `on_month_changed` | `std::function<void(int, int)>` | `nullptr` | Callback fired when navigating to another month/year. |
| `on_event_clicked` | `std::function<void(const CalendarEvent&)>` | `nullptr` | Callback fired when clicking a date containing an event. |

---

## Code Examples (From `widgets_demo/calendar_demo/main.cpp`)

### 1. Interactive Event Calendar with Date Selection
```cpp
#include "enki/widgets/calendar.hpp"

using namespace enki;

class CalendarViewState : public State {
    std::shared_ptr<CalendarController> cal_ctrl_ = std::make_shared<CalendarController>();
    CalendarDate active_date_ = {2026, 8, 19};

public:
    WidgetPtr build(BuildContext& ctx) override {
        std::vector<CalendarEvent> events = {
            CalendarEvent("e1", "Sprint Retrospective", CalendarDate(2026, 8, 19), 0xFF10B981, "10:00 AM"),
            CalendarEvent("e2", "v1.0 Production Deploy", CalendarDate(2026, 8, 22), 0xFFEF4444, "02:00 PM")
        };

        return Calendar {
            .events = std::move(events),
            .controller = cal_ctrl_,
            .selection_mode = CalendarSelectionMode::Single,
            .initial_date = active_date_,
            .on_date_selected = [this](CalendarDate selected) {
                active_date_ = selected;
                std::cout << "Selected date: " << selected.toString() << "\n";
                setState([]{});
            }
        };
    }
};
```

---

## See Also
- [**DatePicker**](../Input%20Forms/date_picker.md) — Popup dialog date selection field.
- [**Timeline**](./timeline.md) — Sequential workflow progress.
- [**DataGrid**](./data_grid.md) — Large tabular record views.
