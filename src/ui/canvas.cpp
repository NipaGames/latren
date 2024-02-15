#include <latren/ui/canvas.h>
#include <latren/ui/component/uicomponent.h>
#include <latren/game.h>

using namespace UI;

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
    
    for (auto& [p, components] : components_) {
        for (ComponentReference<UIComponent>& c : components) {
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

void Canvas::AddUIComponent(ComponentReference<UIComponent> c, int priority) {
    components_[priority].push_back(c);
}

void Canvas::UpdateComponentsOnWindowSize(float m) {
    for (auto& [p, components] : components_) {
        for (GeneralComponentReference& c : components) {
            c.CastComponent<UI::UIComponent>().aspectRatioModifier_ = m;
            c.CastComponent<UI::UIComponent>().UpdateWindowSize();
        }
    }
}

void Canvas::UpdateWindowSize(int w, int h) {
    float aspectRatioModifier = ((float) h * 1280.0f) / ((float) w * 720.0f);
    UpdateComponentsOnWindowSize(aspectRatioModifier);
}

void Canvas::RemoveUIComponent(const ComponentReference<UIComponent>& c) {
    for (auto& [p, components] : components_) {
        const auto it = std::find(components.begin(), components.end(), c);
        if (it == components.end())
            continue;
        components.erase(it);
        return;
    }
}