#include <latren/ui/component/textcomponent.h>
#include <latren/systems.h>
#include <latren/io/resourcemanager.h>
#include <latren/gamewindow.h>
#include <latren/debugmacros.h>
#include <algorithm>
#include <functional>

using namespace UI;

void TextComponent::Delete() {
    if (renderingMethod_ == TextRenderingMethod::RENDER_TO_TEXTURE) {
        if (fbo_ != GL_NONE)
            glDeleteFramebuffers(1, &fbo_);
        if (texture_ != GL_NONE)
            glDeleteTextures(1, &texture_);
    }
    fbo_ = GL_NONE;
    texture_ = GL_NONE;
}

void TextComponent::Start() {
    renderingMethod_ = renderingMethod;
    #ifdef LATREN_FORCE_TEXT_RENDER_EVERY_FRAME
    renderingMethod_ = TextRenderingMethod::RENDER_EVERY_FRAME;
    #endif
    #ifdef LATREN_FORCE_TEXT_RENDER_TO_TEXTURE
    renderingMethod_ = TextRenderingMethod::RENDER_TO_TEXTURE;
    #endif
    shape_.numVertexAttributes = 4;
    shape_.stride = 4;
    shape_.GenerateVAO();

    Systems::GetGameWindow().eventHandler.Subscribe(WindowEventType::WINDOW_RESIZE,
        LambdaByReference([](TextComponent& c, const glm::ivec2& size) {
            c.UpdateWindowSize(size);
        })
    );
    UpdateWindowSize(Systems::GetGameWindow().GetSize());

    hasStarted_ = true;
    UpdateTextMetrics();
    CalculateBounds();
    if (renderingMethod_ == TextRenderingMethod::RENDER_TO_TEXTURE) {
        glGenFramebuffers(1, &fbo_);
        glGenTextures(1, &texture_);
        RenderTexture();
    }
    UIComponent::Start();
}

void TextComponent::CalculateBounds() {
    auto& f = Systems::GetResources().GetFontManager()->Get(font);
    float fontModifier = f.GetSizeModifier();
    const UITransform& trans = GetTransform();
    glm::vec2 pos = trans.pos;
    float size = trans.size;

    if (forceTextSize.x != -1)
        actualTextSize_.x = std::min(textSize_.x * aspectRatioModifier_, forceTextSize.x);
    else
        actualTextSize_.x = textSize_.x * aspectRatioModifier_;
    if (forceTextSize.y != -1)
        actualTextSize_.y = std::min(textSize_.y, forceTextSize.y);
    else
        actualTextSize_.y = textSize_.y;

    glm::vec2 offset = glm::vec2(0.0f);

    switch (horizontalAlignment) {
        case HorizontalAlignment::LEFT:
            offset.x = 0.0f;
            break;
        case HorizontalAlignment::RIGHT:
            offset.x = (float) -actualTextSize_.x;
            break;
        case HorizontalAlignment::CENTER:
            offset.x = -actualTextSize_.x / 2.0f;
            break;
    }
    switch (verticalAlignment) {
        case VerticalAlignment::BOTTOM:
            offset.y = 0.0f;
            break;
        case VerticalAlignment::TOP:
            offset.y = -(f.baseLine.fromGlyphTop - f.baseLine.fromGlyphBottom) * fontModifier * size;
            break;
        case VerticalAlignment::MIDDLE:
            offset.y = -((f.baseLine.fromGlyphTop - f.baseLine.fromGlyphBottom) * fontModifier * size) / 2.0f;
            break;
    }

    // yeah...
    float adtRowHeight = anchorRowsOver ? 0.0f : additionalRowsHeight_;
    float boundsOffsetY = offset.y - (adtRowHeight + baseLine_.fromGlyphBottom) * size;
    bounds_ = {
        pos.x + offset.x,
        pos.x + offset.x + actualTextSize_.x,
        pos.y + boundsOffsetY + actualTextSize_.y,
        pos.y + boundsOffsetY
    };
    float generalBoundsOffsetY = offset.y - (adtRowHeight + f.baseLine.fromGlyphBottom * fontModifier) * size;
    generalBounds_ = {
        pos.x + offset.x,
        pos.x + offset.x + ((forceTextSize.x != -1) ? forceTextSize.x : actualTextSize_.x),
        pos.y + generalBoundsOffsetY + ((forceTextSize.y != -1) ? forceTextSize.y : fixedTextHeight_),
        pos.y + generalBoundsOffsetY
    };
}

void TextComponent::RenderTextToPos(glm::vec2 pos) {
    float size = GetTransform().size;
    auto& f = Systems::GetResources().GetFontManager()->Get(font);
    float fontModifier = f.GetSizeModifier();

    pos.y += (additionalRowsHeight_ + baseLine_.fromGlyphBottom) * size;
    pos += textOffset_;
    Text::RenderText(f, text_, pos, size * fontModifier, aspectRatioModifier_, horizontalAlignment, lineSpacing * size);
}

void TextComponent::SetShader(const Shader& s) {
    shader_ = s;
    if (renderingMethod_ == TextRenderingMethod::RENDER_TO_TEXTURE && hasStarted_)
        RenderTexture();
}

