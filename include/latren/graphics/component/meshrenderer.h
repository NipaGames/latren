#pragma once

#include "renderable.h"
#include "../mesh.h"
#include "../object.h"

#include <latren/defines/opengl.h>
#include <memory>

class MeshRenderer : public Renderable<MeshRenderer> {
friend class Renderer;
private:
    mutable glm::mat4 modelMatrix_;
    ViewFrustum::AABB aabb_;
    bool isAssignedToRenderer_ = false;
public:
    // enable this if the object transform doesn't update, no need to calculate model matrices every frame that way
    std::string object = ""; LE_RCDV(object)
    std::vector<std::shared_ptr<Mesh>> meshes; LE_RCDV(meshes)
    bool copyMeshes = false; LE_RCDV(copyMeshes)

    LATREN_API virtual void CalculateMatrices();
    LATREN_API virtual void UpdateUniforms(const Shader&, const glm::mat4&, const glm::mat4&, const glm::mat4&, const glm::vec3&) const;

    LATREN_API virtual void Start();
    LATREN_API virtual bool IsOnFrustum(const ViewFrustum&) const;
    LATREN_API virtual void Render(const glm::mat4&, const glm::mat4&, const glm::vec3&, const Shader* = nullptr, bool = false) const;
    const ViewFrustum::AABB& GetAABB() const { return aabb_; }
};