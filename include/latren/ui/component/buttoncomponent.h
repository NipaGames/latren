#pragma once

#include "textcomponent.h"
#include <latren/game.h>
#include <latren/input.h>

namespace UI {
    template <typename T>
    class ButtonComponent : public T {
    using T::T;
    protected:
        bool prevHovered_ = false;
        bool isHoveredOver_ = false;
    public:
        void Update() {
            T::Update();
            if (!isActive)
                return;
            const glm::vec2& mousePos = Game::GetGameInstanceBase()->GetGameWindow().GetRelativeMousePosition();
            const UI::Rect& rect = GetBounds();
            isHoveredOver_ = (mousePos.x > rect.left && mousePos.x < rect.right && mousePos.y > rect.bottom && mousePos.y < rect.top);
            if (isHoveredOver_) {
                if (!prevHovered_) {
                    eventHandler.Dispatch("mouseEnter");
                }
                prevHovered_ = true;
                if (Input::IsMouseButtonPressedDown(GLFW_MOUSE_BUTTON_1)) {
                    eventHandler.Dispatch("click");
                }
            }
            else {
                if (prevHovered_) {
                    eventHandler.Dispatch("mouseLeave");
                }
                prevHovered_ = false;
            }
        }
        bool IsHoveredOver() { return isHoveredOver_; }
    };
    
    class TextButtonComponent : public ButtonComponent<TextComponent> {
    using ButtonComponent::ButtonComponent;
    protected:
        Shape bgShape_;
    public:
        std::shared_ptr<Material> bgMaterial = nullptr;
        
        LATREN_API void Start();
        LATREN_API void Render(const glm::mat4&);
    };
};