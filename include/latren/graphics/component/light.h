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
    LATREN_API int GetNextLightIndex();
    typedef uint32_t LightReserves;
    extern LightReserves RESERVED_LIGHTS;
    extern const uint8_t MAX_LIGHTS;

    LATREN_API void ReserveIndex(int);
    LATREN_API bool IsReserved(int);
    LATREN_API int& GetIndex(LightType);
    LATREN_API void ResetIndices();

    class ILight {
    protected:
        bool isAssignedToRenderer_ = false;
    public:
        virtual void UseAsNext() = 0;
        virtual void ApplyLight(GLuint) const = 0;
        LATREN_API virtual void AssignToRenderer();
        LATREN_API virtual void RemoveFromRenderer();
        LATREN_API virtual void ApplyForAllShaders() const;
    };

    template <typename Derived>
    class Light : public Component<Derived>, public ILight {
    private:
        LightType lightType_ = LightType::NONE;
    protected:
        void SetLightType(LightType t) { lightType_ = t; }
        std::string lightUniform_;
    public:
        DEFINE_COMPONENT_DATA_VALUE(glm::vec3, color, glm::vec3(1.0f));
        DEFINE_COMPONENT_DATA_VALUE(float, intensity, 1.0f);

        virtual ~Light() {
            if (isAssignedToRenderer_)
                RemoveFromRenderer();
        }
        virtual void Start() {
            if (!isAssignedToRenderer_)
                AssignToRenderer();
        }
        LightType GetType() { return lightType_; }
        virtual void UseAsNext() override {
            lightUniform_ = "lights[" + std::to_string(GetNextLightIndex()) + "]";
        }
        virtual void ApplyLight(GLuint shader) const override {
            glUniform1i(glGetUniformLocation(shader, std::string(lightUniform_ + ".enabled").c_str()), GL_TRUE);
            glUniform1i(glGetUniformLocation(shader, std::string(lightUniform_ + ".type").c_str()), static_cast<int>(lightType_));
            glUniform3f(glGetUniformLocation(shader, std::string(lightUniform_ + ".color").c_str()), color.x, color.y, color.z);
            glUniform1f(glGetUniformLocation(shader, std::string(lightUniform_ + ".intensity").c_str()), intensity);
        }
    };

    class PointLight : public Light<PointLight> {
    public:
        DEFINE_COMPONENT_DATA_VALUE(float, range, 20.0f);

        PointLight() { Light::SetLightType(LightType::POINT); }
        LATREN_API void ApplyLight(GLuint) const;
    };

    class DirectionalLight : public Light<DirectionalLight> {
    public:
        DEFINE_COMPONENT_DATA_VALUE_DEFAULT(glm::vec3, dir);
        
        DirectionalLight() { Light::SetLightType(LightType::DIRECTIONAL); }
        LATREN_API void ApplyLight(GLuint) const;
    };

    class DirectionalLightPlane : public Light<DirectionalLightPlane> {
    public:
        DEFINE_COMPONENT_DATA_VALUE_DEFAULT(glm::vec3, dir);
        DEFINE_COMPONENT_DATA_VALUE(float, range, 20.0f);
        
        DirectionalLightPlane() { Light::SetLightType(LightType::DIRECTIONAL_PLANE); }
        LATREN_API void ApplyLight(GLuint) const;
    };

    class Spotlight : public Light<Spotlight> {
    public:
        DEFINE_COMPONENT_DATA_VALUE_DEFAULT(float, cutOffMin);
        DEFINE_COMPONENT_DATA_VALUE_DEFAULT(float, cutOffMax);
        DEFINE_COMPONENT_DATA_VALUE_DEFAULT(glm::vec3, dir);
        DEFINE_COMPONENT_DATA_VALUE(float, range, 20.0f);

        Spotlight() { Light::SetLightType(LightType::SPOTLIGHT); }
        LATREN_API void ApplyLight(GLuint) const;
    };
}