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
    DEFINE_COMPONENT_DATA_VALUE_VECTOR(glm::vec3, positions);
    DEFINE_COMPONENT_DATA_VALUE_DEFAULT(std::shared_ptr<Material>, material);

    LATREN_API virtual ~BillboardRenderer();

    LATREN_API virtual void UpdateVertexBuffer();
    LATREN_API virtual void CalculateMatrices();
    LATREN_API virtual void UpdateUniforms(const Shader&, const glm::mat4&, const glm::mat4&, const glm::vec3&) const override;
    LATREN_API virtual void Start();
    LATREN_API virtual void Render(const glm::mat4&, const glm::mat4&, const glm::vec3&, const Shader* = nullptr, bool = false) const;
};