# DataGrid

> An enterprise-grade data table widget supporting column resizing, multi-column sorting, column pinning, cell progress bars, badges, pagination, quick live filtering, summary footers, and CSV export.

- **Header File**: `#include "enki/widgets/data_grid.hpp"`
- **C++ Class**: `enki::DataGridWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::DataGrid` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::DataGridProps`
- **Controller**: `enki::DataGridController`
- **Descriptive Structs**: `enki::DataGridColumn`, `enki::DataGridRow`, `enki::DataGridCell`
- **Enums**: `enki::DataGridAlign`, `enki::DataGridPin`, `enki::DataGridSortDirection`, `enki::DataGridSelectionMode`, `enki::DataGridCellType`

---

## Overview

`DataGrid` provides high-density tabular presentation for enterprise workstations and dashboards. Users can sort columns by clicking headers, drag column borders to resize them, pin columns to the left or right, paginate large datasets, filter rows via search queries, and view summary aggregate calculations.

---

## C++ API Definition

### Enums
```cpp
namespace enki {

enum class DataGridAlign {
    Left,
    Center,
    Right
};

enum class DataGridPin {
    None,
    Left,
    Right
};

enum class DataGridSortDirection {
    None,
    Ascending,
    Descending
};

enum class DataGridSelectionMode {
    None,
    RowSingle,
    RowMultiple,
    CellRange
};

enum class DataGridCellType {
    Text,
    Number,
    Badge,
    Progress,
    Custom
};

} // namespace enki
```

### Column & Row Descriptors
```cpp
namespace enki {

struct DataGridColumn {
    std::string      key         = "";             ///< Unique key/field identifier
    std::string      title       = "";             ///< Header text
    float            width       = 120.0f;         ///< Pixel width
    float            min_width   = 50.0f;
    float            max_width   = 600.0f;

    bool             sortable    = true;           ///< Click header to sort
    bool             resizable   = true;           ///< Drag border to resize
    bool             filterable  = true;           ///< Include in quick-filter search
    bool             visible     = true;

    DataGridAlign    align       = DataGridAlign::Left;
    DataGridPin      pin         = DataGridPin::None;
    DataGridCellType cell_type   = DataGridCellType::Text;

    DataGridColumn() = default;
    DataGridColumn(std::string k, std::string t, float w = 120.0f, DataGridAlign a = DataGridAlign::Left);
};

struct DataGridRow {
    std::string id = "";
    std::map<std::string, DataGridCell> cells;
    bool selected = false;

    DataGridRow() = default;
    explicit DataGridRow(std::string row_id);

