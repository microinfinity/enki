# PasswordField

> A secure password input widget featuring obscure masking, eye visibility peek, live entropy strength meter, CapsLock detection, validation checklist, and random password generator.

- **Header File**: `#include "enki/widgets/password_field.hpp"`
- **C++ Class**: `enki::PasswordFieldWidget` (inherits from `enki::StatefulWidget`)
- **Declarative Struct**: `enki::PasswordField` (converts implicitly to `WidgetPtr`)
- **Props Struct**: `enki::PasswordFieldProps`
- **Controller**: `enki::PasswordFieldController`
- **Types**: `enki::PasswordStrengthLevel` (Enum), `enki::PasswordValidationRules` (Struct)

---

## Overview

`PasswordField` is built specifically for secure credential entry in authentication dialogues and sign-up forms. Rather than simply hiding characters, it provides:
1. **Interactive Eye Toggle / Peek**: Click to reveal password or hold-to-peek.
2. **Live Strength Meter**: Real-time entropy computation (`calculateEntropy()`) evaluating length, casing, digits, and symbols.
3. **Criteria Checklist**: Visual checklist dynamically marking rules (e.g. "8+ chars", "1 number", "1 symbol").
4. **CapsLock Warning**: Built-in amber warning banner if CapsLock is detected active.
5. **Secure Generator**: One-click generation of 16-character high-entropy passwords.

---

## C++ API Definition

### Controller (`PasswordFieldController`)
```cpp
namespace enki {

enum class PasswordStrengthLevel {
    Empty,       ///< No text entered
    VeryWeak,    ///< Red (< 25 entropy)
    Weak,        ///< Orange (25-45 entropy)
    Medium,      ///< Yellow (45-65 entropy)
    Strong,      ///< Green (65-85 entropy)
    VeryStrong   ///< Emerald (> 85 entropy)
};

class PasswordFieldController {
public:
    PasswordFieldController(std::string initial_password = "", PasswordValidationRules rules = {});

    [[nodiscard]] const std::string& getPassword() const;
    void setPassword(std::string_view p);

    [[nodiscard]] bool isObscured() const;
    void setObscured(bool obscured);
    void toggleObscured();

    [[nodiscard]] PasswordStrengthLevel calculateStrength() const;
    [[nodiscard]] double calculateEntropy() const;
    [[nodiscard]] bool meetsAllRules() const;

    void generateStrongPassword(size_t length = 16, bool use_symbols = true);
    void clear();
};

} // namespace enki
```

