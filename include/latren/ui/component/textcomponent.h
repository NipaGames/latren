#pragma once

#include <latren/defines/opengl.h>

#include "uicomponent.h"
#include "../text.h"
#include "../canvas.h"
#include "../alignment.h"
#include <latren/ec/component.h>
#include <latren/graphics/component/meshrenderer.h>
#include <latren/graphics/shape.h>

namespace UI {
    enum class TextRenderingMethod {
        RENDER_EVERY_FRAME,
        RENDER_TO_TEXTURE
    };

    class TextComponent : public UIComponent, RegisterComponent<TextComponent> {
    using UIComponent::UIComponent;
    protected:
        Shader shader_ = Shader(Shaders::ShaderID::UI_TEXT);
        Shader textureShader_ = Shader(Shaders::ShaderID::UI_TEXT);
        std::string text_ = "";
        GLuint fbo_ = NULL;
        GLuint texture_ = NULL;
        glm::vec2 textSize_ = glm::vec2(0.0f);
        glm::vec2 actualTextSize_ = glm::vec2(0.0f);
        glm::vec2 padding_ = glm::vec2(0.0f);
        int additionalRows_ = 0;
        float additionalRowsHeight_ = 0;
        TextRenderingMethod renderingMethod_;
        bool hasStarted_ = false;
        Shape shape_;
        UI::Rect bounds_;
        UI::Rect generalBounds_;
        glm::vec2 textOffset_ = glm::vec2(0.0f);
        LATREN_API void RenderTexture();
    public:
        std::string font;
        float lineSpacing = 5.0f;
        bool anchorRowsOver = false;
        glm::vec2 forceTextSize = glm::vec2(-1);
        glm::vec4 color = glm::vec4(1.0f);
        TextRenderingMethod renderingMethod = TextRenderingMethod::RENDER_TO_TEXTURE; // Must be declared before start, no effect otherwise
        HorizontalAlignment horizontalAlignment = HorizontalAlignment::LEFT;
        VerticalAlignment verticalAlignment = VerticalAlignment::BOTTOM;
        float textureResolutionModifier = 1.0f;
        
        LATREN_API void Start() override;
        LATREN_API void Delete() override;
        LATREN_API void Render(const Canvas&) override;
        LATREN_API void UpdateWindowSize() override;
        LATREN_API void SetText(const std::string&);
        LATREN_API void SetShader(const Shader&);
        LATREN_API void UpdateBounds();
        LATREN_API const std::string& GetText() const;
        LATREN_API const glm::vec2& GetTextSize() const;
        // get the "actual" text bounds (the ones that start from the actual first rendered pixels), variable height and bottom
        LATREN_API const UI::Rect& GetActualTextBounds() const;
        // get the text bounds fixed to the font size, const height and bottom
        LATREN_API UI::Rect GetLocalBounds() const override;
    };
};