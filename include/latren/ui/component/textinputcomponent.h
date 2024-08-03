#pragma once

#include "textcomponent.h"
#include "interactable.h"
#include "imagecomponent.h"
#include "subcomponent.h"
#include "../materials.h"

namespace UI {
    class LATREN_API TextInputComponent : public TextComponent {
    private:
        bool inputFocus_ = false;
        bool overflow_ = false;
        float textWidth_ = 0.0f;
        Shape caretShape_;
        float blinkStart_;
        EventID specialKeyEvent_ = -1;
        EventID asciiKeyEvent_ = -1;
    public:
        glm::vec2 caretOffset = glm::vec2(0.0f);
        float caretWidth = 3.0f;
        std::shared_ptr<Material> caretMaterial = DEFAULT_CARET_MATERIAL;
        int maxLength = -1;
        std::string value;
        TextInputComponent();
        void Focus();
        void Unfocus();
        void Start();
        void Delete();
        void UIUpdate();
        void SetValue(const std::string&);
        void Render(const glm::mat4&);
        void UpdateCaretAndOffsetPos();
        Image& GetBackground();
    };
};