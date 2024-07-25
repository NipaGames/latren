#pragma once

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

class Quaternion {
private:
    glm::quat orientation_;
    #ifdef LATREN_QUATERNION_STORE_EULERS
    glm::vec3 eulers_;
    #endif
public:
    Quaternion() = default;
    Quaternion(const glm::quat& orientation) {
        SetOrientation(orientation);
    }
    Quaternion(const glm::vec3& eulers) {
        SetEulers(eulers);
    }
    const glm::quat& GetOrientation() const {
        return orientation_;
    }
    #ifdef LATREN_QUATERNION_STORE_EULERS
    const glm::vec3& GetEulers() const {
        return eulers_;
    }
    #else
    glm::vec3 GetEulers() const {
        return glm::eulerAngles(GetOrientation());
    }
    #endif
    void SetOrientation(const glm::quat& orientation) {
        orientation_ = orientation;
        #ifdef LATREN_QUATERNION_STORE_EULERS
        eulers_ = glm::eulerAngles(orientation);
        #endif
    }
    void SetEulers(const glm::vec3& eulers) {
        #ifdef LATREN_QUATERNION_STORE_EULERS
        eulers_ = eulers;
        #endif
        orientation_ = glm::quat(eulers);
    }
};