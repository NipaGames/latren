#pragma once

#include "renderable.h"
#include "../shaders.h"
#include "../material.h"

class BillboardRenderer : public Renderable<BillboardRenderer> {
private:
    mutable glm::mat4 modelMatrix_;
    GLuint vao_ = NULL;
    GLuint vbo_ = NULL;
    GLsizei pointsCount_ = 0;
    inline static Shader SHADER_ = Shaders::ShaderID::BILLBOARD;
public:
    std::vector<glm::vec3> positions; LE_RCDV(positions)
    std::shared_ptr<Material> material; LE_RCDV(material)

    LATREN_API virtual void Start();
    LATREN_API virtual void Delete();

    LATREN_API virtual void UpdateVertexBuffer();
    LATREN_API virtual void CalculateMatrices();
    LATREN_API virtual void UpdateUniforms(const Shader&, const glm::mat4&, const glm::mat4&, const glm::vec3&) const override;
    LATREN_API virtual void Render(const glm::mat4&, const glm::mat4&, const glm::vec3&, const Shader* = nullptr, bool = false) const;
};