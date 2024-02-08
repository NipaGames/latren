#pragma once

#include <latren/defines/opengl.h>
#include <latren/entity/component.h>
#include <latren/entity/entity.h>
#include <latren/entity/serializable.h>

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
        glm::vec3 color = glm::vec3(1.0f); LE_RCDV(color)
        float intensity = 1.0f; LE_RCDV(intensity)

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
        float range = 20.0f; LE_RCDV(range)

        PointLight() { Light::SetLightType(LightType::POINT); }
        LATREN_API void ApplyLight(GLuint) const;
    };

    class DirectionalLight : public Light<DirectionalLight> {
    public:
        glm::vec3 dir; LE_RCDV(dir)
        
        DirectionalLight() { Light::SetLightType(LightType::DIRECTIONAL); }
        LATREN_API void ApplyLight(GLuint) const;
    };

    class DirectionalLightPlane : public Light<DirectionalLightPlane> {
    public:
        glm::vec3 dir; LE_RCDV(dir);
        float range = 20.0f; LE_RCDV(range)
        
        DirectionalLightPlane() { Light::SetLightType(LightType::DIRECTIONAL_PLANE); }
        LATREN_API void ApplyLight(GLuint) const;
    };

    class Spotlight : public Light<Spotlight> {
    public:
        float cutOffMin; LE_RCDV(cutOffMin)
        float cutOffMax; LE_RCDV(cutOffMax)
        glm::vec3 dir; LE_RCDV(dir)
        float range = 20.0f; LE_RCDV(range)

        Spotlight() { Light::SetLightType(LightType::SPOTLIGHT); }
        LATREN_API void ApplyLight(GLuint) const;
    };
}