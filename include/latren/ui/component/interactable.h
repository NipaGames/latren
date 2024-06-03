#pragma once

#include "uicomponent.h"

namespace UI {
    class LATREN_API Interactable : public UISubcomponent {
    private:
        bool isSelected_ = false;
    public:
        void Update(UIComponent*) override;
    };

    template <typename T>
    class ButtonComponent : public T {
    public:
        ButtonComponent() {
            UIComponent::AddSubcomponent<Interactable>();
        }
    };
};