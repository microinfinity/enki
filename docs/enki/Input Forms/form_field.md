# FormField & Form Inputs

> Form-bound input fields (`TextFormField`, `CheckboxFormField`) integrated with the composable `Validators` library and automatic live error text rendering.

- **Header File**: `#include "enki/widgets/form.hpp"`
- **C++ Classes**: `enki::TextFormFieldWidget`, `enki::CheckboxFormFieldWidget`
- **Declarative Structs**: `enki::TextFormField`, `enki::CheckboxFormField` (convert implicitly to `WidgetPtr`)
- **Validation Combinators**: `enki::Validators`
- **Callback Signature**: `using ValidatorFn = std::function<std::optional<std::string>(const std::string& value)>`

---

## Overview

While standard inputs (`TextField`, `Checkbox`) handle basic interaction, **FormFields** wrap them with:
1. **Form Registration**: Automatically binds to a parent `FormState` for unified validation.
2. **Validator Functions**: Returns `std::nullopt` on success or an error message string on failure.
3. **Error Presentation**: Highlights the field border in red and displays the error message underneath.
4. **Data Extraction**: Calls `on_saved()` with final validated values.

---

## Composable Validators Library (`Validators`)

Enki provides standard, chainable validator functions:

| Validator | Signature | Description |
|---|---|---|
| `required` | `Validators::required(msg)` | Rejects empty or whitespace-only input. |
| `email` | `Validators::email(msg)` | Validates email syntax via regular expressions. |
| `minLength` | `Validators::minLength(n, msg)` | Ensures string length is at least `n` characters. |
| `match` | `Validators::match(targetFn, msg)` | Checks if value equals another field (e.g. Confirm Password). |
| `compose` | `Validators::compose({v1, v2, ...})` | Evaluates rules sequentially; stops on the first error. |

---

## C++ API Definition

### `TextFormField` Declarative Struct
```cpp
namespace enki {

struct TextFormField {
    Key                                     key           = Key::none();
    std::string                             label;
    std::string                             hint;
    std::string                             initial_value;
    std::string                             helper_text;
    bool                                    required      = false;
    bool                                    obscure_text  = false;
    float                                   width         = 340.0f;

    std::shared_ptr<FormState>              form_state    = nullptr;
    std::shared_ptr<TextFieldController>    controller    = nullptr;
    ValidatorFn                             validator     = nullptr;
    std::function<void(const std::string&)> on_saved      = nullptr;
    std::function<void(const std::string&)> on_changed    = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

### `CheckboxFormField` Declarative Struct
```cpp
namespace enki {

struct CheckboxFormField {
    Key                                                   key           = Key::none();
    std::string                                           label;
    bool                                                  initial_value = false;
    bool                                                  required      = false;

    std::shared_ptr<FormState>                            form_state    = nullptr;
    std::function<std::optional<std::string>(bool value)> validator     = nullptr;
    std::function<void(bool)>                             on_saved      = nullptr;
    std::function<void(bool)>                             on_changed    = nullptr;

    operator WidgetPtr() const;
};

} // namespace enki
```

---

## Properties Reference

### `TextFormField` Properties

| Property | Type | Default | Description |
|---|---|---|---|
| `key` | `Key` | `Key::none()` | Optional widget identification key. |
| `label` | `std::string` | `""` | Label text displayed above the input field. |
| `hint` | `std::string` | `""` | Placeholder text when the field is empty. |
| `initial_value` | `std::string` | `""` | Initial text value if no controller is provided. |
| `helper_text` | `std::string` | `""` | Subtitle helper text shown when there are no validation errors. |
| `required` | `bool` | `false` | Marks the field visually and behaviorally as mandatory. |
| `obscure_text` | `bool` | `false` | Masks entered characters for passwords or sensitive data. |
| `width` | `float` | `340.0f` | Total width of the input field. |
| `form_state` | `std::shared_ptr<FormState>` | `nullptr` | Parent form coordinator to register validation with. |
| `controller` | `std::shared_ptr<TextFieldController>` | `nullptr` | Controller for programmatic reading/writing of text. |
| `validator` | `ValidatorFn` | `nullptr` | Validation rule returning error message or `std::nullopt`. |
| `on_saved` | `std::function<void(const std::string&)>` | `nullptr` | Callback executed when `form_state->save()` is called. |
| `on_changed` | `std::function<void(const std::string&)>` | `nullptr` | Callback invoked immediately when text changes. |

### `CheckboxFormField` Properties

| Property | Type | Default | Description |
|---|---|---|---|
| `key` | `Key` | `Key::none()` | Optional widget identification key. |
| `label` | `std::string` | `""` | Label text next to the checkbox. |
| `initial_value` | `bool` | `false` | Initial check state if no external state is bound. |
| `required` | `bool` | `false` | Marks checkbox as required (e.g. Terms & Conditions). |
| `form_state` | `std::shared_ptr<FormState>` | `nullptr` | Parent form coordinator to register validation with. |
| `validator` | `std::function<std::optional<std::string>(bool)>` | `nullptr` | Custom boolean validation callback. |
| `on_saved` | `std::function<void(bool)>` | `nullptr` | Callback executed when `form_state->save()` is called. |
| `on_changed` | `std::function<void(bool)>` | `nullptr` | Callback invoked immediately when checkbox is toggled. |

---

## Code Examples (From `widgets_demo/form_demo/main.cpp`)

### 1. Email Field with Composed Rules
```cpp
#include "enki/widgets/form.hpp"

using namespace enki;

auto emailField = TextFormField {
    .label = "Corporate Email",
    .hint = "name@company.com",
    .required = true,
    .form_state = formState,
    .validator = Validators::compose({
        Validators::required("Email is mandatory"),
        Validators::email("Please enter a valid format")
    }),
    .on_saved = [](const std::string& email) {
        // Save email to model
    }
};
```

### 2. Confirm Password Match Validation
```cpp
auto passCtrl = std::make_shared<TextFieldController>();

auto confirmPassField = TextFormField {
    .label = "Confirm Password",
    .obscure_text = true,
    .form_state = formState,
    .validator = Validators::compose({
        Validators::required("Please confirm password"),
        Validators::match([passCtrl]() { return passCtrl->text; }, "Passwords do not match")
    }),
};
```

### 3. Required Checkbox Agreement
```cpp
auto termsField = CheckboxFormField {
    .label = "I agree to the End User License Agreement",
    .required = true,
    .form_state = formState,
    .validator = [](bool checked) -> std::optional<std::string> {
        if (!checked) return "You must accept the terms to proceed";
        return std::nullopt;
    }
};
```

---

## See Also
- [**Form**](./form.md) — The parent form orchestrator.
- [**TextField**](./text_field.md) — The underlying text input engine.
- [**Checkbox**](./checkbox.md) — The underlying binary checkbox.
