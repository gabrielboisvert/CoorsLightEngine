#pragma once

#include <functional>
#include <Bullet/btBulletDynamicsCommon.h>
#include <Bullet/BulletCollision/BroadphaseCollision/btBroadphaseInterface.h>
#include <Bullet/BulletDynamics/ConstraintSolver/btBatchedConstraints.h>

using CollisionEnterCallback = std::function<void(btRigidBody*, btRigidBody*, void*, btPersistentManifold*)>;
using CollisionExitCallback = std::function<void(btRigidBody*, btRigidBody*, void*, btPersistentManifold*)>;

namespace Physics::Data
{
	struct CollisionCallbacks
	{
		CollisionEnterCallback enter;
		CollisionEnterCallback stay;
		CollisionExitCallback exit;
		void* physicsObject;
	};
}