#pragma once

namespace Game::Component
{
	class AComponent
	{
		public:
			virtual AComponent* clone() = 0;
			virtual ~AComponent() = default;
	};
}