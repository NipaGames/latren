#include <latren/physics/physics.h>
#include <latren/physics/debugdrawer.h>
#include <latren/systems.h>
#include <latren/game.h>

void PhysicsWorld::Init() {
    btVector3 worldSize = btVector3(2000, 2000, 2000);
    collisionConfiguration_ = std::make_unique<btDefaultCollisionConfiguration>();
    collisionDispatcher_ = std::make_unique<btCollisionDispatcher>(collisionConfiguration_.get());
    axisSweep_ = std::make_unique<btAxisSweep3>(-worldSize / 2, worldSize / 2);
    ghostPairCallback_ = std::make_unique<btGhostPairCallback>();
    axisSweep_->getOverlappingPairCache()->setInternalGhostPairCallback(ghostPairCallback_.get());
    constraintSolver_ = std::make_unique<btSequentialImpulseConstraintSolver>();
    dynamicsWorld_ = std::make_unique<btDiscreteDynamicsWorld>(collisionDispatcher_.get(), axisSweep_.get(), constraintSolver_.get(), collisionConfiguration_.get());
    dynamicsWorld_->setGravity(btVector3(0, -10, 0));

    debugDrawer_ = std::make_unique<Physics::DebugDrawer>();
    dynamicsWorld_->setDebugDrawer(debugDrawer_.get());
}

void PhysicsWorld::Update(double dt) {
    dynamicsWorld_->stepSimulation(btScalar(dt), 10, btScalar(Systems::GetGame().GetFixedDeltaTime()));
}

void PhysicsWorld::Destroy() {
    dynamicsWorld_ = nullptr;
    constraintSolver_ = nullptr;
    axisSweep_ = nullptr;
    ghostPairCallback_ = nullptr;
    collisionDispatcher_ = nullptr;
    collisionConfiguration_ = nullptr;
    debugDrawer_ = nullptr;
}

btDiscreteDynamicsWorld* PhysicsWorld::GetDynamicsWorld() {
    return dynamicsWorld_.get();
}

btAxisSweep3* PhysicsWorld::GetAxisSweep() {
    return axisSweep_.get();
}