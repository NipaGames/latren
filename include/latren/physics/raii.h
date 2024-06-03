// so in bullet physics everything is handled with raw pointers
// and the news and deletes get really messy especially with (the owner's) copy constructors
// SO FUCK POINTERS
// modern c++ here we go, i've provided some much-needed raii-wrappers to use these safely (enough) in shared pointers and whatnot
// we'll wait, maybe someday bullet handles the memory management for us

#pragma once

#include "physics.h"
#include <latren/systems.h>

namespace Physics {
    template <typename BtObj>
    class RAIIBtObject {
    protected:
        BtObj obj_;
    public:
        template <typename... T>
        RAIIBtObject(T... args) : obj_(args...) { }
        virtual ~RAIIBtObject() = default;
        // bullet uses shit ton of raw pointers so we'll just return that although i prefer references
        BtObj* Get() {
            return &obj_;
        }
        virtual void AddToDynamicsWorld() { }
    };

    template <typename BtCollisionObj>
    class RAIIBtCollisionObject : public RAIIBtObject<BtCollisionObj> {
    using RAIIBtObject<BtCollisionObj>::RAIIBtObject;
    public:
        virtual void AddToDynamicsWorld() override {
            if (Systems::GetPhysics().GetDynamicsWorld() == nullptr)
                return;
            if constexpr (std::is_base_of_v<btRigidBody, BtCollisionObj>)
                Systems::GetPhysics().GetDynamicsWorld()->addRigidBody(&this->obj_);
            else
                Systems::GetPhysics().GetDynamicsWorld()->addCollisionObject(&this->obj_);
        }
        virtual ~RAIIBtCollisionObject() {
            if (Systems::GetPhysics().GetDynamicsWorld() != nullptr)
                Systems::GetPhysics().GetDynamicsWorld()->removeCollisionObject(&this->obj_);
        }
    };
    
    template <typename BtCharacterControllerColliderObject>
    class RAIIBtCharacterControllerColliderObject : public RAIIBtCollisionObject<BtCharacterControllerColliderObject> {
    using RAIIBtCollisionObject<BtCharacterControllerColliderObject>::RAIIBtCollisionObject;
    public:
        void AddToDynamicsWorld() override {
            if (Systems::GetPhysics().GetDynamicsWorld() != nullptr)
                Systems::GetPhysics().GetDynamicsWorld()->addCollisionObject(&this->obj_, btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);
        }
    };

    class RAIIBtTriangleIndexVertexArray : public RAIIBtObject<btTriangleIndexVertexArray> {
    public:
        virtual ~RAIIBtTriangleIndexVertexArray() {
            for (int i = 0; i < obj_.getIndexedMeshArray().size(); i++) {
                delete[] this->obj_.getIndexedMeshArray()[i].m_vertexBase;
                delete[] this->obj_.getIndexedMeshArray()[i].m_triangleIndexBase;
            }
        }
    };
};