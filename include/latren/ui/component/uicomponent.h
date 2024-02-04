#pragma once

#include <latren/defines/opengl.h>

#include "../canvas.h"
#include <latren/entity/component.h>
#include <latren/entity/transform.h>
#include <latren/event/eventhandler.h>

namespace UI {
    struct UITransform {
        glm::vec2 pos;
        float size;
    };
    struct Rect {
        float left, right, top, bottom;
    };
    enum class UITransformFrom {
        ENTITY_TRANSFORM_2D,
        UI_TRANSFORM
    };
    class UIComponent : public Component<UIComponent> {
    friend class Canvas;
    protected:
        Canvas* canvas_ = nullptr;
        int priority_ = 0;
        bool isAddedToCanvas_ = false;
        float aspectRatioModifier_ = 1.0f;
    public:
        UITransform transform { glm::vec2(0.0f), 1.0f };
        UITransformFrom transformFrom = UITransformFrom::ENTITY_TRANSFORM_2D;
        EventHandler<std::string> eventHandler;
        bool isActive = true;
        bool isVisible = true;
        LATREN_API UIComponent(Canvas*, int = 0);
        LATREN_API UIComponent() = default;
        LATREN_API virtual ~UIComponent();
        LATREN_API UITransform GetTransform() const;
        LATREN_API virtual void SetTransform(const UITransform&);
        LATREN_API virtual void AddToCanvas();
        LATREN_API virtual void AddToCanvas(Canvas*);
        virtual void Render(const glm::mat4&) { }
        virtual void UpdateWindowSize() { }
        virtual Rect GetLocalBounds() const { return Rect{ }; }
        LATREN_API Rect GetBounds() const;
    };
};