#pragma once
#include "Game/Component/AComponent.h"
#include "Rendering/Resources/Model.h"

namespace Game::Component
{
	class CPModel : public AComponent
	{
		public:
			Rendering::Resources::Model* mModel = nullptr;
			std::string mName = "Empty";
			std::string mPath = "";

			AComponent* clone() override;
			void setModel(const std::string& pName, const char* pModel);
			void draw(void* pCmd);
			~CPModel() override = default;
	};
}