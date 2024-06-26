#include <latren/graphics/component/meshrenderer.h>
#include <latren/ec/entity.h>
#include <latren/systems.h>
#include <latren/io/resourcemanager.h>
#include <latren/graphics/renderer.h>

#include <limits>

Shader aabbShader = Shader(Shaders::ShaderID::UNLIT);

void MeshRenderer::Start() {
    if (!object->empty()) {
        for (const auto& mesh : Systems::GetResources().modelManager[object].meshes) {
            if (copyMeshes) {
                std::shared_ptr<Mesh> meshCopy = std::make_shared<Mesh>(*mesh);
                meshCopy->material = mesh->material;
                meshCopy->GenerateVAO();
                meshes->push_back(meshCopy);
            }
            else {
                meshes->push_back(mesh);
            }
        }
    }
    glm::vec3 aabbMin = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 aabbMax = glm::vec3(-std::numeric_limits<float>::max());
    for (const auto& mesh : meshes.Get()) {
        glm::vec3 meshMin = mesh->aabb.GetMin();
        glm::vec3 meshMax = mesh->aabb.GetMax();
        aabbMin = glm::vec3(std::min(aabbMin.x, meshMin.x), std::min(aabbMin.y, meshMin.y), std::min(aabbMin.z, meshMin.z));
        aabbMax = glm::vec3(std::max(aabbMax.x, meshMax.x), std::max(aabbMax.y, meshMax.y), std::max(aabbMax.z, meshMax.z));
    }
    aabb_ = ViewFrustum::AABB::FromMinMax(aabbMin, aabbMax);
    Renderable::Start();
}

void MeshRenderer::CalculateMatrices() {
    modelMatrix_ = glm::translate(glm::mat4(1.0f), GetPosition());
    modelMatrix_ *= glm::mat4_cast(parent.GetTransform().rotation.Get());
    modelMatrix_ = glm::scale(modelMatrix_, parent.GetTransform().size.Get());
}


void MeshRenderer::UpdateUniforms(const Shader& shader, const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const glm::mat4& transformMatrix, const glm::vec3& viewPos) const {
    Renderable::UpdateUniforms(shader, projectionMatrix, viewMatrix, viewPos);
    shader.SetUniform("model", modelMatrix_ * transformMatrix);
}

void MeshRenderer::Render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const glm::vec3& viewPos, const Shader* shader, bool aabbDebug) const {
    bool useDefaultShaders = (shader == nullptr);
    for (int i = 0; i < meshes->size(); i++) {
        const std::shared_ptr<Mesh>& mesh = meshes->at(i);
        if (mesh->material == nullptr)
            continue;
        
        if (useDefaultShaders) {
            const Shader& s = GetMaterialShader(mesh->material);
            UpdateUniforms(s, projectionMatrix, viewMatrix, mesh->transformMatrix, viewPos);
            mesh->material->Use(s);
            if (useCustomMaterial && (meshesUsingCustomMaterial->empty() || meshesUsingCustomMaterial->count(i) > 0)) {
                customMaterial->Use(s);
                s.SetUniform("material.hasTexture", mesh->material->GetTexture() != TEXTURE_NONE);
            }
        }
        else {
            UpdateUniforms(*shader, projectionMatrix, viewMatrix, mesh->transformMatrix, viewPos);
            mesh->material->Use(*shader);
        }


        mesh->Bind();
        mesh->Render();

        // unbinding
        glBindVertexArray(0);
    }
    // not gonna optimize this since it's just a debug visualizer
    // yeah, it's pretty horrible that a new meshes gets created every frame
    // well, at least this doesn't cause a memory leak afaik (meshes are also deleted)
    if (aabbDebug) {
        glm::vec3 aabbMin = aabb_.GetMin();
        glm::vec3 aabbMax = aabb_.GetMax();
        Mesh aabbMesh(
            {
                aabbMin.x, aabbMax.y, aabbMax.z,
                aabbMin.x, aabbMin.y, aabbMax.z,
                aabbMax.x, aabbMax.y, aabbMax.z,
                aabbMax.x, aabbMin.y, aabbMax.z,
                aabbMin.x, aabbMax.y, aabbMin.z,
                aabbMin.x, aabbMin.y, aabbMin.z,
                aabbMax.x, aabbMax.y, aabbMin.z,
                aabbMax.x, aabbMin.y, aabbMin.z
            },
            {
                3, 2, 0, 1, 3, 0,
                7, 6, 2, 3, 7, 2,
                5, 4, 6, 7, 5, 6,
                1, 0, 4, 5, 1, 4,
                6, 4, 0, 2, 6, 0,
                1, 5, 7, 1, 7, 3,
            }
        );
        aabbMesh.GenerateVAO();

        aabbShader.Use();
        glBindVertexArray(aabbMesh.vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, aabbMesh.ebo);

        aabbShader.SetUniform("projection", projectionMatrix);
        aabbShader.SetUniform("view", viewMatrix);
        aabbShader.SetUniform("model", modelMatrix_);
        aabbShader.SetUniform("viewPos", Systems::GetRenderer().GetCamera().pos);
        aabbShader.SetUniform("time", (float) GetTime());

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        aabbShader.SetUniform("material.color", glm::vec3(1.0f, 0.0f, 0.0f));
        aabbShader.SetUniform("material.opacity", 1.0f);
        aabbMesh.Render();

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

bool MeshRenderer::IsOnFrustum(const ViewFrustum& frustum) const {    
    glm::vec3 center = modelMatrix_  * glm::vec4(aabb_.center, 1.0f);

    // Scaled orientation
    glm::vec3 right = modelMatrix_[0] * aabb_.extents.x;
    glm::vec3 up = modelMatrix_[1] * aabb_.extents.y;
    glm::vec3 forward = -modelMatrix_[2] * aabb_.extents.z;

    glm::vec3 extents;

    extents.x =
        std::abs(glm::dot(glm::vec3{ 1.0f, 0.0f, 0.0f }, right)) +
        std::abs(glm::dot(glm::vec3{ 1.0f, 0.0f, 0.0f }, up)) +
        std::abs(glm::dot(glm::vec3{ 1.0f, 0.0f, 0.0f }, forward));

    extents.y =
        std::abs(glm::dot(glm::vec3{ 0.0f, 1.0f, 0.0f }, right)) +
        std::abs(glm::dot(glm::vec3{ 0.0f, 1.0f, 0.0f }, up)) +
        std::abs(glm::dot(glm::vec3{ 0.0f, 1.0f, 0.0f }, forward));

    extents.z =
        std::abs(glm::dot(glm::vec3{ 0.0f, 0.0f, 1.0f }, right)) +
        std::abs(glm::dot(glm::vec3{ 0.0f, 0.0f, 1.0f }, up)) +
        std::abs(glm::dot(glm::vec3{ 0.0f, 0.0f, 1.0f }, forward));

    //We not need to divise scale because it's based on the half extention of the AABB
    ViewFrustum::AABB newAABB { center, extents };
    return frustum.IsOnFrustum(newAABB);
}

const ViewFrustum::AABB& MeshRenderer::GetAABB() const {
    return aabb_;
}