#pragma once
#include "Maths/FVector3.h"
#include "Maths/FMatrix4.h"
#include "Physics/Data/TypeRigidBody.h"

namespace Physics::Resources
{
	class IRigidbody
	{
		virtual void setType(Data::TypeRigidBody type) = 0;

		virtual Maths::FVector3 getPositionFromPhysics() = 0;
	}; 
}