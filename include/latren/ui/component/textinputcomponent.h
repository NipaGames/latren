#pragma once

#include "buttoncomponent.h"
#include "../materials.h"

namespace UI {
    class TextInputComponent : public TextButtonComponent, RegisterComponent<TextInputComponent> {
    using TextButtonComponent::TextButtonComponent;
    private:
        bool inputFocus_ = false;
        bool overflow_ = false;
        float caretPos_;
        Shape caretShape_;
        float blinkStart_;
        EventID specialKeyEvent_ = -1;
        EventID asciiKeyEvent_ = -1;
    public:
        glm::vec2 caretOffset = glm::vec2(0.0f);
        float caretWidth = 3.0f;
        std::shared_ptr<Material> caretMaterial = DEFAULE_CARET_MATERIAL;
        int maxLength = -1;
        std::string value;
        LATREN_API virtual ~TextInputComponent();
        LATREN_API void Focus();
        LATREN_API void Unfocus();
        LATREN_API void Start();
        LATREN_API void Update();
        LATREN_API void SetValue(const std::string&);
        LATREN_API void UpdateCaretPos();
        LATREN_API void Render(const glm::mat4&);
        LATREN_API void UpdateWindowSize();
    };
};