#pragma once

#include <latren/defines/opengl.h>

#include <latren/ec/component.h>
#include <latren/ec/transform.h>
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
    class Canvas;
    class UIComponent : public Component<UIComponent> {
    friend class Canvas;
    protected:
        float aspectRatioModifier_ = 1.0f;
        bool renderGlobally_ = true;
    public:
        UITransform transform { glm::vec2(0.0f), 1.0f };
        UITransformFrom transformFrom = UITransformFrom::ENTITY_TRANSFORM_2D;
        EventHandler<std::string> eventHandler;
        bool isActive = true;
        bool isVisible = true;
        LATREN_API UITransform GetTransform() const;
        LATREN_API virtual void SetTransform(const UITransform&);
        virtual void Render(const Canvas&) { }
        virtual void UIUpdate(const Canvas&) { }
        virtual void UpdateWindowSize() { }
        virtual Rect GetLocalBounds() const { return Rect{ }; }
        bool ShouldRenderGlobally() const { return renderGlobally_; }
        LATREN_API Rect GetBounds() const;
    };
};