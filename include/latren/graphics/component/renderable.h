#pragma once

#include <latren/entity/component.h>
#include <latren/io/serializer.h>
#include <latren/io/serializetypes.h>
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
JSON_SERIALIZE_ENUM(RenderPass::Enum);

class Renderer;
class IRenderable {
public:
    virtual RenderPass::Enum GetRenderPass() const = 0;
    virtual void CalculateMatrices() = 0;
    virtual bool IsStatic() const = 0;
    virtual bool IsAlwaysOnFrustum() const = 0;
    virtual bool IsOnFrustum(const ViewFrustum&) const = 0;
    virtual operator IComponent&() = 0;
    virtual void IRender(const glm::mat4&, const glm::mat4&, const glm::vec3&, const Shader* = nullptr, bool = false) const = 0;
};

template <typename Derived>
class Renderable : public Component<Derived>, public IRenderable {
public:
    // enable this if the object transform doesn't update, no need to calculate model matrices every frame that way
    bool isStatic = false; LE_RCDV(isStatic)
    bool alwaysOnFrustum = false; LE_RCDV(alwaysOnFrustum)
    bool disableDepthTest = false; LE_RCDV(disableDepthTest)
    Material customMaterial; LE_RCDV(customMaterial)
    bool useCustomMaterial = false; LE_RCDV(useCustomMaterial)
    // render over skybox and any other entities
    RenderPass::Enum renderPass = RenderPass::NORMAL; LE_RCDV(renderPass)

    virtual void Start() override {
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

    virtual operator IComponent&() override { return *this; }
    virtual bool IsStatic() const override { return isStatic; }
    virtual bool IsAlwaysOnFrustum() const override { return alwaysOnFrustum; }
    virtual bool IsOnFrustum(const ViewFrustum&) const override { return true; }
    virtual RenderPass::Enum GetRenderPass() const override { return renderPass; }

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