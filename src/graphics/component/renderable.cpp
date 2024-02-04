#include <latren/graphics/component/renderable.h>
#include <latren/game.h>

Renderable::~Renderable() {
    if (isAssignedToRenderer_)
        Game::GetGameInstanceBase()->GetRenderer().RemoveRenderable(this);
}

void Renderable::Start() {
    if (assignToRenderer && !isAssignedToRenderer_) {
        Game::GetGameInstanceBase()->GetRenderer().AddRenderable(this);
    }
}

void Renderable::FirstUpdate() {
    if (isStatic)
        CalculateMatrices();
}

void Renderable::UseMaterial(const std::shared_ptr<Material>& mat) const {
    mat->Use();
}

void Renderable::UpdateUniforms(const Shader& shader, const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix) const {
    shader.Use();
    // default uniforms
    // idk if it would be better to define these as default initializers for Material class
    shader.SetUniform("material.color", glm::vec3(1.0f));
    shader.SetUniform("material.opacity", 1.0f);
    shader.SetUniform("material.fog.use", false);

    shader.SetUniform("projection", projectionMatrix);
    shader.SetUniform("view", viewMatrix);
    shader.SetUniform("viewPos", Game::GetGameInstanceBase()->GetRenderer().GetCamera().pos);
    shader.SetUniform("time", (float) glfwGetTime());
}

const Shader& Renderable::GetMaterialShader(const std::shared_ptr<Material>& mat) const {
    return mat->GetShader();
}

void Renderable::IRender(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const Shader* shader, bool aabbDebug) const {
    if (disableDepthTest)
        glDisable(GL_DEPTH_TEST);
    Render(projectionMatrix, viewMatrix, shader, aabbDebug);
    glBindVertexArray(0);
    if (disableDepthTest)
        glEnable(GL_DEPTH_TEST);
}

bool Renderable::IsOnFrustum(const ViewFrustum& frustum) const {
    return true;
}