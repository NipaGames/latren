#include <latren/ui/component/textinputcomponent.h>

using namespace UI;

void TextInputComponent::Delete() {
    if (specialKeyEvent_ != -1)
        Game::GetGameInstanceBase()->GetGameWindow().keyboardEventHandler.Unsubscribe(Input::KeyboardEventType::TEXT_INPUT_SPECIAL, specialKeyEvent_);
    if (asciiKeyEvent_ != -1)
        Game::GetGameInstanceBase()->GetGameWindow().keyboardEventHandler.Unsubscribe(Input::KeyboardEventType::TEXT_INPUT_ASCII_CHAR, asciiKeyEvent_);
}

void TextInputComponent::UpdateCaretPos() {
    float w = Text::GetTextWidth(Game::GetGameInstanceBase()->GetResources().fontManager.Get(font), value) * GetTransform().size;
    caretPos_ = GetBounds().left + w * aspectRatioModifier_;
    if (forceTextSize.x != -1 && overflow_) {
        caretPos_ = GetBounds().right;
    }
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
    UpdateCaretPos();
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
    caretPos_ = GetTransform().pos.x;
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

void TextInputComponent::Update() {
    TextButtonComponent::Update();
    if (!isActive) {
        Unfocus();
        return;
    }
    if (Input::IsMouseButtonPressedDown(GLFW_MOUSE_BUTTON_1) && !IsHoveredOver()) {
        Unfocus();
    }
}

void TextInputComponent::Render(const glm::mat4& proj) {
    TextButtonComponent::Render(proj);
    if (inputFocus_) {
        caretMaterial->Use();
        caretMaterial->GetShader().SetUniform("time", (float) glfwGetTime() - blinkStart_);
        caretMaterial->GetShader().SetUniform("projection", proj);
        
        float caretTop = GetBounds().top + caretOffset.y;
        float caretBottom = GetBounds().bottom + caretOffset.y;
        float caretLeft = caretPos_ + caretOffset.x;
        float caretRight = caretPos_ + caretWidth * aspectRatioModifier_ + caretOffset.x;

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

void TextInputComponent::UpdateWindowSize() {
    TextButtonComponent::UpdateWindowSize();
    UpdateCaretPos();
}