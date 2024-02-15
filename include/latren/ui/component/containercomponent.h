#pragma once

#include "uicomponent.h"
#include "../canvas.h"

namespace UI {
    // basically the <div>-equivelant with display: block
    class ContainerComponent : public UIComponent, public Canvas {
    using UIComponent::UIComponent;
    public:
        LATREN_API void Render(const glm::mat4&) override;
        LATREN_API void Update() override;
        LATREN_API void UpdateWindowSize() override;
        LATREN_API glm::vec2 GetOffset() const override;
        LATREN_API Rect GetLocalBounds() const override;
    };
};