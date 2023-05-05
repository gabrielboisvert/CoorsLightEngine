#pragma once
#include "Game/Component/CPLight.h"
#include <Rendering/Data/UniformData.h>

namespace Game::Component
{
	class CPDirLight : public CPLight
	{
	public:
		AComponent* clone() override;
		~CPDirLight() override = default;

		virtual void fillUniform(const int& pIdx, Rendering::Data::UniformDataLight& pData) override;
		void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& pWriter) override;
	};
}