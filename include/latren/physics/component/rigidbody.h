#pragma once

#include "../physics.h"
#include <latren/entity/component.h>

namespace Physics {
    enum class ColliderConstructor {
        TRANSFORM,
        AABB,
        MESH
    };

    template <typename BtObj>
    class RAIIBtCollisionObject {
    private:
        BtObj obj_;
    public:
        template <typename... T>
        RAIIBtCollisionObject(T... args) : obj_(args...) {
            if constexpr(std::is_base_of_v<btRigidBody, BtObj>)
                GetGlobalDynamicsWorld()->addRigidBody(&obj_);
            else
                GetGlobalDynamicsWorld()->addCollisionObject(&obj_);
        }
        ~RAIIBtCollisionObject() {
            if (GetGlobalDynamicsWorld() != nullptr)
                GetGlobalDynamicsWorld()->removeCollisionObject(&obj_);
        }
        BtObj* Get() {
            return &obj_;
        }
    };

    class RAIIBtTriangleIndexVertexArray {
    private:
        btTriangleIndexVertexArray arr_;
    public:
        template <typename... T>
        RAIIBtTriangleIndexVertexArray(T... args) : arr_(args...) { }
        ~RAIIBtTriangleIndexVertexArray() {
            for (int i = 0; i < arr_.getIndexedMeshArray().size(); i++) {
                delete[] arr_.getIndexedMeshArray()[i].m_vertexBase;
                delete[] arr_.getIndexedMeshArray()[i].m_triangleIndexBase;
            }
        }
        btTriangleIndexVertexArray* Get() {
            return &arr_;
        }
    };

    // would be great to have a separate collider component
    class RigidBody : public Component<RigidBody> {
    private:
        bool enableDebugVisualization_ = true;
        bool enableRotation_ = true;
        glm::vec3 colliderOriginOffset_ = glm::vec3(0.0f);
        std::shared_ptr<RAIIBtTriangleIndexVertexArray> meshData_ = nullptr;
        std::shared_ptr<btBvhTriangleMeshShape> CreateMeshCollider();
    public:
        std::shared_ptr<RAIIBtCollisionObject<btRigidBody>> rigidBody = nullptr;
        std::shared_ptr<btCollisionShape> collider = nullptr; LE_RCDV(collider)
        ColliderConstructor colliderFrom = ColliderConstructor::TRANSFORM; LE_RCDV(colliderFrom)
        std::vector<int> colliderMeshes; LE_RCDV(colliderMeshes)
        float mass = 1.0f; LE_RCDV(mass)
        bool interpolate = false; LE_RCDV(interpolate)
        bool overwriteTransform = true; LE_RCDV(overwriteTransform)
        bool doesMassAffectGravity = false; LE_RCDV(doesMassAffectGravity)
        bool disableCollisions = false; LE_RCDV(disableCollisions)
        RigidBody() = default;
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