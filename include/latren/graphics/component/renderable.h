#pragma once

#include <latren/entity/component.h>
#include "../material.h"
#include "../camera.h"


class Renderer;
class IRenderable {
protected:
    bool isAssignedToRenderer_ = false;
    LATREN_API virtual void AssignToRenderer();
    LATREN_API virtual void RemoveFromRenderer();
public:
    virtual void CalculateMatrices() = 0;
    virtual bool IsStatic() const = 0;
    virtual bool IsAlwaysOnFrustum() const = 0;
    virtual bool IsOnFrustum(const ViewFrustum&) const = 0;
    virtual bool RenderLate() const = 0;
    virtual bool RenderAfterPostProcessing() const = 0;
    virtual void IRender(const glm::mat4&, const glm::mat4&, const glm::vec3&, const Shader* = nullptr, bool = false) const = 0;
};

template <typename Derived>
class Renderable : public Component<Derived>, public IRenderable {
public:
    // enable this if the object transform doesn't update, no need to calculate model matrices every frame that way
    DEFINE_COMPONENT_DATA_VALUE(bool, isStatic, false);
    DEFINE_COMPONENT_DATA_VALUE(bool, alwaysOnFrustum, false);
    DEFINE_COMPONENT_DATA_VALUE(bool, assignToRenderer, true);
    DEFINE_COMPONENT_DATA_VALUE(bool, disableDepthTest, false);
    DEFINE_COMPONENT_DATA_VALUE_DEFAULT(Material, customMaterial);
    DEFINE_COMPONENT_DATA_VALUE(bool, useCustomMaterial, false);
    // render over skybox and any other entities
    DEFINE_COMPONENT_DATA_VALUE(bool, renderLate, false);
    DEFINE_COMPONENT_DATA_VALUE(bool, renderAfterPostProcessing, false);

    virtual ~Renderable() {
        if (isAssignedToRenderer_)
            RemoveFromRenderer();
    }

    virtual void Start() {
        if (assignToRenderer && !isAssignedToRenderer_)
            AssignToRenderer();
    }
    virtual void FirstUpdate() {
        if (isStatic)
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
        shader.SetUniform("time", (float) glfwGetTime());
    }

    virtual bool IsStatic() const override { return isStatic; }
    virtual bool IsAlwaysOnFrustum() const override { return alwaysOnFrustum; }
    virtual bool IsOnFrustum(const ViewFrustum&) const override { return true; }
    virtual bool RenderLate() const override { return renderLate; }
    virtual bool RenderAfterPostProcessing() const override { return renderAfterPostProcessing; }

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