    DataGridRow& set(const std::string& key, const std::string& val);
    DataGridRow& setBadge(const std::string& key, const std::string& val, Color bg, Color fg);
    DataGridRow& setProgress(const std::string& key, float prog, const std::string& label);
    [[nodiscard]] const std::string& get(const std::string& key) const;
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct DataGrid {
    Key                                     key                 = Key::none();
    std::shared_ptr<DataGridController>     controller          = nullptr;

    DataGridSelectionMode                   selection_mode      = DataGridSelectionMode::RowMultiple;
    bool                                    show_header         = true;
    bool                                    show_row_numbers    = false;
    bool                                    show_pagination     = true;
    bool                                    show_quick_filter   = true;
    bool                                    show_summary_footer = false;
    bool                                    show_borders        = true;
    bool                                    zebra_stripes       = true;

    float                                   header_height       = 38.0f;
    float                                   row_height          = 36.0f;
    float                                   footer_height       = 36.0f;
    float                                   pagination_height   = 42.0f;

    Color                                   background_color    = 0xFF0F172A;
    Color                                   header_bg_color     = 0xFF1E293B;
    Color                                   header_text_color   = 0xFFF1F5F9;
    Color                                   row_bg_color        = 0xFF0F172A;
    Color                                   zebra_row_bg_color  = 0xFF172033;
    Color                                   row_hover_color     = 0xFF1E293B;
    Color                                   row_selected_color  = 0x4038BDF8;
    Color                                   border_color        = 0xFF334155;
    Color                                   text_color          = 0xFFE2E8F0;
    Color                                   sort_icon_color     = 0xFF38BDF8;
    Color                                   footer_bg_color     = 0xFF1E293B;

    float                                   border_radius       = 8.0f;

    // Callbacks
    std::function<void(const std::set<std::string>& selected_ids)> on_selection_changed = nullptr;
    std::function<void(const std::string& row_id)>                 on_row_tap           = nullptr;
    std::function<void(const std::string& row_id)>                 on_row_double_tap    = nullptr;
    std::function<void(const std::string& col, DataGridSortDirection dir)> on_sort_changed = nullptr;
    std::function<void(int new_page)>                              on_page_changed      = nullptr;
    std::function<std::string(const std::string& col_key, const std::vector<DataGridRow>& rows)> summary_calculator = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `key` | `Key` | `Key::none()` | Widget key identifier. |
| `controller` | `std::shared_ptr<DataGridController>`| `nullptr`| Manages columns, rows, sorting, filtering, and pagination. |
| `selection_mode` | `DataGridSelectionMode` | `RowMultiple` | Row selection behavior (`None`, `RowSingle`, `RowMultiple`, `CellRange`). |
| `show_header` | `bool` | `true` | Whether to display the top column header bar. |
| `show_row_numbers` | `bool` | `false` | Shows 1-based sequential row index column. |
| `show_pagination` | `bool` | `true` | Renders bottom page navigation controls. |
| `show_quick_filter` | `bool` | `true` | Shows integrated live search bar. |
| `show_summary_footer` | `bool` | `false` | Renders summary row with calculated aggregates. |
| `show_borders` | `bool` | `true` | Renders borders around the grid and cells. |
| `zebra_stripes` | `bool` | `true` | Alternates row background colors for improved legibility. |
| `header_height` | `float` | `38.0f` | Height in points of the table header row. |
| `row_height` | `float` | `36.0f` | Height in points of each data row. |
| `footer_height` | `float` | `36.0f` | Height in points of the summary footer row. |
| `pagination_height` | `float` | `42.0f` | Height in points of the pagination navigation bar. |
| `border_radius` | `float` | `8.0f` | Outer container corner rounding radius. |
| `background_color` | `Color` | `0xFF0F172A` | Base background fill color for grid container. |
| `header_bg_color` | `Color` | `0xFF1E293B` | Column header bar background color. |
| `header_text_color` | `Color` | `0xFFF1F5F9` | Text color of column header labels. |
| `row_bg_color` | `Color` | `0xFF0F172A` | Background color for odd rows. |
| `zebra_row_bg_color` | `Color` | `0xFF172033` | Background color for even rows (when zebra_stripes = true). |
| `row_hover_color` | `Color` | `0xFF1E293B` | Background highlight fill on mouse hover. |
| `row_selected_color` | `Color` | `0x4038BDF8` | Background highlight fill for selected rows. |
| `border_color` | `Color` | `0xFF334155` | Border stroke color between cells and headers. |
| `text_color` | `Color` | `0xFFE2E8F0` | Default text color for cell values. |
| `sort_icon_color` | `Color` | `0xFF38BDF8` | Color of column sort arrows (▲ / ▼). |
| `footer_bg_color` | `Color` | `0xFF1E293B` | Background fill color of summary footer row. |
| `on_selection_changed` | `std::function<void(const std::set<std::string>&)>` | `nullptr` | Callback fired when selected row IDs change. |
| `on_row_tap` | `std::function<void(const std::string&)>` | `nullptr` | Callback fired on single-clicking a row. |
| `on_row_double_tap` | `std::function<void(const std::string&)>` | `nullptr` | Callback fired on double-clicking a row. |
| `on_sort_changed` | `std::function<void(const std::string&, DataGridSortDirection)>` | `nullptr` | Callback fired when sorting column/direction changes. |
| `on_page_changed` | `std::function<void(int)>` | `nullptr` | Callback fired when navigating pages. |
| `summary_calculator` | `std::function<std::string(const std::string&, const std::vector<DataGridRow>&)>` | `nullptr` | Custom aggregate calculation function for footer. |

---

## Code Examples (From `widgets_demo/data_grid_demo/main.cpp`)

### 1. Initializing Columns, Rows, and DataGrid
```cpp
#include "enki/widgets/data_grid.hpp"

using namespace enki;

WidgetPtr buildEmployeeDataGrid() {
    auto gridCtrl = std::make_shared<DataGridController>();

    // 1. Define Columns
    std::vector<DataGridColumn> cols = {
        DataGridColumn("id", "ID", 70.0f, DataGridAlign::Center),
        DataGridColumn("name", "Employee Name", 170.0f, DataGridAlign::Left),
        DataGridColumn("dept", "Department", 130.0f, DataGridAlign::Left),
    };

    // Badge Column
    DataGridColumn colStatus("status", "Status", 110.0f, DataGridAlign::Center);
    colStatus.cell_type = DataGridCellType::Badge;
    cols.push_back(colStatus);

    // Progress Column
    DataGridColumn colPerf("perf", "Performance", 140.0f, DataGridAlign::Left);
    colPerf.cell_type = DataGridCellType::Progress;
    cols.push_back(colPerf);

    gridCtrl->setColumns(cols);

    // 2. Add Rows
    DataGridRow r1("emp_101");
    r1.set("id", "#101")
      .set("name", "Sarah Jenkins")
      .set("dept", "Engineering")
      .setBadge("status", "ACTIVE", 0x2E10B981, 0xFF10B981)
      .setProgress("perf", 0.94f, "94% Exceptional");
    gridCtrl->addRow(r1);

    // 3. Render Widget
    return DataGrid {
        .controller = gridCtrl,
        .selection_mode = DataGridSelectionMode::RowSingle,
        .zebra_stripes = true,
        .show_pagination = true,
        .on_row_tap = [](const std::string& row_id) {
            std::cout << "Selected Employee: " << row_id << "\n";
        }
    };
}
```

---

## See Also
- [**Table**](../Scrolling-Lists/table.md) — Simple static layout table.
- [**DataTable**](../Scrolling-Lists/data_table.md) — Material-style data table.
- [**SplitView**](./split_view.md) — Resizable dual-pane container.
