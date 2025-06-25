#pragma once

#include "../physics.h"
#include "../raii.h"
#include <latren/ec/component.h>

namespace Physics {
    enum class ColliderConstructor {
        TRANSFORM,
        AABB,
        MESH
    };

    // would be great to have a separate collider component
    class  RigidBody : public Component<RigidBody> {
    private:
        bool enableDebugVisualization_ = true;
        bool enableRotation_ = true;
        glm::vec3 colliderOriginOffset_ = glm::vec3(0.0f);
        std::shared_ptr<RAIIBtTriangleIndexVertexArray> meshData_ = nullptr;
        std::shared_ptr<btBvhTriangleMeshShape> CreateMeshCollider();
    public:
        std::shared_ptr<RAIIBtCollisionObject<btRigidBody>> rigidBody = nullptr;
        SERIALIZABLE(std::shared_ptr<btCollisionShape>, collider) = nullptr;
        SERIALIZABLE(ColliderConstructor, colliderFrom) = ColliderConstructor::TRANSFORM;
        SERIALIZABLE(std::vector<int>, colliderMeshes);
        SERIALIZABLE(float, mass) = 1.0f;
        // interpolate transform every frame
        SERIALIZABLE(bool, enableSmoothInterpolation) = false;
        SERIALIZABLE(bool, overwriteTransform) = true;
        SERIALIZABLE(bool, doesMassAffectGravity) = false;
        SERIALIZABLE(bool, disableCollisions) = false;
        RigidBody() = default;
        void Start();
        void Update();
        void FixedUpdate();
        void SetPos(const glm::vec3&);
        void UpdateTransform();
        void CopyTransform();
        void EnableDebugVisualization(bool);
        void EnableRotation(bool);
    };
};