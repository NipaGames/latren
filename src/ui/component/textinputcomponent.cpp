#include <latren/ui/component/textinputcomponent.h>
#include <latren/systems.h>
#include <latren/input.h>
#include <latren/gamewindow.h>
#include <latren/io/resourcemanager.h>

using namespace UI;

TextInputComponent::TextInputComponent() {
    AddSubcomponent<Interactable>();
    AddSubcomponent<Image>();
}

void TextInputComponent::Delete() {
    if (specialKeyEvent_ != -1)
        Systems::GetGameWindow().keyboardEventHandler.Unsubscribe(Input::KeyboardEventType::TEXT_INPUT_SPECIAL, specialKeyEvent_);
    if (asciiKeyEvent_ != -1)
        Systems::GetGameWindow().keyboardEventHandler.Unsubscribe(Input::KeyboardEventType::TEXT_INPUT_ASCII_CHAR, asciiKeyEvent_);
}

void TextInputComponent::SetValue(const std::string& val) {
    blinkStart_ = (float) GetTime();
    if (maxLength != -1 && val.length() > maxLength)
        return;
    value = val;
    float w = Text::GetTextWidth(Systems::GetResources().fontManager.Get(font), val) * GetTransform().size;
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
    textWidth_ = Text::GetTextWidth(Systems::GetResources().fontManager.Get(font), value) * GetTransform().size;
}

void TextInputComponent::Focus() {
    inputFocus_ = true;
    CalculateBounds();
    SetValue(value);
}

void TextInputComponent::Unfocus() {
    inputFocus_ = false;
}

void TextInputComponent::Start() {
    TextComponent::Start();
    caretShape_ = Shapes::GetDefaultShape(Shapes::DefaultShape::RECTANGLE_VEC4);

    specialKeyEvent_ = Systems::GetGameWindow().keyboardEventHandler.Subscribe(Input::KeyboardEventType::TEXT_INPUT_SPECIAL, [&](Input::KeyboardEvent e) {
        if (!inputFocus_)
            return;
        if (e.special == Input::SpecialKey::BACKSPACE) {
            std::string val = value;
            if (Systems::GetInputSystem().keyboardListener.IsDown(GLFW_KEY_LEFT_CONTROL) || Systems::GetInputSystem().keyboardListener.IsDown(GLFW_KEY_RIGHT_CONTROL))
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
    asciiKeyEvent_ = Systems::GetGameWindow().keyboardEventHandler.Subscribe(Input::KeyboardEventType::TEXT_INPUT_ASCII_CHAR, [&](Input::KeyboardEvent e) {
        if (!inputFocus_)
            return;
        SetValue(value + e.character);
    });
    eventHandler.Subscribe("mouseUp", [&] {
        Focus();
    });
}

void TextInputComponent::UIUpdate() {
    TextComponent::UIUpdate();
    if (!isActive) {
        Unfocus();
        return;
    }
    if (Systems::GetInputSystem().mouseButtonListener.IsPressedDown(GLFW_MOUSE_BUTTON_1) && !GetInteractionState().isHoveredOver) {
        Unfocus();
    }
}

void TextInputComponent::Render(const glm::mat4& proj) {
    TextComponent::Render(proj);
    if (inputFocus_) {
        caretMaterial->Use();
        caretMaterial->GetShader().SetUniform("time", (float) GetTime() - blinkStart_);
        caretMaterial->GetShader().SetUniform("projection", proj);
        Rect bounds = GetBounds();
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

Image& TextInputComponent::GetBackground() {
    return GetSubcomponent<Image>(1);
}