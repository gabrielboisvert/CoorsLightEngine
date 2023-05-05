#pragma once
#include "AComponent.h"
#include "Maths/FVector3.h"
#include "Maths/FTransform.h"
#include "Physics/Data/TypeRigidBody.h"

namespace Game::Component
{
	class ACollider : public AComponent
	{
	public:
		virtual AComponent* clone() = 0;
		virtual void recreateCollider() = 0;
		virtual void UpdateTransform(Maths::FTransform& pTransform) = 0;
		virtual Maths::FVector3 getPhysicsPosition() = 0;
		virtual Maths::FQuaternion getPhysicsRotation() = 0;

		Physics::Data::TypeRigidBody mColliderType = Physics::Data::TypeRigidBody::E_DYNAMIC;

		void OnCollisionEnter(void* pOtherActor, ACollider* pOtherCollider);
		void OnTriggerEnter(void* pOtherActor, ACollider* pOtherCollider);

		void OnCollisionStay(void* pOtherActor, ACollider* pOtherCollider);
		void OnTriggerStay(void* pOtherActor, ACollider* pOtherCollider);

		void OnCollisionExit(void* pOtherActor, ACollider* pOtherCollider);
		void OnTriggerExit(void* pOtherActor, ACollider* pOtherCollider);

		virtual void addForce(Maths::FVector3 Force) = 0;

		virtual void clearForces() = 0;

		virtual void setLinearVelocity(Maths::FVector3 pVelocity) = 0;

		virtual void setAngularVelocity(Maths::FVector3 pVelocity) = 0;

		virtual void clearLinearVelocity() = 0;

		virtual void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& pWriter) = 0;
	};
}