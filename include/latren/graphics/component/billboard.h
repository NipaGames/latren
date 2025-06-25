#pragma once

#include "renderable.h"
#include "../shaders.h"
#include "../material.h"

class  BillboardRenderer : public Renderable<BillboardRenderer> {
private:
    mutable glm::mat4 modelMatrix_;
    GLuint vao_ = GL_NONE;
    GLuint vbo_ = GL_NONE;
    GLsizei pointsCount_ = 0;
    inline static Shader SHADER_ = Shaders::ShaderID::BILLBOARD;
public:
    SERIALIZABLE(std::vector<glm::vec3>, positions);
    SERIALIZABLE(std::shared_ptr<Material>, material);

    virtual void Start();
    virtual void Delete();

    virtual void UpdateVertexBuffer();
    virtual void CalculateMatrices();
    virtual void UpdateUniforms(const Shader&, const glm::mat4&, const glm::mat4&, const glm::vec3&) const override;
    virtual void Render(const glm::mat4&, const glm::mat4&, const glm::vec3&, const Shader* = nullptr, int = RENDER_MODE_NORMAL) const;
};