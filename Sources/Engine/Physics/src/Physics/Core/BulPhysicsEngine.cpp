#include "Physics/Core/BulPhysicsEngine.h"
#include "EngineCore/Service/ServiceLocator.h"
#include "Physics/Data/CollisionCallbacks.h"
#include "Physics/Data/CollisionGroup.h"
#include <algorithm>
#include <iterator>
#include "Physics/Renderer/PhysicsDebugDrawer.h"

using namespace Physics::Core;

BulPhysicsEngine::BulPhysicsEngine() 
{
	setGravity(0, EARTH_GRAVITY, 0);
	setDebugDrawer(new Renderer::PhysicsDebugDrawer());
}

BulPhysicsEngine::~BulPhysicsEngine()
{
	
}

void BulPhysicsEngine::update(float pDeltaTime)
{
	if (pDeltaTime == 0)
		return;

	std::unique_lock lock(mMutex);
	stepSimulation(pDeltaTime);
	checkForCollisionEvents();
}

void BulPhysicsEngine::checkForCollisionEvents()
{
	btDispatcher* dispatcher = getDispatcher();
	if (!dispatcher)
		return;

	CollisionSet NewCollisions;
	int numManifolds = dispatcher->getNumManifolds();
	for (int i = 0; i < numManifolds; i++)
	{
		btPersistentManifold* contactManifold = dispatcher->getManifoldByIndexInternal(i);

		if (!contactManifold || contactManifold->getNumContacts() <= 0)
			continue;

		const btCollisionObject* obA = contactManifold->getBody0();
		const btCollisionObject* obB = contactManifold->getBody1();

		bool const Swapped = obA > obB;
		const btCollisionObject* RigidA = Swapped ? obA : obB;
		const btCollisionObject* RigidB = Swapped ? obB : obA;

		NewCollisions.emplace(RigidA, RigidB, contactManifold);
	}

	CollisionSet differenceSet;
	std::set_symmetric_difference(mCollisionPairs.begin(), mCollisionPairs.end(), NewCollisions.begin(), NewCollisions.end(), std::inserter(differenceSet, differenceSet.begin()));

	CollisionSet stayColision;
	std::set_union(mCollisionPairs.begin(), mCollisionPairs.end(), NewCollisions.begin(), NewCollisions.end(), std::inserter(stayColision, stayColision.begin()));
	for (std::tuple<const btCollisionObject*, const btCollisionObject*, const btPersistentManifold*> const& pair : stayColision)
	{
		Data::CollisionCallbacks* Callbacks = (Data::CollisionCallbacks*)std::get<0>(pair)->getUserPointer();
		Data::CollisionCallbacks* Callbacks2 = (Data::CollisionCallbacks*)std::get<1>(pair)->getUserPointer();
		btRigidBody* body0 = (btRigidBody*)std::get<0>(pair);
		btRigidBody* body1 = (btRigidBody*)std::get<1>(pair);
		btPersistentManifold* manifold = (btPersistentManifold*)std::get<2>(pair);
		if (Callbacks->stay)
			Callbacks->stay(body0, body1, Callbacks2->physicsObject, manifold);

		if (Callbacks2->stay)
			Callbacks2->stay(body1, body0, Callbacks->physicsObject, manifold);
	}

	for (std::tuple<const btCollisionObject*, const btCollisionObject*, const btPersistentManifold*> const& pair : differenceSet)
	{
		if (mCollisionPairs.find(pair) == mCollisionPairs.end())
		{
			Data::CollisionCallbacks* Callbacks = (Data::CollisionCallbacks*)std::get<0>(pair)->getUserPointer();
			Data::CollisionCallbacks* Callbacks2 = (Data::CollisionCallbacks*)std::get<1>(pair)->getUserPointer();
			btRigidBody* body0 = (btRigidBody*)std::get<0>(pair);
			btRigidBody* body1 = (btRigidBody*)std::get<1>(pair);
			btPersistentManifold* manifold = (btPersistentManifold*)std::get<2>(pair);
			if (Callbacks->enter)
				Callbacks->enter(body0, body1, Callbacks2->physicsObject, manifold);

			if (Callbacks2->enter)
				Callbacks2->enter(body1, body0, Callbacks->physicsObject, manifold);

			mCollisionPairs.emplace(pair);
		}
		else
		{
			Data::CollisionCallbacks* Callbacks = (Data::CollisionCallbacks*)std::get<0>(pair)->getUserPointer();
			Data::CollisionCallbacks* Callbacks2 = (Data::CollisionCallbacks*)std::get<1>(pair)->getUserPointer();
			btRigidBody* body0 = (btRigidBody*)std::get<0>(pair);
			btRigidBody* body1 = (btRigidBody*)std::get<1>(pair);
			btPersistentManifold* manifold = (btPersistentManifold*)std::get<2>(pair);
			if (Callbacks && Callbacks2)
			{

				if (Callbacks->exit)
					Callbacks->exit(body0, body1, Callbacks2->physicsObject, manifold);


				if (Callbacks2->exit)
					Callbacks2->exit(body1, body0, Callbacks->physicsObject, manifold);
			}
		}
	}
	mCollisionPairs = NewCollisions;
}

void BulPhysicsEngine::addBoxCollider(btRigidBody& pCollider)
{
	std::unique_lock lock(mMutex);
	PhysicWorld::addBoxCollider(pCollider);
	mColliders.push_back(&pCollider);
}

void BulPhysicsEngine::removeCollider(btRigidBody& pCollider)
{
	std::unique_lock lock(mMutex);
	PhysicWorld::removeCollider(pCollider);
	mColliders.remove(&pCollider);
}

void BulPhysicsEngine::reset()
{
	for (btRigidBody* collider : mColliders)
		removeRigidBody(collider);
	
	mColliders.clear();
	mCollisionPairs.clear();
}

void BulPhysicsEngine::lock()
{
	std::unique_lock lock(mMutex);
}