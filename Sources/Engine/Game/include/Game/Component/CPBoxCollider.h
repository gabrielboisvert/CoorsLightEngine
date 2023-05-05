#pragma once
#include "ACollider.h"
#include "Physics/Resources/bulBoxCollider.h"

namespace Game::Component
{
	class CPBoxCollider : public ACollider, public Physics::Resources::BulBoxCollider
	{
		public:
			CPBoxCollider(Maths::FVector3 pPosition, Maths::FVector3 pSize, void* pActor);
			void UpdateTransform(Maths::FTransform& pTransform) override;
			void recreateCollider() override;
			Maths::FVector3 getPhysicsPosition() override;
			Maths::FQuaternion getPhysicsRotation() override;
			AComponent* clone() override;
			~CPBoxCollider() override = default;


			void OnAnyCollisionEnter(void* pRigidbody, void* pOtherCollider, btPersistentManifold* persistent = nullptr) override;
			void OnAnyCollisionStay(void* pRigidbody, void* pOtherCollider, btPersistentManifold* persistent = nullptr) override;
			void OnAnyCollisionExit(void* pRigidbody, void* pOtherCollider, btPersistentManifold* persistent = nullptr) override;

			void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& pWriter) override;

			virtual void addForce(Maths::FVector3 Force) override;

			virtual void clearForces() override;

			virtual void setLinearVelocity(Maths::FVector3 pVelocity) override;
			virtual void setAngularVelocity(Maths::FVector3 pVelocity) override;

			virtual void clearLinearVelocity() override;
	};
}

