#pragma once

#include <latren/defines/opengl.h>
#include <latren/ec/component.h>
#include <latren/ec/transform.h>
#include <latren/event/eventhandler.h>
#include "subcomponent.h"

namespace UI {
    struct UITransform {
        glm::vec2 pos;
        float size;
    };
    struct Rect {
        float left, right, top, bottom;
        void operator+=(const glm::vec2&);
        Rect operator+(const glm::vec2&) const;
        void operator-=(const glm::vec2&);
        Rect operator-(const glm::vec2&) const;
        operator glm::vec2() const;
    };
    enum class UITransformFrom {
        ENTITY_TRANSFORM_2D,
        UI_TRANSFORM
    };
    struct InteractionState {
        bool prevHovered = false;
        bool isHoveredOver = false;
    };
    class Canvas;
    class Interactable;
    class  UIComponent : public Component<UIComponent> {
    friend class Canvas;
    friend class Interactable;
    protected:
        Canvas* parent_ = nullptr;
        InteractionState interaction_;
        Rect localBounds_;
        std::vector<std::shared_ptr<UISubcomponent>> subcomponents_;
    public:
        UITransform transform { glm::vec2(0.0f), 1.0f };
        UITransformFrom transformFrom = UITransformFrom::ENTITY_TRANSFORM_2D;
        EventHandler<std::string> eventHandler;
        bool isActive = true;
        bool isVisible = true;
        
        UITransform GetTransform() const;
        virtual void SetTransform(const UITransform&);

        virtual void Start() override;
        virtual void Render(const glm::mat4& proj);
        virtual void UIUpdate();

        virtual Rect GetLocalBounds() const;
        // this will only affect certain components
        virtual void SetLocalBounds(const Rect&);
        virtual Rect GetBounds() const;
        virtual const InteractionState& GetInteractionState() const;
        virtual Canvas* GetCanvas() const;

        template <typename T>
        T& AddSubcomponent() {
            subcomponents_.push_back(std::make_unique<T>());
            return static_cast<T&>(*subcomponents_.back());
        }
        template <typename T>
        T& GetSubcomponent(int i) {
            return static_cast<T&>(*subcomponents_.at(i));
        }
    };

    template <typename... SInit>
    class ExtendedUIComponent : public UIComponent {
    public:
        ExtendedUIComponent() {
            ([&] {
                AddSubcomponent<SInit>();
            } (), ...);
        }
    };
};