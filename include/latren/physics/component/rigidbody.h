#pragma once

#include "../physics.h"
#include <latren/entity/component.h>

namespace Physics {
    enum class ColliderConstructor {
        TRANSFORM,
        AABB,
        MESH
    };

    // would be great to have a separate collider component
    class RigidBody : public Component<RigidBody> {
    private:
        bool enableDebugVisualization_ = true;
        bool enableRotation_ = true;
        glm::vec3 colliderOriginOffset_ = glm::vec3(0.0f);
        btTriangleIndexVertexArray* meshData_ = nullptr;
        btCollisionShape* CreateMeshCollider();
    public:
        btRigidBody* rigidBody = nullptr;

        btCollisionShape* collider = nullptr; LE_RCDV(collider)
        ColliderConstructor colliderFrom = ColliderConstructor::TRANSFORM; LE_RCDV(colliderFrom)
        std::vector<int> colliderMeshes; LE_RCDV(colliderMeshes)
        float mass = 1.0f; LE_RCDV(mass)
        bool interpolate = false; LE_RCDV(interpolate)
        bool overwriteTransform = true; LE_RCDV(overwriteTransform)
        bool doesMassAffectGravity = false; LE_RCDV(doesMassAffectGravity)
        bool disableCollisions = false; LE_RCDV(disableCollisions)
        LATREN_API ~RigidBody();
        LATREN_API void Start();
        LATREN_API void Update();
        LATREN_API void FixedUpdate();
        LATREN_API void SetPos(const glm::vec3&);
        LATREN_API void UpdateTransform();
        LATREN_API void CopyTransform();
        LATREN_API void EnableDebugVisualization(bool);
        LATREN_API void EnableRotation(bool);
    };
};