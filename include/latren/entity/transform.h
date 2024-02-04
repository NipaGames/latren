#pragma once

#include "component.h"

#include <latren/defines/opengl.h>
#include <btBulletDynamicsCommon.h>

class Transform : public Component<Transform> {
public:
    DEFINE_COMPONENT_DATA_VALUE(glm::vec3, position, glm::vec3(0.0f));
    DEFINE_COMPONENT_DATA_VALUE(glm::vec3, size, glm::vec3(1.0f));
    DEFINE_COMPONENT_DATA_VALUE(glm::quat, rotation, glm::quat(0.0f, 0.0f, 0.0f, 0.0f));

    LATREN_API btVector3 btGetPos();
    LATREN_API btVector3 btGetSize();
    LATREN_API btQuaternion btGetRotation();
    LATREN_API btTransform btGetTransform();
    LATREN_API void btSetTransform(const btTransform&);
};