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
     int GetNextLightIndex();
    typedef uint32_t LightReserves;
    extern LightReserves RESERVED_LIGHTS;
    extern const uint8_t MAX_LIGHTS;

     void ReserveIndex(int);
     bool IsReserved(int);
     int& GetIndex(LightType);
     void ResetIndices();

    class ILight {
    public:
        virtual ~ILight() = default;
        virtual void UseAsNext() = 0;
        virtual void ApplyLight(GLuint) const = 0;
        virtual void ApplyForShaders(const std::vector<GLuint>&) const = 0;

        virtual operator IComponent&() = 0;
    };

    template <typename Derived>
    class Light : public Component<Derived>, public ILight {
    private:
        LightType lightType_ = LightType::NONE;
    protected:
        void SetLightType(LightType t) { lightType_ = t; }
        std::string lightUniform_;
    public:
        SERIALIZABLE(glm::vec3, color) = glm::vec3(1.0f);
        SERIALIZABLE(float, intensity) = 1.0f;

        LightType GetType() { return lightType_; }
        virtual void UseAsNext() override {
            lightUniform_ = "lights[" + std::to_string(GetNextLightIndex()) + "]";
        }
        virtual void ApplyLight(GLuint shader) const override {
            glUniform1i(glGetUniformLocation(shader, std::string(lightUniform_ + ".enabled").c_str()), GL_TRUE);
            glUniform1i(glGetUniformLocation(shader, std::string(lightUniform_ + ".type").c_str()), static_cast<int>(lightType_));
            glUniform3f(glGetUniformLocation(shader, std::string(lightUniform_ + ".color").c_str()), color->x, color->y, color->z);
            glUniform1f(glGetUniformLocation(shader, std::string(lightUniform_ + ".intensity").c_str()), intensity);
        }
        virtual void ApplyForShaders(const std::vector<GLuint>& shaders) const override {
            for (GLuint shader : shaders) {
                glUseProgram(shader);
                ApplyLight(shader);
            }
            glUseProgram(0);
        }

        virtual operator IComponent&() override { return *this; }
    };

    class  PointLight : public Light<PointLight> {
    public:
        SERIALIZABLE(float, range) = 20.0f;
        SERIALIZABLE(glm::vec3, offset) = glm::vec3(0.0f);

        PointLight() { Light::SetLightType(LightType::POINT); }
        void ApplyLight(GLuint) const;
    };

    class  DirectionalLight : public Light<DirectionalLight> {
    public:
        SERIALIZABLE(glm::vec3, dir);
        
        DirectionalLight() { Light::SetLightType(LightType::DIRECTIONAL); }
        void ApplyLight(GLuint) const;
    };

    class  DirectionalLightPlane : public Light<DirectionalLightPlane> {
    public:
        SERIALIZABLE(glm::vec3, dir);
        SERIALIZABLE(float, range) = 20.0f;
        SERIALIZABLE(float, offset) = 0.0f;

        DirectionalLightPlane() { Light::SetLightType(LightType::DIRECTIONAL_PLANE); }
        void ApplyLight(GLuint) const;
    };

    class  Spotlight : public Light<Spotlight> {
    public:
        SERIALIZABLE(float, cutOffMin);
        SERIALIZABLE(float, cutOffMax);
        SERIALIZABLE(glm::vec3, dir);
        SERIALIZABLE(float, range) = 20.0f;
        SERIALIZABLE(glm::vec3, offset) = glm::vec3(0.0f);

        Spotlight() { Light::SetLightType(LightType::SPOTLIGHT); }
        void ApplyLight(GLuint) const;
    };
}