#pragma once

#include <latren/entity/component.h>
#include "../material.h"
#include "../camera.h"

class Renderable : public Component<Renderable> {
friend class Renderer;
private:
    bool isAssignedToRenderer_ = false;
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

    LATREN_API virtual ~Renderable();
    LATREN_API virtual const Shader& GetMaterialShader(const std::shared_ptr<Material>&) const;
    LATREN_API virtual void UpdateUniforms(const Shader&, const glm::mat4&, const glm::mat4&) const;
    LATREN_API virtual void UseMaterial(const std::shared_ptr<Material>&) const;

    LATREN_API virtual void Start();
    LATREN_API virtual void FirstUpdate();
    LATREN_API virtual bool IsOnFrustum(const ViewFrustum&) const;
    LATREN_API virtual void IRender(const glm::mat4&, const glm::mat4&, const Shader* = nullptr, bool = false) const;
    
    virtual void CalculateMatrices() { }
    virtual void Render(const glm::mat4&, const glm::mat4&, const Shader* = nullptr, bool = false) const { }
};