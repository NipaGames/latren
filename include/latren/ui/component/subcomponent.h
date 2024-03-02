#pragma once

#include <latren/defines/opengl.h>

namespace UI {
    class UIComponent;
    class UISubcomponent {
    public:
        virtual void Start(UIComponent*) { }
        virtual void Update(UIComponent*) { }
        virtual void Render(UIComponent*, const glm::mat4&) { }
    };
}