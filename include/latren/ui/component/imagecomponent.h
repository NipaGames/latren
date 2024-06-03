#pragma once

#include "uicomponent.h"
#include "../materials.h"
#include <latren/graphics/material.h>
#include <latren/graphics/shape.h>

namespace UI {
    class LATREN_API Image : public UISubcomponent {
    private:
        Shape quadShape_;
    public:
        std::shared_ptr<Material> material = SOLID_UI_SHAPE_MATERIAL;
        Texture::TextureID texture = TEXTURE_NONE;
        bool flipHorizontally = false;
        bool flipVertically = false;

        void Start(UIComponent*) override;
        void Render(UIComponent*, const glm::mat4&) override;
    };

    class ImageComponent : public ExtendedUIComponent<Image> {
    public:
        Image& GetImage() { return GetSubcomponent<Image>(0); }
    };

    class LayeredImageComponent : public ExtendedUIComponent<Image, Image> {
    public:
        Image& GetBackground() { return GetSubcomponent<Image>(0); }
        Image& GetForeground() { return GetSubcomponent<Image>(1); }
        Image& AddLayer() { return AddSubcomponent<Image>(); }
    };
};
