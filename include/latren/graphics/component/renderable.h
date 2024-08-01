#pragma once

#include <latren/ec/component.h>
#include <latren/io/serializationinterface.h>
#include "../material.h"
#include "../camera.h"
#include "../renderpass.h"
#include "../rendermode.h"

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
    virtual void IRender(const glm::mat4&, const glm::mat4&, const glm::vec3&, const Shader* = nullptr, int = RENDER_MODE_NORMAL) const = 0;
    
    virtual operator IComponent&() = 0;
};

template <typename Derived>
class Renderable : public Component<Derived>, public IRenderable {
public:
    SERIALIZABLE(bool, alwaysOnFrustum) = false;
    SERIALIZABLE(bool, disableDepthTest) = false;
    SERIALIZABLE(Material, customMaterial);
    // leave empty to affect all meshes
    SERIALIZABLE(bool, useCustomMaterial) = false;
    SERIALIZABLE(std::unordered_set<int>, meshesUsingCustomMaterial);
    SERIALIZABLE(RenderPass::Enum, renderPass) = RenderPass::NORMAL;
    SERIALIZABLE(glm::vec3, offset) = glm::vec3(0.0f);

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
        shader.SetUniform("projection", projectionMatrix);
        shader.SetUniform("view", viewMatrix);
        shader.SetUniform("viewPos", viewPos);
        shader.SetUniform("time", (float) IComponent::GetTime());
    }

    virtual bool IsAlwaysOnFrustum() const override { return alwaysOnFrustum; }
    virtual bool IsOnFrustum(const ViewFrustum&) const override { return true; }
    virtual RenderPass::Enum GetRenderPass() const override { return renderPass; }
    virtual glm::vec3 GetPosition() const {
        return this->parent.GetTransform().position.Get() + offset.Get();
    }

    virtual void CalculateMatrices() override { }
    virtual void IRender(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix, const glm::vec3& viewPos, const Shader* shader = nullptr, int renderMode = RENDER_MODE_NORMAL) const override {
        if (disableDepthTest)
            glDisable(GL_DEPTH_TEST);
        Render(projectionMatrix, viewMatrix, viewPos, shader, renderMode);
        glBindVertexArray(0);
        if (disableDepthTest)
            glEnable(GL_DEPTH_TEST);
    }
    virtual void Render(const glm::mat4&, const glm::mat4&, const glm::vec3&, const Shader* = nullptr, int = RENDER_MODE_NORMAL) const { }

    virtual operator IComponent&() override { return *this; }
};