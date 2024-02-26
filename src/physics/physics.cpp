#include <latren/physics/physics.h>
#include <latren/game.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
    
// don't really know if these have a reason to be heap-allocated but the official Bullet HelloWorld.cpp has them allocated like this,
// so there must be some reason, right?
btDefaultCollisionConfiguration* Physics::GLOBAL_COLLISION_CONFIGURATION_ = nullptr;
btCollisionDispatcher* Physics::GLOBAL_COLLISION_DISPATCHER_ = nullptr;
btAxisSweep3* Physics::GLOBAL_AXIS_SWEEP_ = nullptr;
btSequentialImpulseConstraintSolver* Physics::GLOBAL_CONSTRAINT_SOLVER_ = nullptr;
btDiscreteDynamicsWorld* Physics::GLOBAL_DYNAMICS_WORLD_ = nullptr;
Physics::DebugDrawer* Physics::GLOBAL_DEBUG_DRAWER_ = nullptr;

void Physics::Init() {
    btVector3 worldSize = btVector3(2000, 2000, 2000);
    GLOBAL_COLLISION_CONFIGURATION_ = new btDefaultCollisionConfiguration();
    GLOBAL_COLLISION_DISPATCHER_ = new btCollisionDispatcher(GLOBAL_COLLISION_CONFIGURATION_);
    GLOBAL_AXIS_SWEEP_ = new btAxisSweep3(-worldSize / 2, worldSize / 2);
    GLOBAL_AXIS_SWEEP_->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
    GLOBAL_CONSTRAINT_SOLVER_ = new btSequentialImpulseConstraintSolver;
    GLOBAL_DYNAMICS_WORLD_ = new btDiscreteDynamicsWorld(GLOBAL_COLLISION_DISPATCHER_, GLOBAL_AXIS_SWEEP_, GLOBAL_CONSTRAINT_SOLVER_, GLOBAL_COLLISION_CONFIGURATION_);
    GLOBAL_DYNAMICS_WORLD_->setGravity(btVector3(0, -10, 0));

    GLOBAL_DEBUG_DRAWER_ = new DebugDrawer();
    GLOBAL_DYNAMICS_WORLD_->setDebugDrawer(GLOBAL_DEBUG_DRAWER_);
}

void Physics::Update(double dt) {
    GLOBAL_DYNAMICS_WORLD_->stepSimulation(btScalar(dt), 10, btScalar(Game::GetGameInstanceBase()->GetFixedDeltaTime()));
}

void Physics::Destroy() {
    delete GLOBAL_DYNAMICS_WORLD_;
    delete GLOBAL_CONSTRAINT_SOLVER_;
    delete GLOBAL_AXIS_SWEEP_;
    delete GLOBAL_COLLISION_DISPATCHER_;
    delete GLOBAL_COLLISION_CONFIGURATION_;
    delete GLOBAL_DEBUG_DRAWER_;
    GLOBAL_DYNAMICS_WORLD_ = nullptr;
    GLOBAL_CONSTRAINT_SOLVER_ = nullptr;
    GLOBAL_AXIS_SWEEP_ = nullptr;
    GLOBAL_COLLISION_DISPATCHER_ = nullptr;
    GLOBAL_COLLISION_CONFIGURATION_ = nullptr;
    GLOBAL_DEBUG_DRAWER_ = nullptr;
}

btDiscreteDynamicsWorld* Physics::GetGlobalDynamicsWorld() {
    return GLOBAL_DYNAMICS_WORLD_;
}

btAxisSweep3* Physics::GetGlobalAxisSweep() {
    return GLOBAL_AXIS_SWEEP_;
}