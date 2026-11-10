#include "enki/enki.hpp"
#include <iostream>

using namespace enki;

class FormDemoState : public State {
private:
    std::shared_ptr<FormState> form_state_ = std::make_shared<FormState>();
    std::shared_ptr<TextFieldController> name_ctrl_ = std::make_shared<TextFieldController>();
    std::shared_ptr<TextFieldController> email_ctrl_ = std::make_shared<TextFieldController>();

public:
    WidgetPtr build(BuildContext&) override {
        return container({
            .color = 0xFF0B0F19,
            .align = Alignment::Center,
            .width = StyleValue::percent(100.0f),
            .height = StyleValue::percent(100.0f),
            .child = container({
                .color = 0xFF151C2C,
                .border_radius = BorderRadius::circular(16.0f),
                .border = Border(0xFF26334D, 1.0f),
                .width = StyleValue::point(460.0f),
                .padding = StyleInsets::all(32.0f),
                .child = Form{
                    .child = column({
                        .gap = StyleValue::point(20.0f),
                        .children = {
                            text("Create Account", {
                                .color = 0xFFFFFFFF,
                                .font_size = 24.0f,
                                .font_weight = FontWeight::Bold,
                            }),
                            text("Fill in the fields below to register with ENKI Form validation.", {
                                .color = 0xFF94A3B8,
                                .font_size = 14.0f,
                            }),

                            // Full Name Input
                            TextFormField{
                                .label = "Full Name",
                                .hint = "Enter your display name",
                                .form_state = form_state_,
                                .controller = name_ctrl_,
                                .validator = Validators::compose({
                                    Validators::required("Full name is required"),
                                    Validators::minLength(3, "Must be at least 3 characters"),
                                }),
                            },

                            // Email Input
                            TextFormField{
                                .label = "Email Address",
                                .hint = "user@example.com",
                                .form_state = form_state_,
                                .controller = email_ctrl_,
                                .validator = Validators::compose({
                                    Validators::required("Email is required"),
                                    Validators::email("Please enter a valid email address"),
                                }),
                            },

                            // Submit Button
                            button(
                                text("Submit Form", {
                                    .color = 0xFFFFFFFF,
                                    .font_size = 15.0f,
                                    .font_weight = FontWeight::SemiBold,
                                }),
                                [this]() {
                                    if (form_state_->validate()) {
                                        std::cout << "Form is valid!\n";
                                        std::cout << "Name: " << name_ctrl_->text << "\n";
                                        std::cout << "Email: " << email_ctrl_->text << "\n";
                                    } else {
                                        std::cout << "Validation failed. Please review the errors.\n";
                                    }
                                },
                                ButtonProps{
                                    .normal_color = 0xFF2563EB,
                                    .border_radius = 8.0f,
                                }
                            ),
                        }
                    }),
                    .controller = form_state_,
                }
            })
        });
    }
};

class FormDemoApp : public StatefulWidget {
public:
    std::unique_ptr<State> createState() override {
        return std::make_unique<FormDemoState>();
    }
    std::string_view typeName() const override { return "FormDemoApp"; }
};

int main() {
    AppConfig config;
    config.title = "ENKI Form & Validation";
    config.width = 900;
    config.height = 700;
    return runApp(std::make_shared<FormDemoApp>(), config);
}
