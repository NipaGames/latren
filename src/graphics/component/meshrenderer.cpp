#include <latren/graphics/component/meshrenderer.h>
#include <latren/ec/entity.h>
#include <latren/systems.h>
#include <latren/io/resourcemanager.h>
#include <latren/graphics/renderer.h>

#include <limits>

Shader DEBUG_AABB_SHADER = Shader(Shaders::ShaderID::LINE);
Shader DEBUG_NORMAL_SHADER = Shader(Shaders::ShaderID::HIGHLIGHT_NORMALS);

void MeshRenderer::Start() {
    if (!object->empty()) {
        for (const auto& mesh : Systems::GetResources().GetModelManager()->Get(object).meshes) {
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
    modelMatrix_ = glm::translate(glm::mat4(1.0f), offset.Get());
    modelMatrix_ *= parent.GetTransform().CreateTransformationMatrix();
}

void MeshRenderer::UpdateUniforms(const Shader& shader, const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const glm::mat4& transformMatrix, const glm::vec3& viewPos) const {
    Renderable::UpdateUniforms(shader, projectionMatrix, viewMatrix, viewPos);
    shader.SetUniform("model", modelMatrix_ * transformMatrix);
}

void MeshRenderer::Render(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const glm::vec3& viewPos, const Shader* shader, int renderMode) const {
    switch (renderMode) {
        case RENDER_MODE_NORMAL:
        case RENDER_MODE_NO_MATERIALS: {
            bool overrideShader = (shader != nullptr);
            for (int i = 0; i < meshes->size(); i++) {
                const std::shared_ptr<Mesh>& mesh = meshes->at(i);
                if (mesh->material == nullptr)
                    continue;
                if (!overrideShader)
                    shader = &GetMaterialShader(mesh->material);
                UpdateUniforms(*shader, projectionMatrix, viewMatrix, mesh->transformMatrix, viewPos);
                if (renderMode != RENDER_MODE_NO_MATERIALS) {
                    mesh->material->Use(*shader);
                    if (useCustomMaterial && (meshesUsingCustomMaterial->empty() || meshesUsingCustomMaterial->count(i) > 0)) {
                        customMaterial->Use(*shader);
                        shader->SetUniform("material.hasTexture", mesh->material->GetTexture() != TEXTURE_NONE);
                    }
                }
                else {
                    if (mesh->material->cullFaces)
                        glEnable(GL_CULL_FACE);
                    else
                        glDisable(GL_CULL_FACE);
                }
                mesh->Bind();
                mesh->Render();
                glBindVertexArray(0);
            }
        } break;
        case RENDER_MODE_DEBUG_NORMALS:
            shader = &DEBUG_NORMAL_SHADER;
            for (int i = 0; i < meshes->size(); i++) {
                const std::shared_ptr<Mesh>& mesh = meshes->at(i);
                UpdateUniforms(*shader, projectionMatrix, viewMatrix, mesh->transformMatrix, viewPos);
                mesh->Bind();
                mesh->Render();
                glBindVertexArray(0);
            }
            break;
        case RENDER_MODE_DEBUG_AABBS:
            shader = &DEBUG_AABB_SHADER;
            glm::vec3 aabbMin = aabb_.GetMin();
            glm::vec3 aabbMax = aabb_.GetMax();

            shader->Use();
            shader->SetUniform("lineColor", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
            shader->SetUniform("view", viewMatrix);
            shader->SetUniform("projection", projectionMatrix);

            GLuint aabbVao, aabbVbo;
            glGenVertexArrays(1, &aabbVao);
            glGenBuffers(1, &aabbVbo);

            glBindVertexArray(aabbVao);
            glBindBuffer(GL_ARRAY_BUFFER, aabbVbo);
            // I LOVE WRITING PERMUTATIONS BY HAND
            std::vector<glm::vec3> vertices = {
                modelMatrix_ * glm::vec4(aabbMin.x, aabbMin.y, aabbMin.z, 1.0f),
                modelMatrix_ * glm::vec4(aabbMax.x, aabbMin.y, aabbMin.z, 1.0f),
                modelMatrix_ * glm::vec4(aabbMin.x, aabbMin.y, aabbMin.z, 1.0f),
                modelMatrix_ * glm::vec4(aabbMin.x, aabbMin.y, aabbMax.z, 1.0f),
                modelMatrix_ * glm::vec4(aabbMax.x, aabbMin.y, aabbMax.z, 1.0f),
                modelMatrix_ * glm::vec4(aabbMin.x, aabbMin.y, aabbMax.z, 1.0f),
                modelMatrix_ * glm::vec4(aabbMax.x, aabbMin.y, aabbMax.z, 1.0f),
                modelMatrix_ * glm::vec4(aabbMax.x, aabbMin.y, aabbMin.z, 1.0f),
                modelMatrix_ * glm::vec4(aabbMin.x, aabbMax.y, aabbMin.z, 1.0f),
                modelMatrix_ * glm::vec4(aabbMax.x, aabbMax.y, aabbMin.z, 1.0f),
                modelMatrix_ * glm::vec4(aabbMin.x, aabbMax.y, aabbMin.z, 1.0f),
                modelMatrix_ * glm::vec4(aabbMin.x, aabbMax.y, aabbMax.z, 1.0f),
                modelMatrix_ * glm::vec4(aabbMax.x, aabbMax.y, aabbMax.z, 1.0f),
                modelMatrix_ * glm::vec4(aabbMin.x, aabbMax.y, aabbMax.z, 1.0f),
                modelMatrix_ * glm::vec4(aabbMax.x, aabbMax.y, aabbMax.z, 1.0f),
                modelMatrix_ * glm::vec4(aabbMax.x, aabbMax.y, aabbMin.z, 1.0f),
                modelMatrix_ * glm::vec4(aabbMin.x, aabbMin.y, aabbMin.z, 1.0f),
                modelMatrix_ * glm::vec4(aabbMin.x, aabbMax.y, aabbMin.z, 1.0f),
                modelMatrix_ * glm::vec4(aabbMax.x, aabbMin.y, aabbMin.z, 1.0f),
                modelMatrix_ * glm::vec4(aabbMax.x, aabbMax.y, aabbMin.z, 1.0f),
                modelMatrix_ * glm::vec4(aabbMin.x, aabbMin.y, aabbMax.z, 1.0f),
                modelMatrix_ * glm::vec4(aabbMin.x, aabbMax.y, aabbMax.z, 1.0f),
                modelMatrix_ * glm::vec4(aabbMax.x, aabbMin.y, aabbMax.z, 1.0f),
                modelMatrix_ * glm::vec4(aabbMax.x, aabbMax.y, aabbMax.z, 1.0f)
            };
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * 3 * sizeof(float), vertices.data(), GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(0));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
            glDrawArrays(GL_POINTS, 0, (GLsizei) vertices.size() * 3);

            glDeleteVertexArrays(1, &aabbVao);
            glDeleteBuffers(1, &aabbVbo);

            break;
    }
}

bool MeshRenderer::IsOnFrustum(const ViewFrustum& frustum) const {
    glm::vec3 center = modelMatrix_ * glm::vec4(aabb_.center, 1.0f);

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