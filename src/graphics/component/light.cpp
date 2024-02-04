#include <latren/graphics/component/light.h>
#include <latren/game.h>

using namespace Lights;

int Lights::LIGHTS_INDEX = 0;
const int Lights::MAX_LIGHTS = 32;
// this is fucking evil right here
// 32 lights, 1 bit for reserved, 0 for not
uint32_t Lights::RESERVED_LIGHTS = 0x0;

namespace Lights {
    void ReserveIndex(int index) {
        RESERVED_LIGHTS |= (1 << index);
    }
    
    bool IsReserved(int i) {
        return (RESERVED_LIGHTS >> i) == 0x1;
    }

    void ResetIndices() {
        LIGHTS_INDEX = 0;
    }

    int LIGHT_NONE_INDEX = -1;
    int& GetIndex(LightType type) {
        return LIGHTS_INDEX;
    }

    Light::~Light() {
        if (isAssignedToRenderer_)
            Game::GetGameInstanceBase()->GetRenderer().RemoveLight(this);
    }

    void Light::Start() {
        if (!isAssignedToRenderer_)
            Game::GetGameInstanceBase()->GetRenderer().AddLight(this);
    }
    
    void Light::UseAsNext() {
        lightUniform_ = "lights[" + std::to_string(LIGHTS_INDEX++) + "]";
    }
    void Light::ApplyLight(GLuint shader) const {
        glUniform1i(glGetUniformLocation(shader, std::string(lightUniform_ + ".enabled").c_str()), GL_TRUE);
        glUniform1i(glGetUniformLocation(shader, std::string(lightUniform_ + ".type").c_str()), static_cast<int>(type_));
        glUniform3f(glGetUniformLocation(shader, std::string(lightUniform_ + ".color").c_str()), color.x, color.y, color.z);
        glUniform1f(glGetUniformLocation(shader, std::string(lightUniform_ + ".intensity").c_str()), intensity);
    }

    void Light::ApplyForAllShaders() const {
        for (GLuint shader : Game::GetGameInstanceBase()->GetRenderer().GetShaders()) {
            glUseProgram(shader);
            ApplyLight(shader);
        }
        glUseProgram(0);
    }

    IComponent* Light::Clone() const {
        IComponent* c = nullptr;
        switch (type_) {
        case LightType::POINT:
            c = new PointLight(*static_cast<const PointLight*>(this));
            break;
        case LightType::DIRECTIONAL:
            c = new DirectionalLight(*static_cast<const DirectionalLight*>(this));
            break;
        case LightType::DIRECTIONAL_PLANE:
            c = new DirectionalLightPlane(*static_cast<const DirectionalLightPlane*>(this));
            break;
        case LightType::SPOTLIGHT:
            c = new Spotlight(*static_cast<const Spotlight*>(this));
            break;
        }
        return c;
    }
    
    void PointLight::ApplyLight(GLuint shader) const {
        Light::ApplyLight(shader);
        glUniform3f(glGetUniformLocation(shader, std::string(lightUniform_ + ".pos").c_str()), parent->transform->position.x, parent->transform->position.y, parent->transform->position.z);
        glUniform1f(glGetUniformLocation(shader, std::string(lightUniform_ + ".range").c_str()), range);
    }

    void DirectionalLight::ApplyLight(GLuint shader) const {
        Light::ApplyLight(shader);        
        glUniform3f(glGetUniformLocation(shader, std::string(lightUniform_ + ".dir").c_str()), dir.x, dir.y, dir.z);
    }

    void DirectionalLightPlane::ApplyLight(GLuint shader) const {
        Light::ApplyLight(shader);
        glUniform3f(glGetUniformLocation(shader, std::string(lightUniform_ + ".dir").c_str()), dir.x, dir.y, dir.z);
        glUniform1f(glGetUniformLocation(shader, std::string(lightUniform_ + ".y").c_str()), parent->transform->position.y);
        glUniform1f(glGetUniformLocation(shader, std::string(lightUniform_ + ".range").c_str()), range);
    }

    void Spotlight::ApplyLight(GLuint shader) const {
        Light::ApplyLight(shader);
        glUniform3f(glGetUniformLocation(shader, std::string(lightUniform_ + ".dir").c_str()), dir.x, dir.y, dir.z);
        glUniform3f(glGetUniformLocation(shader, std::string(lightUniform_ + ".pos").c_str()), parent->transform->position.x, parent->transform->position.y, parent->transform->position.z);
        glUniform1f(glGetUniformLocation(shader, std::string(lightUniform_ + ".range").c_str()), range);
        glUniform1f(glGetUniformLocation(shader, std::string(lightUniform_ + ".cutOffMin").c_str()), cutOffMin);
        glUniform1f(glGetUniformLocation(shader, std::string(lightUniform_ + ".cutOffMax").c_str()), cutOffMax);
    }
};