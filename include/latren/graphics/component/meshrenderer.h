#pragma once

#include "renderable.h"
#include "../mesh.h"
#include "../object.h"

#include <latren/defines/opengl.h>
#include <memory>

class LATREN_API MeshRenderer : public Renderable<MeshRenderer> {
friend class Renderer;
private:
    mutable glm::mat4 modelMatrix_;
    ViewFrustum::AABB aabb_;
    bool isAssignedToRenderer_ = false;
public:
    // enable this if the object transform doesn't update, no need to calculate model matrices every frame that way
    SERIALIZABLE(std::string, object) = { };
    SERIALIZABLE(std::vector<std::shared_ptr<Mesh>>, meshes);
    SERIALIZABLE(bool, copyMeshes) = false;

    void CalculateMatrices() override;
    virtual void UpdateUniforms(const Shader&, const glm::mat4&, const glm::mat4&, const glm::mat4&, const glm::vec3&) const;

    void Start() override;
    bool IsOnFrustum(const ViewFrustum&) const override;
    void Render(const glm::mat4&, const glm::mat4&, const glm::vec3&, const Shader* = nullptr, int = RENDER_MODE_NORMAL) const override;
    const ViewFrustum::AABB& GetAABB() const;
};