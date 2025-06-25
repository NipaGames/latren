#pragma once

#include "uicomponent.h"
#include "../canvas.h"

namespace UI {
    // basically the <div>-equivelant
    class  ContainerComponent : public UIComponent, public Canvas {
    using UIComponent::UIComponent;
    public:
        void Render(const glm::mat4&) override;
        void UIUpdate() override;
        glm::vec2 GetOffset() const override;
        Rect GetLocalBounds() const override;
    };
};