void TextComponent::Render(const glm::mat4& proj) {
    UIComponent::Render(proj);
    if (color.w == 0.0f)
        return;
    shader_.Use();
    shader_.SetUniform("textColor", color);
    shader_.SetUniform("projection", proj);
    shader_.SetUniform("time", (float) GetTime());

    CalculateBounds();
    if (renderingMethod_ == TextRenderingMethod::RENDER_EVERY_FRAME) {
        bool scissor = (forceTextSize.x != -1 || forceTextSize.y != -1);
        if (scissor) {
            glm::vec2 wndRatio = (glm::vec2) Systems::GetGameWindow().GetSize() / glm::vec2(1280.0f, 720.0f);
            glEnable(GL_SCISSOR_TEST);
            glm::vec2 scissorPos = glm::vec2(generalBounds_.left, generalBounds_.bottom) * wndRatio;
            glm::vec2 scissorSize = forceTextSize * wndRatio;
            glScissor((GLint) scissorPos.x, (GLint) scissorPos.y, (GLint) scissorSize.x, (GLint) scissorSize.y);
        }
        RenderTextToPos(glm::vec2(bounds_.left, bounds_.bottom));
        if (scissor)
            glDisable(GL_SCISSOR_TEST);
    }
    else if (renderingMethod_ == TextRenderingMethod::RENDER_TO_TEXTURE) {
        shape_.SetVertexData(new float[24] {
            bounds_.left,   bounds_.top,        0.0f, 1.0f,
            bounds_.left,   bounds_.bottom,     0.0f, 0.0f,
            bounds_.right,  bounds_.bottom,     1.0f, 0.0f,

            bounds_.left,   bounds_.top,        0.0f, 1.0f,
            bounds_.right,  bounds_.bottom,     1.0f, 0.0f,
            bounds_.right,  bounds_.top,        1.0f, 1.0f
        }, false);
        shape_.Bind();

        glBindTexture(GL_TEXTURE_2D, texture_);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

        #ifdef LATREN_DEBUG_TEXT_TEXTURES
        auto& f = Systems::GetResources().GetFontManager()->Get(font);
        Shader s = Shader(Shaders::ShaderID::UI_TEXT);
        s.Use();
        s.SetUniform("textColor", glm::vec4(glm::vec3(1.0f) - glm::vec3(color), 1.0f));
        s.SetUniform("projection", proj);
        Text::RenderText(f, fmt::format("{}x{}", textureSize_.x, textureSize_.y), glm::vec2(bounds_.left, bounds_.bottom), .35f * f.GetSizeModifier(), aspectRatioModifier_, HorizontalAlignment::LEFT, lineSpacing * .35f);
        #endif
    }
}

void TextComponent::RenderTexture() {
    auto& f = Systems::GetResources().GetFontManager()->Get(font);

    glm::vec2 wndRatio = (glm::vec2) Systems::GetGameWindow().GetSize() / glm::vec2(1280.0f, 720.0f);
    glm::ivec2 texSize = actualTextSize_ * wndRatio;
    textureSize_ = texSize;
    glViewport(0, 0, (int) texSize.x, texSize.y);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    GLfloat borderColor[4] = { 0, 0, 0, 1 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, texSize.x, texSize.y, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glClearTexImage(texture_, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture_, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    
    // finally drawing
    textureShader_.Use();
    textureShader_.SetUniform("textColor", glm::vec4(1.0f));
    textureShader_.SetUniform("projection", glm::ortho(0.0f, texSize.x / wndRatio.x, 0.0f, texSize.y / wndRatio.y));
    #ifdef LATREN_DEBUG_TEXT_TEXTURES
    glClearColor(0.5f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    #endif
    RenderTextToPos(glm::vec2(0.0f));
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    Systems::GetRenderer().RestoreViewport();
}

void TextComponent::UpdateTextMetrics() {
    auto& f = Systems::GetResources().GetFontManager()->Get(font);
    additionalRows_ = (int) std::count(text_.begin(), text_.end(), '\n');
    baseLine_ = Text::GetBaseLine(f, text_);
    additionalRowsHeight_ = additionalRows_ * (f.fontHeight * f.GetSizeModifier() + lineSpacing);
    textSize_.x = Text::GetTextWidth(f, text_) * GetTransform().size;
    textSize_.y = Text::GetTextHeight(f, text_, (int) lineSpacing) * GetTransform().size;
    fixedTextHeight_ = Text::GetFixedTextHeight(f, text_, (int) lineSpacing) * GetTransform().size;
}

void TextComponent::SetText(const std::string& t) {
    if (text_ == t)
        return;
    text_ = t;
    UpdateTextMetrics();
    CalculateBounds();
    if (renderingMethod_ == TextRenderingMethod::RENDER_TO_TEXTURE && hasStarted_)
        RenderTexture();
}

const std::string& TextComponent::GetText() const {
    return text_;
}

const glm::vec2& TextComponent::GetTextSize() const {
    return actualTextSize_;
}

const UI::Rect& TextComponent::GetActualTextBounds() const {
    return bounds_;
}

UI::Rect TextComponent::GetLocalBounds() const {
    return generalBounds_;
}

void TextComponent::UpdateWindowSize(const glm::ivec2& size) {
    aspectRatioModifier_ = (16.0f * size.y) / (9.0f * size.x);
    CalculateBounds();
    if (renderingMethod_ == TextRenderingMethod::RENDER_TO_TEXTURE && hasStarted_)
        RenderTexture();
}