### Declarative Struct (C++20 Designated Initializers)
```cpp
namespace enki {

struct PasswordField {
    Key                                      key                     = Key::none();
    std::shared_ptr<PasswordFieldController> controller              = nullptr;

    std::string                              placeholder             = "Enter password...";
    std::string                              mask_char               = "•";

    // Feature Toggles
    bool                                     show_lock_icon          = true;
    bool                                     show_visibility_toggle  = true;
    bool                                     show_clear_button       = false;
    bool                                     show_generator_button   = false;
    bool                                     show_strength_meter     = false;
    bool                                     show_rules_checklist    = false;
    bool                                     show_capslock_warning   = true;
    bool                                     hold_to_peek            = false;

    bool                                     auto_focus              = false;
    bool                                     read_only               = false;

    // Styling
    TextStyle                                style;
    Color                                    background_color        = 0xFF0F172A;
    Color                                    border_color            = 0xFF334155;
    Color                                    focus_border_color      = 0xFF38BDF8;
    Color                                    icon_color              = 0xFF94A3B8;
    Color                                    placeholder_color       = 0xFF64748B;
    Color                                    cursor_color            = 0xFF38BDF8;
    Color                                    selection_color         = 0x4D38BDF8;
    Color                                    warning_color           = 0xFFF59E0B; // CapsLock

    float                                    border_radius           = 8.0f;
    EdgeInsets                               padding                 = EdgeInsets::symmetric(8.0f, 12.0f);

    // Callbacks
    std::function<void(std::string_view)>               on_changed          = nullptr;
    std::function<void(std::string_view)>               on_submitted        = nullptr;
    std::function<void(PasswordStrengthLevel strength)> on_strength_changed = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

| Property | Type | Default | Description |
|---|---|---|---|
| `key` | `Key` | `Key::none()` | Optional widget identification key. |
| `controller` | `std::shared_ptr<PasswordFieldController>` | `nullptr` | Shared controller managing password text, obscured state, entropy, and rules. |
| `placeholder` | `std::string` | `"Enter password..."` | Watermark text displayed when the field is empty. |
| `mask_char` | `std::string` | `"•"` | Glyph used to mask characters when obscured. |
| `show_lock_icon` | `bool` | `true` | Displays leading lock icon inside the field. |
| `show_visibility_toggle` | `bool` | `true` | Displays trailing eye icon to toggle visibility. |
| `show_clear_button` | `bool` | `false` | Displays a clear button when text is present. |
| `show_generator_button`| `bool` | `false` | Displays a generator button to create a random strong password. |
| `show_strength_meter` | `bool` | `false` | Renders a 5-stage colored progress bar indicating password strength. |
| `show_rules_checklist`| `bool` | `false` | Displays real-time criteria checklist (min length, uppercase, numbers). |
| `show_capslock_warning`| `bool` | `true` | Shows an amber warning alert if CapsLock is active during typing. |
| `hold_to_peek` | `bool` | `false` | If true, only reveals password while mouse button is held down. |
| `auto_focus` | `bool` | `false` | Automatically focuses the password field upon mounting. |
| `read_only` | `bool` | `false` | Disables keyboard editing while retaining selection and visibility toggle. |
| `style` | `TextStyle` | `{}` | Text styling for password text. |
| `background_color` | `Color` | `0xFF0F172A` | Background fill color of the input box. |
| `border_color` | `Color` | `0xFF334155` | Default outline border color. |
| `focus_border_color`| `Color` | `0xFF38BDF8` | Border color when the field is focused. |
| `icon_color` | `Color` | `0xFF94A3B8` | Tint color for lock, eye, and generator icons. |
| `placeholder_color`| `Color` | `0xFF64748B` | Text color for placeholder text. |
| `cursor_color` | `Color` | `0xFF38BDF8` | Blinking caret insertion cursor color. |
| `selection_color` | `Color` | `0x4D38BDF8` | Color of highlighted text selection. |
| `warning_color` | `Color` | `0xFFF59E0B` | Warning color used for CapsLock notification. |
| `border_radius` | `float` | `8.0f` | Corner rounding radius of the field. |
| `padding` | `EdgeInsets` | `symmetric(8, 12)` | Internal content padding. |
| `on_changed` | `std::function<void(std::string_view)>` | `nullptr` | Callback invoked whenever password text changes. |
| `on_submitted` | `std::function<void(std::string_view)>` | `nullptr` | Callback invoked when Enter is pressed. |
| `on_strength_changed`| `std::function<void(PasswordStrengthLevel)>` | `nullptr` | Callback invoked when password strength rating changes. |

---

## Code Examples (From `widgets_demo/password_field_demo/main.cpp`)

### 1. Registration Password Field with Full Security Suite
```cpp
#include "enki/widgets/password_field.hpp"

using namespace enki;

auto signupController = std::make_shared<PasswordFieldController>();

auto registrationPasswordField = PasswordField {
    .controller = signupController,
    .placeholder = "Create a strong password...",
    .show_visibility_toggle = true,
    .show_generator_button = true,
    .show_strength_meter = true,
    .show_rules_checklist = true,
    .show_capslock_warning = true,
    .on_strength_changed = [](PasswordStrengthLevel lvl) {
        // Can be used to enable/disable the submit button
    }
};
```

### 2. Login Password Field with Hold-To-Peek
```cpp
auto loginPasswordField = PasswordField {
    .placeholder = "Enter your password",
    .hold_to_peek = true, // Reveals only while held
    .show_lock_icon = true,
    .show_visibility_toggle = true,
    .on_submitted = [](std::string_view pass) {
        // Trigger login authentication
    }
};
```

---

## See Also
- [**TextField**](./text_field.md) — Standard text input field.
- [**FormField**](./form_field.md) — Form wrapper for validation.
