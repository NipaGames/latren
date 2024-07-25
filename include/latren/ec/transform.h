#pragma once

#include "component.h"

#include <latren/defines/opengl.h>
#include <latren/util/quaternion.h>

#include <btBulletDynamicsCommon.h>

class LATREN_API Transform : public Component<Transform> {
public:
    SERIALIZABLE(glm::vec3, position) = glm::vec3(0.0f);
    SERIALIZABLE(glm::vec3, size) = glm::vec3(1.0f);
    SERIALIZABLE(Quaternion, rotation) = glm::quat(0.0f, 0.0f, 0.0f, 0.0f); 
    
    // enable this if the object transform doesn't update
    SERIALIZABLE(bool, isStatic) = false;

    btVector3 btGetPos();
    btVector3 btGetSize();
    btQuaternion btGetRotation();
    btTransform btGetTransform();
    void btSetTransform(const btTransform&);
};
