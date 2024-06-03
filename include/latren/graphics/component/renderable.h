#pragma once

#include <latren/ec/component.h>
#include <latren/io/serializer.h>
#include "../material.h"
#include "../camera.h"

namespace RenderPass {
    enum Enum : size_t {
        // until i fix transparency all transparent objects should be rendered late
        NORMAL,
        LATE,
        AFTER_POST_PROCESSING,
        // doesn't render in the renderer pipeline
        CUSTOM
    };
    inline constexpr size_t TOTAL_RENDER_PASSES = magic_enum::enum_count<Enum>();
};

class Renderer;
class IRenderable {
public:
    virtual ~IRenderable() = default;
    virtual RenderPass::Enum GetRenderPass() const = 0;
    virtual void CalculateMatrices() = 0;
    virtual bool IsStatic() const = 0;
    virtual bool IsAlwaysOnFrustum() const = 0;
    virtual bool IsOnFrustum(const ViewFrustum&) const = 0;
    virtual glm::vec3 GetPosition() const = 0;
    virtual operator IComponent&() = 0;
    virtual void IRender(const glm::mat4&, const glm::mat4&, const glm::vec3&, const Shader* = nullptr, bool = false) const = 0;
};

template <typename Derived>
class Renderable : public Component<Derived>, public IRenderable {
public:
    bool alwaysOnFrustum = false; LE_RCDV(alwaysOnFrustum)
    bool disableDepthTest = false; LE_RCDV(disableDepthTest)
    Material customMaterial; LE_RCDV(customMaterial)
    // leave empty to affect all meshes
    bool useCustomMaterial = false; LE_RCDV(useCustomMaterial)
    std::unordered_set<int> meshesUsingCustomMaterial;
    RenderPass::Enum renderPass = RenderPass::NORMAL; LE_RCDV(renderPass)
    glm::vec3 offset = glm::vec3(0.0f); LE_RCDV(offset)

    virtual bool IsStatic() const override {
        return this->parent.GetTransform().isStatic;
    }

    virtual void Start() override {
        if (IsStatic())
            CalculateMatrices();
    }

    virtual const Shader& GetMaterialShader(const std::shared_ptr<Material>& mat) const { return mat->GetShader(); }
    virtual void UseMaterial(const std::shared_ptr<Material>& mat) const { mat->Use(); }
    virtual void UpdateUniforms(const Shader& shader, const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const glm::vec3& viewPos) const {
        shader.Use();
        // default uniforms
        // idk if it would be better to define these as default initializers for Material class
        shader.SetUniform("material.color", glm::vec3(1.0f));
        shader.SetUniform("material.opacity", 1.0f);
        shader.SetUniform("material.fog.use", false);

        shader.SetUniform("projection", projectionMatrix);
        shader.SetUniform("view", viewMatrix);
        shader.SetUniform("viewPos", viewPos);
        shader.SetUniform("time", (float) IComponent::GetTime());
    }

    virtual operator IComponent&() override { return *this; }
    virtual bool IsAlwaysOnFrustum() const override { return alwaysOnFrustum; }
    virtual bool IsOnFrustum(const ViewFrustum&) const override { return true; }
    virtual RenderPass::Enum GetRenderPass() const override { return renderPass; }
    virtual glm::vec3 GetPosition() const {
        return this->parent.GetTransform().position + offset;
    }

    virtual void CalculateMatrices() override { }
    virtual void IRender(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const glm::vec3& viewPos, const Shader* shader = nullptr, bool aabbDebug = false) const override {
        if (disableDepthTest)
            glDisable(GL_DEPTH_TEST);
        Render(projectionMatrix, viewMatrix, viewPos, shader, aabbDebug);
        glBindVertexArray(0);
        if (disableDepthTest)
            glEnable(GL_DEPTH_TEST);
    }
    virtual void Render(const glm::mat4&, const glm::mat4&, const glm::vec3&, const Shader* = nullptr, bool = false) const { }
};