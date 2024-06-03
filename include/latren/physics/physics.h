#pragma once
#include "debugdrawer.h"
#include <latren/latren.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <memory>

class LATREN_API PhysicsWorld {
private:
    // don't really know if these have a reason to be heap-allocated but the official Bullet HelloWorld.cpp has them allocated like this,
    // so there must be some reason, right?
    // nvm bullet has its own allocs, better keep using them
    std::unique_ptr<btDefaultCollisionConfiguration> collisionConfiguration_;
    std::unique_ptr<btCollisionDispatcher> collisionDispatcher_;
    std::unique_ptr<btAxisSweep3> axisSweep_;
    std::unique_ptr<btGhostPairCallback> ghostPairCallback_;
    std::unique_ptr<btSequentialImpulseConstraintSolver> constraintSolver_;
    std::unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld_;
    std::unique_ptr<Physics::DebugDrawer> debugDrawer_;
public:
    void Init();
    void Destroy();
    void Update(double);

    btDiscreteDynamicsWorld* GetDynamicsWorld();
    btAxisSweep3* GetAxisSweep();
};
