#pragma once
#include "Game/Component/CPLight.h"
#include <Rendering/Data/UniformData.h>

namespace Game::Component
{
	class CPSpotLight : public CPLight
	{
	public:
		float mCutOff = 12.5f;
		float mCutOffRad = 0;
		float mOuterCutOff = 17.5;
		float mOuterCutOffRad = 0;

		AComponent* clone() override;
		~CPSpotLight() override = default;

		void updateRad();

		virtual void fillUniform(const int& pIdx, Rendering::Data::UniformDataLight& pData) override;
		void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& pWriter) override;
	};
}