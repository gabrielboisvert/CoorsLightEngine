#pragma once
#include "Game/Component/CPLight.h"
#include <Rendering/Data/UniformData.h>

namespace Game::Component
{
	class CPPointLight : public CPLight
	{
	public:
		float mRadius = 5;
		float mBrightness = 3;

		AComponent* clone() override;
		~CPPointLight() override = default;

		virtual void fillUniform(const int& pIdx, Rendering::Data::UniformDataLight& pData) override;
		void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& pWriter) override;
	};
}