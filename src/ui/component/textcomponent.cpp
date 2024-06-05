#include <latren/ui/component/textcomponent.h>
#include <latren/systems.h>
#include <latren/io/resourcemanager.h>
#include <latren/gamewindow.h>
#include <algorithm>

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
    shape_.numVertexAttributes = 4;
    shape_.stride = 4;
    shape_.GenerateVAO();

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
    auto& f = Systems::GetResources().fontManager.Get(font);
    float fontModifier = (float) BASE_FONT_SIZE / f.size.y;
    const UITransform& trans = GetTransform();
    glm::vec2 pos = trans.pos;
    float size = trans.size;

    if (forceTextSize.x != -1)
        actualTextSize_.x = textSize_.x = forceTextSize.x;
    else
        actualTextSize_.x = textSize_.x * aspectRatioModifier_;
    if (forceTextSize.y != -1)
        textSize_.y = forceTextSize.y;
    actualTextSize_.y = textSize_.y;

    float adtRowHeight = anchorRowsOver ? 0.0f : additionalRowsHeight_;
    float marginY = (adtRowHeight + padding_[0] * fontModifier) * size;
    glm::vec2 offset = glm::vec2(0.0f);

    switch (horizontalAlignment) {
        case HorizontalAlignment::LEFT:
            offset.x = 0.0f;
            break;
        case HorizontalAlignment::RIGHT:
            offset.x = (float) -actualTextSize_.x;
            break;
        case HorizontalAlignment::CENTER:
            // todo: check for anchorRowsOver
            offset.x = -actualTextSize_.x / 2.0f;
            break;
    }
    switch (verticalAlignment) {
        case VerticalAlignment::BOTTOM:
            offset.y = 0.0f;
            break;
        case VerticalAlignment::TOP:
            offset.y = -(f.padding[1] - f.padding[0]) * fontModifier * size;
            break;
        case VerticalAlignment::MIDDLE:
            offset.y = -((f.padding[1] - f.padding[0]) * fontModifier * size) / 2.0f;
            break;
    }
    if (renderingMethod_ == TextRenderingMethod::RENDER_EVERY_FRAME) {
        offset.y -= actualTextSize_.y - (additionalRowsHeight_ - adtRowHeight) * fontModifier * size;
    }
    else if (renderingMethod_ == TextRenderingMethod::RENDER_TO_TEXTURE) {
        offset.y -= marginY;
    }
    bounds_ = { pos.x + offset.x, pos.x + offset.x + actualTextSize_.x, pos.y + offset.y + actualTextSize_.y, pos.y + offset.y };
    generalBounds_ = { pos.x + offset.x, pos.x + offset.x + actualTextSize_.x, pos.y + (f.padding[1] - f.padding[0]) * fontModifier * size, pos.y - f.padding[0] * fontModifier * size };
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
    const UITransform& trans = GetTransform();
    float size = trans.size;

    auto& f = Systems::GetResources().fontManager.Get(font);
    float fontModifier = (float) BASE_FONT_SIZE / f.size.y;
    CalculateBounds();
    if (renderingMethod_ == TextRenderingMethod::RENDER_EVERY_FRAME) {
        bool scissor = (forceTextSize.x != -1 || forceTextSize.y != -1);
        if (scissor) {
            glm::vec2 wndRatio = (glm::vec2) Systems::GetGameWindow().GetWindowSize() / glm::vec2(1280.0f, 720.0f);
            glEnable(GL_SCISSOR_TEST);
            // good enough
            glm::vec2 scissorPos = glm::vec2(bounds_.left, (bounds_.top - (f.padding[0]) * fontModifier * size)) * wndRatio;
            glm::vec2 scissorSize = forceTextSize * wndRatio;
            glScissor(scissorPos.x, scissorPos.y, scissorSize.x, scissorSize.y);
        }
        Text::RenderText(f, text_, glm::vec2(bounds_.left, bounds_.top) + textOffset_, size * fontModifier, aspectRatioModifier_, horizontalAlignment, lineSpacing * size);
        if (scissor)
            glDisable(GL_SCISSOR_TEST);
    }
    else if (renderingMethod_ == TextRenderingMethod::RENDER_TO_TEXTURE) {
        float rightBound = bounds_.left + textSize_.x * aspectRatioModifier_; 
        shape_.SetVertexData(new float[24] {
            bounds_.left,   bounds_.top,        0.0f, 1.0f,
            bounds_.left,   bounds_.bottom,     0.0f, 0.0f,
            rightBound,     bounds_.bottom,     1.0f, 0.0f,

            bounds_.left,   bounds_.top,        0.0f, 1.0f,
            rightBound,     bounds_.bottom,     1.0f, 0.0f,
            rightBound,     bounds_.top,        1.0f, 1.0f
        }, false);
        shape_.Bind();

        glBindTexture(GL_TEXTURE_2D, texture_);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void TextComponent::RenderTexture() {
    auto& f = Systems::GetResources().fontManager.Get(font);

    glm::vec2 wndRatio = (glm::vec2) Systems::GetGameWindow().GetWindowSize() / glm::vec2(1280.0f, 720.0f);
    float fontModifier = ((float) BASE_FONT_SIZE / f.size.y);
    float size = GetTransform().size;
    glm::ivec2 texSize = (glm::ivec2) glm::ceil(wndRatio * (glm::vec2) textSize_);
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
    textureShader_.SetUniform("projection", glm::ortho(0.0f, 1280.0f, 0.0f, 720.0f));

    Text::RenderText(f, text_, glm::vec2(0, (additionalRowsHeight_ + padding_[0] * fontModifier) * size) + textOffset_ * wndRatio.x, size * fontModifier, 1.0f, horizontalAlignment, lineSpacing * size);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TextComponent::UpdateTextMetrics() {
    auto& f = Systems::GetResources().fontManager.Get(font);
    additionalRows_ = (int) std::count(text_.begin(), text_.end(), '\n');
    padding_ = Text::GetVerticalPadding(f, text_);
    additionalRowsHeight_ = additionalRows_ * (f.fontHeight + lineSpacing);
    float fontModifier = ((float) BASE_FONT_SIZE / f.size.y);
    if (forceTextSize.x == -1)
        textSize_.x = Text::GetTextWidth(f, text_) * GetTransform().size;
    if (forceTextSize.y == -1)
        textSize_.y = (Text::GetTextHeight(f, text_, (int) lineSpacing) + padding_[0] * fontModifier) * GetTransform().size;
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

void TextComponent::UpdateWindowSize() {
    CalculateBounds();
    if (renderingMethod_ == TextRenderingMethod::RENDER_TO_TEXTURE && hasStarted_)
        RenderTexture();
}