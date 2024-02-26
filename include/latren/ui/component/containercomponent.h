#pragma once

#include "uicomponent.h"
#include "../canvas.h"

namespace UI {
    // basically the <div>-equivelant
    class ContainerComponent : public UIComponent, public Canvas {
    using UIComponent::UIComponent;
    public:
        LATREN_API void Render(const Canvas&) override;
        LATREN_API void UIUpdate(const Canvas&) override;
        LATREN_API void UpdateWindowSize() override;
        LATREN_API glm::vec2 GetOffset() const override;
        LATREN_API Rect GetLocalBounds() const override;
        LATREN_API Rect FromLocalBounds(Rect) const override;
    };
};