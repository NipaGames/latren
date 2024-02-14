// so in bullet physics everything is handled with raw pointers
// and the news and deletes get really messy especially with (the owner's) copy constructors
// SO FUCK POINTERS
// modern c++ here we go, i've provided some much-needed raii-wrappers to use these safely (enough) in shared pointers and whatnot
// we'll wait, maybe someday bullet handles the memory management for us

#pragma once

#include "physics.h"

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
    };

    template <typename BtCollisionObj>
    class RAIIBtCollisionObject : public RAIIBtObject<BtCollisionObj> {
    public:
        template <typename... T>
        RAIIBtCollisionObject(T... args) : RAIIBtObject(args...) {
            if constexpr(std::is_base_of_v<btRigidBody, BtCollisionObj>)
                GetGlobalDynamicsWorld()->addRigidBody(&obj_);
            else
                GetGlobalDynamicsWorld()->addCollisionObject(&obj_);
        }
        virtual ~RAIIBtCollisionObject() {
            if (GetGlobalDynamicsWorld() != nullptr)
                GetGlobalDynamicsWorld()->removeCollisionObject(&obj_);
        }
    };

    class RAIIBtTriangleIndexVertexArray : public RAIIBtObject<btTriangleIndexVertexArray> {
    public:
        virtual ~RAIIBtTriangleIndexVertexArray() {
            for (int i = 0; i < obj_.getIndexedMeshArray().size(); i++) {
                delete[] obj_.getIndexedMeshArray()[i].m_vertexBase;
                delete[] obj_.getIndexedMeshArray()[i].m_triangleIndexBase;
            }
        }
    };
};