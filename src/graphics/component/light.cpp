#include <latren/graphics/component/light.h>
#include <latren/systems.h>
#include <latren/graphics/renderer.h>
#include <latren/ec/transform.h>

using namespace Lights;

int Lights::LIGHTS_INDEX = 0;
// this is fucking evil right here (and way too smartass)
// 32 lights, 1 bit for reserved, 0 for not
LightReserves Lights::RESERVED_LIGHTS = 0x0;
// they warned about premature optimization but this is fucking prenatal optimization
const uint8_t Lights::MAX_LIGHTS = (uint8_t) sizeof(LightReserves) * 8;

namespace Lights {
    int GetNextLightIndex() {
        return LIGHTS_INDEX++;
    }
    
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

    void PointLight::ApplyLight(GLuint shader) const {
        Light::ApplyLight(shader);
        glm::vec3 pos = parent.GetTransform().position.Get() + offset.Get();
        glUniform3f(glGetUniformLocation(shader, std::string(lightUniform_ + ".pos").c_str()), pos.x, pos.y, pos.z);
        glUniform1f(glGetUniformLocation(shader, std::string(lightUniform_ + ".range").c_str()), range);
    }

    void DirectionalLight::ApplyLight(GLuint shader) const {
        Light::ApplyLight(shader);
        glUniform3f(glGetUniformLocation(shader, std::string(lightUniform_ + ".dir").c_str()), dir->x, dir->y, dir->z);
    }

    void DirectionalLightPlane::ApplyLight(GLuint shader) const {
        Light::ApplyLight(shader);
        glUniform3f(glGetUniformLocation(shader, std::string(lightUniform_ + ".dir").c_str()), dir->x, dir->y, dir->z);
        float pos = parent.GetTransform().position->y + offset;
        glUniform1f(glGetUniformLocation(shader, std::string(lightUniform_ + ".y").c_str()), pos);
        glUniform1f(glGetUniformLocation(shader, std::string(lightUniform_ + ".range").c_str()), range);
    }

    void Spotlight::ApplyLight(GLuint shader) const {
        Light::ApplyLight(shader);
        glUniform3f(glGetUniformLocation(shader, std::string(lightUniform_ + ".dir").c_str()), dir->x, dir->y, dir->z);
        glm::vec3 pos = parent.GetTransform().position.Get() + offset.Get();
        glUniform3f(glGetUniformLocation(shader, std::string(lightUniform_ + ".pos").c_str()), pos.x, pos.y, pos.z);
        glUniform1f(glGetUniformLocation(shader, std::string(lightUniform_ + ".range").c_str()), range);
        glUniform1f(glGetUniformLocation(shader, std::string(lightUniform_ + ".cutOffMin").c_str()), cutOffMin);
        glUniform1f(glGetUniformLocation(shader, std::string(lightUniform_ + ".cutOffMax").c_str()), cutOffMax);
    }
};