#include <latren/ui/component/textinputcomponent.h>

using namespace UI;

void TextInputComponent::Delete() {
    if (specialKeyEvent_ != -1)
        Game::GetGameInstanceBase()->GetGameWindow().keyboardEventHandler.Unsubscribe(Input::KeyboardEventType::TEXT_INPUT_SPECIAL, specialKeyEvent_);
    if (asciiKeyEvent_ != -1)
        Game::GetGameInstanceBase()->GetGameWindow().keyboardEventHandler.Unsubscribe(Input::KeyboardEventType::TEXT_INPUT_ASCII_CHAR, asciiKeyEvent_);
}

void TextInputComponent::SetValue(const std::string& val) {
    blinkStart_ = (float) glfwGetTime();
    if (maxLength != -1 && val.length() > maxLength)
        return;
    value = val;
    float w = Text::GetTextWidth(Game::GetGameInstanceBase()->GetResources().fontManager.Get(font), val) * GetTransform().size;
    if (forceTextSize.x != -1) {
        overflow_ = (w > forceTextSize.x);
        if (overflow_) {
            textOffset_.x = forceTextSize.x - w;
        }
        else {
            textOffset_.x = 0.0f;
        }
    }
    SetText(val);
    textWidth_ = Text::GetTextWidth(Game::GetGameInstanceBase()->GetResources().fontManager.Get(font), value) * GetTransform().size;
}

void TextInputComponent::Focus() {
    inputFocus_ = true;
    UpdateBounds();
    SetValue(value);
}

void TextInputComponent::Unfocus() {
    inputFocus_ = false;
}

void TextInputComponent::Start() {
    TextButtonComponent::Start();
    caretShape_ = Shapes::RECTANGLE_VEC4;

    specialKeyEvent_ = Game::GetGameInstanceBase()->GetGameWindow().keyboardEventHandler.Subscribe(Input::KeyboardEventType::TEXT_INPUT_SPECIAL, [&](Input::KeyboardEvent e) {
        if (!inputFocus_)
            return;
        if (e.special == Input::SpecialKey::BACKSPACE) {
            std::string val = value;
            if (Input::IsKeyDown(GLFW_KEY_LEFT_CONTROL) || Input::IsKeyDown(GLFW_KEY_RIGHT_CONTROL))
                SetValue("");
            else if (val.length() > 0)
                SetValue(val.substr(0, val.length() - 1));
            else
                SetValue("");
        }
        if (e.special == Input::SpecialKey::ENTER) {
            eventHandler.Dispatch("enter");
        }
    });
    asciiKeyEvent_ = Game::GetGameInstanceBase()->GetGameWindow().keyboardEventHandler.Subscribe(Input::KeyboardEventType::TEXT_INPUT_ASCII_CHAR, [&](Input::KeyboardEvent e) {
        if (!inputFocus_)
            return;
        SetValue(value + e.character);
    });
    eventHandler.Subscribe("click", [&] {
        Focus();
    });
}

void TextInputComponent::UIUpdate(const Canvas& c) {
    TextButtonComponent::UIUpdate(c);
    if (!isActive) {
        Unfocus();
        return;
    }
    if (Input::IsMouseButtonPressedDown(GLFW_MOUSE_BUTTON_1) && !IsHoveredOver()) {
        Unfocus();
    }
}

void TextInputComponent::Render(const Canvas& c) {
    TextButtonComponent::Render(c);
    if (inputFocus_) {
        caretMaterial->Use();
        caretMaterial->GetShader().SetUniform("time", (float) glfwGetTime() - blinkStart_);
        caretMaterial->GetShader().SetUniform("projection", c.GetProjectionMatrix());
        Rect bounds = c.FromLocalBounds(GetLocalBounds());
        float caretTop = bounds.top + caretOffset.y;
        float caretBottom = bounds.bottom + caretOffset.y;

        float caretPos = bounds.left + textWidth_ * aspectRatioModifier_;
        if (forceTextSize.x != -1 && overflow_) {
            caretPos = bounds.right;
        }
        float caretLeft = caretPos + caretOffset.x;
        float caretRight = caretPos + caretWidth * aspectRatioModifier_ + caretOffset.x;

        float vertices[] = {
            // pos      // texCoords
            caretLeft, caretTop,     0.0f, 0.0f,
            caretLeft, caretBottom,  0.0f, 1.0f,
            caretRight, caretBottom,  1.0f, 1.0f,
            caretLeft, caretTop,     0.0f, 0.0f,
            caretRight, caretBottom,  1.0f, 1.0f,
            caretRight, caretTop,     1.0f, 0.0f
        };
        caretShape_.SetVertexData(vertices);
        caretShape_.Bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}