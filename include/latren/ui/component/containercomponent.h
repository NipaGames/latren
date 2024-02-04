#pragma once

#include "uicomponent.h"

namespace UI {
    // basically the <div>-equivelant with display: block
    class ContainerComponent : public UIComponent, public Canvas {
    using UIComponent::UIComponent;
    protected:
        std::vector<UI::UIComponent*> children_;
    public:        
        LATREN_API virtual ~ContainerComponent();
        LATREN_API void Render(const glm::mat4&) override;
        LATREN_API void Update() override;
        LATREN_API void UpdateWindowSize() override;
        LATREN_API glm::vec2 GetOffset() const override;
        LATREN_API void ClearChildren();
        LATREN_API void DeleteChildAtIndex(size_t);
        LATREN_API void AddChild(UI::UIComponent*);
        LATREN_API size_t GetChildCount();
        LATREN_API Rect GetLocalBounds() const override;
    };
    REGISTER_COMPONENT(ContainerComponent);
};