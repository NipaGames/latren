#pragma once

#include "textcomponent.h"
#include <latren/game.h>
#include <latren/input.h>

namespace UI {
    template <typename T>
    class ButtonComponent : public T {
    using T::T;
    };
    
    class TextButtonComponent : public ButtonComponent<TextComponent>, RegisterComponent<TextButtonComponent> {
    using ButtonComponent::ButtonComponent;
    protected:
        Shape bgShape_;
    public:
        std::shared_ptr<Material> bgMaterial = nullptr;
        
        LATREN_API void Start();
        LATREN_API void Render(const glm::mat4&);
    };
};