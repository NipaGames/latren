#pragma once
#include <latren/latren.h>
#include <btBulletDynamicsCommon.h>
#include "debugdrawer.h"

namespace Physics {
    extern btDefaultCollisionConfiguration* GLOBAL_COLLISION_CONFIGURATION_;
	extern btCollisionDispatcher* GLOBAL_COLLISION_DISPATCHER_;
	extern btAxisSweep3* GLOBAL_AXIS_SWEEP_;
	extern btSequentialImpulseConstraintSolver* GLOBAL_CONSTRAINT_SOLVER_;
	extern btDiscreteDynamicsWorld* GLOBAL_DYNAMICS_WORLD_;
    extern DebugDrawer* GLOBAL_DEBUG_DRAWER_;
    LATREN_API void Init();
    LATREN_API void Destroy();
    LATREN_API void Update(double);

    LATREN_API btDiscreteDynamicsWorld* GetGlobalDynamicsWorld();
    LATREN_API btAxisSweep3* GetGlobalAxisSweep();
};