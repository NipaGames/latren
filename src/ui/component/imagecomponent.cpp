#include <latren/ui/component/imagecomponent.h>
#include <latren/ui/materials.h>

using namespace UI;

ImageComponent::~ImageComponent() {
    
}

void ImageComponent::Start() {
    quadShape_ = Shapes::RECTANGLE_VEC4;
    float texLeft = (float) flipHorizontally;
    float texRight = (float) !flipHorizontally;
    float texTop = (float) flipVertically;
    float texBottom = (float) !flipVertically;
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

void ImageComponent::Render(const glm::mat4& proj) {
    material->Use();
    material->GetShader().SetUniform("projection", proj);
    if (texture != TEXTURE_NONE) {
        material->GetShader().SetUniform("material.hasTexture", true);
        glBindTexture(GL_TEXTURE_2D, texture);
    }
    quadShape_.Bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
}