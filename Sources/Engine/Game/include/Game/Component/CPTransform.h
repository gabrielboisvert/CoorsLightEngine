#pragma once
#include "Game/Component/AComponent.h"
#include "Maths/FTransform.h"

namespace Game::Component
{
	class CPTransform : public AComponent, public Maths::FTransform
	{
		public:
			AComponent* clone() override;
			~CPTransform() override = default;
	};
}