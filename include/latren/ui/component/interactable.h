#pragma once

#include "uicomponent.h"

namespace UI {
    class  Interactable : public UISubcomponent {
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