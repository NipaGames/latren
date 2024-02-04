#pragma once

#include <latren/ui/text.h>
#include "containercomponent.h"

namespace UI {
    class ListComponent : public ContainerComponent {
    private:
        EventID scrollEvent_ = -1;
        float scrollPos_ = 0.0f;
    public:
        float scrollSensitivity = 20.0f;
        float itemSpacing = 30.0f;
        LATREN_API virtual ~ListComponent();
        ListComponent() : ContainerComponent() { bgOverflow = false; }
        ListComponent(UI::Canvas* c, int p = 0) : ContainerComponent(c, p) { bgOverflow = false; }

        LATREN_API void Start() override;
        LATREN_API void AddListItem(UI::UIComponent*);
        LATREN_API void SetScrollPos(float);
    };
    REGISTER_COMPONENT(ListComponent);

    class TextListComponent : public ListComponent {
    using ListComponent::ListComponent;
    public:
        HorizontalAlignment itemAlignment = HorizontalAlignment::CENTER;
        LATREN_API void AddListItem(const std::string&);
    };
    REGISTER_COMPONENT(TextListComponent);
};