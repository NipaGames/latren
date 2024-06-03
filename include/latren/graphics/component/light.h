#pragma once

#include <latren/defines/opengl.h>
#include <latren/ec/component.h>
#include <latren/ec/entity.h>
#include <latren/ec/serializable.h>

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

    class LATREN_API ILight {
    public:
        virtual void UseAsNext() = 0;
        virtual void ApplyLight(GLuint) const = 0;
        virtual void ApplyForAllShaders() const;
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

    class LATREN_API PointLight : public Light<PointLight> {
    public:
        float range = 20.0f; LE_RCDV(range)
        glm::vec3 offset = glm::vec3(0.0f); LE_RCDV(offset)

        PointLight() { Light::SetLightType(LightType::POINT); }
        void ApplyLight(GLuint) const;
    };

    class LATREN_API DirectionalLight : public Light<DirectionalLight> {
    public:
        glm::vec3 dir; LE_RCDV(dir)
        
        DirectionalLight() { Light::SetLightType(LightType::DIRECTIONAL); }
        void ApplyLight(GLuint) const;
    };

    class LATREN_API DirectionalLightPlane : public Light<DirectionalLightPlane> {
    public:
        glm::vec3 dir; LE_RCDV(dir)
        float range = 20.0f; LE_RCDV(range)
        float offset = 0.0f; LE_RCDV(offset)

        DirectionalLightPlane() { Light::SetLightType(LightType::DIRECTIONAL_PLANE); }
        void ApplyLight(GLuint) const;
    };

    class LATREN_API Spotlight : public Light<Spotlight> {
    public:
        float cutOffMin; LE_RCDV(cutOffMin)
        float cutOffMax; LE_RCDV(cutOffMax)
        glm::vec3 dir; LE_RCDV(dir)
        float range = 20.0f; LE_RCDV(range)
        glm::vec3 offset = glm::vec3(0.0f); LE_RCDV(offset)

        Spotlight() { Light::SetLightType(LightType::SPOTLIGHT); }
        void ApplyLight(GLuint) const;
    };
}