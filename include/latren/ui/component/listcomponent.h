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
        ListComponent() : ContainerComponent() { bgOverflow = false; }

        LATREN_API void Start() override;
        LATREN_API void Delete() override;
        template <typename T, typename = std::enable_if_t<std::is_base_of_v<UIComponent, T>>>
        void AddListItem(const SharedComponentPtr<T>& c) {
            c->Get().transform.pos.y = GetComponentCount() * -itemSpacing + scrollPos_;
            AddUIComponent(SharedComponentPtrCast<UIComponent>(c));
        }
        LATREN_API void SetScrollPos(float);
    };

    class TextListComponent : public ListComponent {
    using ListComponent::ListComponent;
    public:
        HorizontalAlignment itemAlignment = HorizontalAlignment::CENTER;
        LATREN_API void AddListItem(const std::string&);
    };
};