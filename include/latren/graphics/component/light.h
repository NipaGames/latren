#pragma once

#include <latren/defines/opengl.h>
#include <latren/entity/component.h>
#include <latren/entity/entity.h>

namespace Lights {
    enum class LightType {
        NONE = 0,
        POINT = 1,
        SPOTLIGHT = 2,
        DIRECTIONAL = 3,
        DIRECTIONAL_PLANE = 4
    };
    
    extern int LIGHTS_INDEX;
    extern const int MAX_LIGHTS;
    extern uint32_t RESERVED_LIGHTS;

    LATREN_API void ReserveIndex(int);
    LATREN_API bool IsReserved(int);
    LATREN_API int& GetIndex(LightType);
    LATREN_API void ResetIndices();

    class Light : public Component<Light> {
    friend class Renderer;
    private:
        LightType type_ = LightType::NONE;
    protected:
        void SetType(LightType t) { type_ = t; }
        std::string lightUniform_;
        bool isAssignedToRenderer_ = false;
    public:
        DEFINE_COMPONENT_DATA_VALUE(glm::vec3, color, glm::vec3(1.0f));
        DEFINE_COMPONENT_DATA_VALUE(float, intensity, 1.0f);

        LATREN_API virtual ~Light();
        LATREN_API IComponent* Clone() const override;
        LATREN_API void Start() override;
        LightType GetType() { return type_; }
        LATREN_API virtual void UseAsNext();
        LATREN_API virtual void ApplyLight(GLuint) const;
        LATREN_API virtual void ApplyForAllShaders() const;
    };

    class PointLight : public Light {
    public:
        DEFINE_COMPONENT_DATA_VALUE(float, range, 20.0f);

        PointLight() { Light::SetType(LightType::POINT); }
        LATREN_API void ApplyLight(GLuint) const;
    };
    REGISTER_COMPONENT(PointLight);

    class DirectionalLight : public Light {
    public:
        DEFINE_COMPONENT_DATA_VALUE_DEFAULT(glm::vec3, dir);
        
        DirectionalLight() { Light::SetType(LightType::DIRECTIONAL); }
        LATREN_API void ApplyLight(GLuint) const;
    };
    REGISTER_COMPONENT(DirectionalLight);

    class DirectionalLightPlane : public Light {
    public:
        DEFINE_COMPONENT_DATA_VALUE_DEFAULT(glm::vec3, dir);
        DEFINE_COMPONENT_DATA_VALUE(float, range, 20.0f);
        
        DirectionalLightPlane() { Light::SetType(LightType::DIRECTIONAL_PLANE); }
        LATREN_API void ApplyLight(GLuint) const;
    };
    REGISTER_COMPONENT(DirectionalLightPlane);

    class Spotlight : public Light {
    public:
        DEFINE_COMPONENT_DATA_VALUE_DEFAULT(float, cutOffMin);
        DEFINE_COMPONENT_DATA_VALUE_DEFAULT(float, cutOffMax);
        DEFINE_COMPONENT_DATA_VALUE_DEFAULT(glm::vec3, dir);
        DEFINE_COMPONENT_DATA_VALUE(float, range, 20.0f);

        Spotlight() { Light::SetType(LightType::SPOTLIGHT); }
        LATREN_API void ApplyLight(GLuint) const;
    };
    REGISTER_COMPONENT(Spotlight);
}