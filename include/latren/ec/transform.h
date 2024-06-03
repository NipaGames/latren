#pragma once

#include "component.h"

#include <latren/defines/opengl.h>
#include <btBulletDynamicsCommon.h>

class LATREN_API Transform : public Component<Transform> {
public:
    glm::vec3 position = glm::vec3(0.0f); LE_RCDV(position)
    glm::vec3 size = glm::vec3(1.0f); LE_RCDV(size)
    glm::quat rotation = glm::quat(0.0f, 0.0f, 0.0f, 0.0f); LE_RCDV(rotation)
    
    // enable this if the object transform doesn't update
    bool isStatic = false; LE_RCDV(isStatic)

    btVector3 btGetPos();
    btVector3 btGetSize();
    btQuaternion btGetRotation();
    btTransform btGetTransform();
    void btSetTransform(const btTransform&);
};
