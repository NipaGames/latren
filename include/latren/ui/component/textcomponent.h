#pragma once

#include <latren/defines/opengl.h>

#include "uicomponent.h"
#include "subcomponent.h"
#include "../text.h"
#include "../canvas.h"
#include "../alignment.h"
#include <latren/graphics/component/meshrenderer.h>
#include <latren/graphics/shape.h>

namespace UI {
    enum class TextRenderingMethod {
        // Render each character every time separately (1 quad/character).
        RENDER_EVERY_FRAME,
        // Group textcomponent's text's into a texture (1 quad/textcomponent).
        // Can slightly increase performance but more buggier and text rendering anyway is seldom the bottleneck.
        // However, can also be used to run cool shaders on the whole text instead of each letter.
        RENDER_TO_TEXTURE
    };

    class LATREN_API TextComponent : public UIComponent {
    protected:
        Shader shader_ = Shader(Shaders::ShaderID::UI_TEXT);
        Shader textureShader_ = Shader(Shaders::ShaderID::UI_TEXT);
        std::string text_ = "";
        GLuint fbo_ = GL_NONE;
        GLuint texture_ = GL_NONE;
        glm::vec2 textSize_ = glm::vec2(0.0f);
        glm::vec2 actualTextSize_ = glm::vec2(0.0f);
        UI::Text::BaseLine baseLine_;
        int additionalRows_ = 0;
        float additionalRowsHeight_ = 0;
        TextRenderingMethod renderingMethod_;
        bool hasStarted_ = false;
        Shape shape_;
        UI::Rect bounds_;
        UI::Rect generalBounds_;
        glm::vec2 textOffset_ = glm::vec2(0.0f);
        void RenderTexture();
        void RenderTextToPos(glm::vec2);
    public:
        std::string font;
        float lineSpacing = 5.0f;
        bool anchorRowsOver = false;
        glm::vec2 forceTextSize = glm::vec2(-1);
        glm::vec4 color = glm::vec4(1.0f);
        TextRenderingMethod renderingMethod = TextRenderingMethod::RENDER_EVERY_FRAME; // Must be declared before start, no effect otherwise
        HorizontalAlignment horizontalAlignment = HorizontalAlignment::LEFT;
        VerticalAlignment verticalAlignment = VerticalAlignment::BOTTOM;
        float textureResolutionModifier = 1.0f;
        
        void Start() override;
        void Delete() override;
        void Render(const glm::mat4&) override;
        void UpdateWindowSize() override;
        void SetText(const std::string&);
        void SetShader(const Shader&);
        void CalculateBounds();
        void UpdateTextMetrics();
        const std::string& GetText() const;
        const glm::vec2& GetTextSize() const;
        // get the "actual" text bounds (the ones that start from the actual first rendered pixels), variable height and bottom
        const Rect& GetActualTextBounds() const;
        // get the text bounds fixed to the font size, const height and bottom
        Rect GetLocalBounds() const override;
    };
};