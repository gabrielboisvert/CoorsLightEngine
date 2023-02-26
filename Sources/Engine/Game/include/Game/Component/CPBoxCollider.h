#pragma once
#include "AComponent.h"
#include "Physics/Resources/bulBoxCollider.h"

namespace Game::Component
{
	class CPBoxCollider : public AComponent, public Physics::Resources::BulBoxCollider
	{
		public:
			CPBoxCollider(Maths::FVector3 pPosition, Maths::FVector3 pSize);

			AComponent* clone() override;
			~CPBoxCollider() override = default;
	};
}

