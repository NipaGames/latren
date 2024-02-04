#include <latren/ui/canvas.h>
#include <latren/ui/component/uicomponent.h>
#include <latren/game.h>

using namespace UI;

Canvas::Canvas(Canvas&& other) : 
    bgShape_(other.bgShape_),
    bgMaterial(other.bgMaterial),
    isVisible(other.isVisible),
    offset(other.offset),
    bgSize(other.bgSize),
    bgVerticalAnchor(other.bgVerticalAnchor),
    components_(std::move(other.components_))
{
    for (const auto& [p, components] : components_) {
        for (UIComponent* c : components) {
            c->canvas_ = this;
        }
    }
}

Canvas& Canvas::operator=(Canvas&& other) {
    bgShape_ = other.bgShape_;
    bgMaterial = other.bgMaterial;
    isVisible = other.isVisible;
    offset = other.offset;
    bgSize = other.bgSize;
    bgVerticalAnchor = other.bgVerticalAnchor;
    components_ = std::move(other.components_);
    for (const auto& [p, components] : components_) {
        for (UIComponent* c : components) {
            c->canvas_ = this;
        }
    }
    return *this;
}

Canvas::~Canvas() {
    return;
    for (const auto& [p, components] : components_) {
        for (UIComponent* c : components) {
            if (c->canvas_ == this)
                c->canvas_ = nullptr;
        }
    }
    components_.clear();
}

Canvas* Canvas::GetCanvas() {
    return static_cast<Canvas*>(this);
}

void Canvas::GenerateBackgroundShape() {
    bgShape_ = Shapes::RECTANGLE_VEC4;
}

void Canvas::Draw() {
    if (!isVisible)
        return;
    auto proj = GetProjectionMatrix();
    
    float w = bgSize.x;
    float h = bgSize.y;
    float top = bgVerticalAnchor == CanvasBackgroundVerticalAnchor::OVER ? h : 0;
    float bottom = bgVerticalAnchor == CanvasBackgroundVerticalAnchor::OVER ? 0 : -h;

    if (!bgOverflow) {
        glEnable(GL_SCISSOR_TEST);
        glm::ivec2 wndSize;
        glfwGetWindowSize(Game::GetGameInstanceBase()->GetGameWindow().GetWindow(), &wndSize.x, &wndSize.y);
        glm::vec2 wndSizeFactor = glm::vec2(wndSize) / glm::vec2(1280.0f, 720.0f);
        glm::vec2 pos = GetOffset();
        glm::ivec2 scissorPos = glm::vec2(pos.x, pos.y + bottom) * wndSizeFactor;
        glm::ivec2 scissorSize = glm::vec2(w, h) * wndSizeFactor;
        glScissor(scissorPos.x, scissorPos.y, scissorSize.x, scissorSize.y);
    }

    if (bgMaterial != nullptr) {
        bgMaterial->Use();
        bgMaterial->GetShader().SetUniform("projection", proj);
        float vertices[] = {
            // pos      // texCoords
            0, top,     0.0f, 0.0f,
            0, bottom,  0.0f, 1.0f,
            w, bottom,  1.0f, 1.0f,
            0, top,     0.0f, 0.0f,
            w, bottom,  1.0f, 1.0f,
            w, top,     1.0f, 0.0f
        };
        bgShape_.SetVertexData(vertices);
        bgShape_.Bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    
    for (const auto& [p, components] : components_) {
        for (UIComponent* c : components) {
            if (c->isVisible) {
                if (!bgOverflow)
                    glEnable(GL_SCISSOR_TEST);
                c->Render(proj);
            }
        }
    }
    if (!bgOverflow) {
        glDisable(GL_SCISSOR_TEST);
    }
}

glm::mat4 Canvas::GetProjectionMatrix() const {
    glm::vec2 pos = GetOffset();
    glm::mat4 proj = glm::ortho(
        (float) -pos.x,
        (float) 1280 - pos.x,
        (float) -pos.y,
        (float) 720 - pos.y);
    return proj;
}

glm::vec2 Canvas::GetOffset() const {
    return offset;
}

void Canvas::AddUIComponent(UIComponent* c, int priority) {
    components_[priority].push_back(c);
    c->canvas_ = this;
}

void Canvas::UpdateComponentsOnWindowSize(float m) {
    for (const auto& [p, components] : components_) {
        for (UIComponent* c : components) {
            c->aspectRatioModifier_ = m;
            c->UpdateWindowSize();
        }
    }
}

void Canvas::UpdateWindowSize(int w, int h) {
    float aspectRatioModifier = ((float) h * 1280.0f) / ((float) w * 720.0f);
    UpdateComponentsOnWindowSize(aspectRatioModifier);
}

void Canvas::RemoveUIComponent(const UI::UIComponent* c) {
    for (auto& [p, components] : components_) {
        const auto it = std::find(components.begin(), components.end(), c);
        if (it == components.end())
            continue;
        components.erase(it);
        return;
    }
}