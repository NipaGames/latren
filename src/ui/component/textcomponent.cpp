#include <latren/ui/component/textcomponent.h>
#include <latren/game.h>

#include <algorithm>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

using namespace UI;

void TextComponent::Delete() {
    if (renderingMethod_ == TextRenderingMethod::RENDER_TO_TEXTURE) {
        if (fbo_ != NULL)
            glDeleteFramebuffers(1, &fbo_);
        if (texture_ != NULL)
            glDeleteTextures(1, &texture_);
    }
    fbo_ = NULL;
    texture_ = NULL;
}

void TextComponent::Start() {
    renderingMethod_ = renderingMethod;
    shape_.numVertexAttributes = 4;
    shape_.stride = 4;
    shape_.GenerateVAO();

    hasStarted_ = true;
    if (renderingMethod_ == TextRenderingMethod::RENDER_TO_TEXTURE) {
        glGenFramebuffers(1, &fbo_);
        glGenTextures(1, &texture_);
        RenderTexture();
    }
}

void TextComponent::UpdateBounds() {
    glm::ivec2 windowSize;
    glfwGetWindowSize(Game::GetGameInstanceBase()->GetGameWindow().GetWindow(), &windowSize.x, &windowSize.y);

    auto& f = Game::GetGameInstanceBase()->GetResources().fontManager.Get(font);
    float fontModifier = (float) BASE_FONT_SIZE / f.size.y;
    const UITransform& trans = GetTransform();
    glm::vec2 pos = trans.pos;
    float size = trans.size;

    if (renderingMethod_ == TextRenderingMethod::RENDER_EVERY_FRAME) {
        float w = Text::GetTextWidth(f, text_) * size;
        float h = Text::GetTextHeight(f, text_) * size;
        switch (horizontalAlignment) {
            case HorizontalAlignment::RIGHT:
                pos.x -= w * size;
                break;
            case HorizontalAlignment::CENTER:
                pos.x -= w / 2.0f;
                break;
        }
        bounds_ = { pos.x, pos.x + w, pos.y + h, pos.y };
        generalBounds_ = { pos.x, pos.x + w, pos.y + (f.padding[1] - f.padding[0]) * fontModifier * size, pos.y - f.padding[0] * fontModifier * size };
    }
    else if (renderingMethod_ == TextRenderingMethod::RENDER_TO_TEXTURE) {
        float w = textSize_.x * aspectRatioModifier_;
        float h = textSize_.y;
        float adtRowHeight = anchorRowsOver ? 0.0f : additionalRowsHeight_;
        float marginY = (adtRowHeight + padding_[0] * fontModifier) * size;

        glm::vec2 offset(0.0f, -marginY);
        switch (horizontalAlignment) {
            case HorizontalAlignment::LEFT:
                offset.x = 0.0f;
                break;
            case HorizontalAlignment::RIGHT:
                offset.x = (float) -w;
                break;
            case HorizontalAlignment::CENTER:
                // todo: check for anchorRowsOver
                offset.x = -w / 2.0f;
                break;
        }
        switch (verticalAlignment) {
            case VerticalAlignment::BOTTOM:
                offset.y = -marginY;
                break;
            case VerticalAlignment::TOP:
                offset.y -= (f.padding[1] - f.padding[0]) * fontModifier * size;
                break;
            case VerticalAlignment::MIDDLE:
                offset.y -= ((f.padding[1] - f.padding[0]) * fontModifier * size) / 2.0f;
                break;
        }
        bounds_ = { pos.x + offset.x, pos.x + offset.x + w, pos.y + offset.y + h, pos.y + offset.y };
        generalBounds_ = { pos.x + offset.x, pos.x + offset.x + w, pos.y + (f.padding[1] - f.padding[0]) * fontModifier * size, pos.y - f.padding[0] * fontModifier * size };
    }
}

void TextComponent::SetShader(const Shader& s) {
    shader_ = s;
    if (renderingMethod_ == TextRenderingMethod::RENDER_TO_TEXTURE && hasStarted_)
        RenderTexture();
}

void TextComponent::Render(const glm::mat4& proj) {
    if (color.w == 0.0f)
        return;
    shader_.Use();
    shader_.SetUniform("textColor", color);
    shader_.SetUniform("projection", proj);
    shader_.SetUniform("time", (float) glfwGetTime());
    float size = GetTransform().size;

    auto& f = Game::GetGameInstanceBase()->GetResources().fontManager.Get(font);
    float fontModifier = (float) BASE_FONT_SIZE / f.size.y;
    UpdateBounds();
    if (renderingMethod_ == TextRenderingMethod::RENDER_EVERY_FRAME) {
        Text::RenderText(f, text_, glm::vec2(bounds_.left, bounds_.top), size * fontModifier, aspectRatioModifier_, horizontalAlignment, lineSpacing * size);
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
    }
}

void TextComponent::RenderTexture() {
    auto& f = Game::GetGameInstanceBase()->GetResources().fontManager.Get(font);

    glm::ivec2 windowSize;
    glfwGetWindowSize(Game::GetGameInstanceBase()->GetGameWindow().GetWindow(), &windowSize.x, &windowSize.y);
    float fontModifier = ((float) BASE_FONT_SIZE / f.size.y);
    float size = GetTransform().size;
    glm::vec2 wndRatio = (glm::vec2) windowSize / glm::vec2(1280.0f, 720.0f);
    textSize_ = glm::vec2(Text::GetTextWidth(f, text_), Text::GetTextHeight(f, text_, (int) lineSpacing) + padding_[0] * fontModifier) * size;
    if (forceTextSize.x != -1)
        textSize_.x = forceTextSize.x;
    if (forceTextSize.y != -1)
        textSize_.y = forceTextSize.y;
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

    Text::RenderText(f, text_, glm::vec2(0, (additionalRowsHeight_ + padding_[0] * fontModifier) * size) + textOffset_, size * fontModifier, 1.0f, horizontalAlignment, lineSpacing * size);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    float modifier = (16.0f * windowSize.y) / (9.0f * windowSize.x);
    actualTextSize_ = { textSize_.x * modifier, textSize_.y };
}

void TextComponent::SetText(const std::string& t) {
    if (text_ == t)
        return;
    text_ = t;
    additionalRows_ = (int) std::count(text_.begin(), text_.end(), '\n');

    auto& f = Game::GetGameInstanceBase()->GetResources().fontManager.Get(font);
    padding_ = Text::GetVerticalPadding(f, text_);

    additionalRowsHeight_ = additionalRows_ * (f.fontHeight + lineSpacing);
    if (additionalRowsHeight_ > 0) {
        additionalRowsHeight_ += padding_[0];
    }
    if (renderingMethod_ == TextRenderingMethod::RENDER_TO_TEXTURE && hasStarted_) {
        RenderTexture();
    }
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
    if (renderingMethod_ == TextRenderingMethod::RENDER_TO_TEXTURE && hasStarted_) {
        RenderTexture();
    }
}