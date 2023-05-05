#pragma once
#include "Game/Component/AComponent.h"
#include "Maths/FVector3.h"
#include "Rendering/Data/UniformData.h"

namespace Game::Component
{
	class CPLight : public AComponent
	{
	public:
		Maths::FVector3 mColor = {1, 1, 1};
		float mAttenuatuion = 0.25;

		CPLight();
		virtual AComponent* clone() override;
		virtual ~CPLight() override;

		virtual void fillUniform(const int& pIdx, Rendering::Data::UniformDataLight& pData) {};
		virtual void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& pWriter) override;
	};
}