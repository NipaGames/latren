#include <latren/ui/component/imagecomponent.h>
#include <latren/ui/materials.h>
#include <latren/ui/canvas.h>

using namespace UI;

void Image::Start(UIComponent* c) {
    quadShape_ = Shapes::GetDefaultShape(Shapes::DefaultShape::RECTANGLE_VEC4);
    float texLeft = (float) flipHorizontally;
    float texRight = (float) !flipHorizontally;
    float texTop = (float) flipVertically;
    float texBottom = (float) !flipVertically;
    const Rect& rect = c->GetBounds();
    float vertices[] = {
        // pos                      // texCoords
        rect.left,  rect.top,       texLeft,   texTop,
        rect.left,  rect.bottom,    texLeft,   texBottom,
        rect.right, rect.bottom,    texRight,  texBottom,
        rect.left,  rect.top,       texLeft,   texTop,
        rect.right, rect.bottom,    texRight,  texBottom,
        rect.right, rect.top,       texRight,  texTop
    };
    quadShape_.SetVertexData(vertices);
}

void Image::Render(UIComponent*, const glm::mat4& proj) {
    material->Use();
    material->GetShader().SetUniform("projection", proj);
    if (texture != TEXTURE_NONE) {
        material->GetShader().SetUniform("material.hasTexture", true);
        glBindTexture(GL_TEXTURE_2D, texture);
    }
    quadShape_.Bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
}