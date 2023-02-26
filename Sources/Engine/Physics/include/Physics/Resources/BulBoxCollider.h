#pragma once
#include "Physics/Resources/IRigidbody.h"
#include "Physics/Core/BulPhysicsEngine.h"
#include <Bullet/btBulletCollisionCommon.h>
#include <Bullet/BulletDynamics/Dynamics/btRigidBody.h>
#include "Maths/FVector3.h"
#include "Maths/FQuaternion.h"
#include "Physics/Data/CollisionCallbacks.h"
#include "Physics/Data/TypeRigidBody.h"

namespace Physics::Resources
{
	class BulBoxCollider : public IRigidbody
	{
		public:
			btRigidBody* mCollider = nullptr;
			btBoxShape mBox;
			btDefaultMotionState mMotion;
			int mDefaultflag = -1;

			Data::TypeRigidBody mType = Data::TypeRigidBody::E_DYNAMIC;
			Maths::FVector3 mPosition;
			Maths::FVector3 mSize;
			Maths::FQuaternion mRotation;
			float mMass = 10;

			Data::CollisionCallbacks mCallback;

			BulBoxCollider(Maths::FVector3 pPosition, Maths::FVector3 pSize);
			BulBoxCollider(BulBoxCollider& pOther);
			~BulBoxCollider();
			
			Maths::FVector3 getPositionFromPhysics(); // From Bullet -> Gameobject
		
			void setTransform(Maths::FVector3 pPosition, Maths::FQuaternion pRotation, Maths::FVector3 pSize);

			void setType(Data::TypeRigidBody pType) override;

			void updateMass(double pValue);

			void updateTransform();

			void createCollider();
	};
}