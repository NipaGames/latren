#pragma once

#include "uicomponent.h"

namespace UI {
    class Interactable : public UISubcomponent {
    public:
        LATREN_API void Update(UIComponent*) override;
    };

    template <typename T>
    class ButtonComponent : public T {
    public:
        ButtonComponent() {
            AddSubcomponent<Interactable>();
        }
    };
};