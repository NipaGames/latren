#include <latren/ui/component/buttoncomponent.h>

using namespace UI;

void TextButtonComponent::Start() {
    TextComponent::Start();
    bgShape_ = Shapes::RECTANGLE_VEC4;
}

void TextButtonComponent::Render(const glm::mat4& proj) {
    if (bgMaterial != nullptr) {
        bgMaterial->Use();
        bgMaterial->GetShader().SetUniform("projection", proj);
        
        Rect bounds = GetBounds();
        float vertices[] = {
            // pos      // texCoords
            bounds.left, bounds.top,     0.0f, 0.0f,
            bounds.left, bounds.bottom,  0.0f, 1.0f,
            bounds.right, bounds.bottom,  1.0f, 1.0f,
            bounds.left, bounds.top,     0.0f, 0.0f,
            bounds.right, bounds.bottom,  1.0f, 1.0f,
            bounds.right, bounds.top,     1.0f, 0.0f
        };
        bgShape_.SetVertexData(vertices);
        bgShape_.Bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    
    TextComponent::Render(proj);
}