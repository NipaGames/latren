#pragma once

#include "uicomponent.h"
#include "../materials.h"
#include <latren/graphics/shape.h>

namespace UI {
    class ImageComponent : public UIComponent, RegisterComponent<ImageComponent> {
    using UIComponent::UIComponent;
    private:
        Shape quadShape_;
    public:
        std::shared_ptr<Material> material = SOLID_UI_SHAPE_MATERIAL;
        Texture::TextureID texture = TEXTURE_NONE;
        // this has to be specified before start
        // since images don't usually move i'm not writing a dynamic setter yet
        Rect rect;
        // these too
        bool flipHorizontally = false;
        bool flipVertically = false;

        LATREN_API virtual ~ImageComponent();
        LATREN_API void Start();
        LATREN_API void Render(const glm::mat4&);
    };
};
