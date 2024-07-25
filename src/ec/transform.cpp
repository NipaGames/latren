#include <latren/ec/transform.h>
#include <latren/physics/utils.h>

glm::mat4 Transform::CreateTransformationMatrix() const {
    glm::mat4 mat = glm::translate(glm::mat4(1.0f), position.Get());
    mat *= glm::mat4_cast(rotation->GetOrientation());
    mat = glm::scale(mat, size.Get());
    return mat;
}

btVector3 Transform::btGetPos() const {
    return Physics::GLMVectorToBtVector3(position);
}

btVector3 Transform::btGetSize() const {
    return Physics::GLMVectorToBtVector3(size);
}

btQuaternion Transform::btGetRotation() const {
    return Physics::GLMQuatToBtQuat(rotation->GetOrientation());
}

btTransform Transform::btGetTransform() const {
    btTransform trans;
    trans.setIdentity();
    trans.setOrigin(btGetPos());
    trans.setRotation(btGetRotation());
    return trans;
}

void Transform::btSetTransform(const btTransform& trans) {
    position = Physics::BtVectorToGLMVector3(trans.getOrigin());
    rotation = Physics::BtQuatToGLMQuat(trans.getRotation());
}