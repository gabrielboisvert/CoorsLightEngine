#pragma once
#include "Physics/Core/IPhysicsEngine.h"
#include "Maths/FVector3.h"
#include "Physics/Data/CollisionCallbacks.h"
#include "Physics/Core/BulPhysicWorld.h"
#include <set>
#include <mutex>

namespace Physics::Core
{
	const float	EARTH_GRAVITY = -9.807f;

	using CollisionSet = std::set<std::tuple<const btCollisionObject*, const btCollisionObject*, const btPersistentManifold*>>;
	
	

	class BulPhysicsEngine : public IPhysicsEngine, public PhysicWorld
	{
		public:
			std::mutex mMutex;

			std::list<btRigidBody*> mColliders;
			CollisionSet mCollisionPairs;

			BulPhysicsEngine();
			virtual ~BulPhysicsEngine();

			void update(float deltaTime) override;
			void checkForCollisionEvents() override;
		
			void addBoxCollider(btRigidBody& pCollider) override;
			void removeCollider(btRigidBody& pCollider) override;

			void reset() override;

			void lock();
	};
}