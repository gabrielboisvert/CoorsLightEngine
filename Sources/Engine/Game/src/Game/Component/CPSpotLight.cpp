#pragma once
#include "Game/Component/CPSpotLight.h"
#include <Game/Utils/ComponentType.h>
#include "Game/Data/Actor.h"

using namespace Game::Component;

AComponent* CPSpotLight::clone()
{
	return new CPSpotLight(*this);
}

void CPSpotLight::updateRad()
{
	mCutOffRad = Maths::degreesToRadians(mCutOff);
	mOuterCutOffRad = Maths::degreesToRadians(mOuterCutOff);
}

void CPSpotLight::fillUniform(const int& pIdx, Rendering::Data::UniformDataLight& pData)
{
	Game::Data::Actor* actor = (Game::Data::Actor*)mOwner;
	if (actor == nullptr)
		return;

	pData.light[pIdx].mLightType = Rendering::Data::SPOT_LIGHT;
	pData.light[pIdx].mColor = mColor;
	pData.light[pIdx].mAtenuation = mAttenuatuion;
	pData.light[pIdx].mPos = actor->getTransform()->mWorldPosition;
	pData.light[pIdx].mCutOff = mCutOffRad;
	pData.light[pIdx].mOuterCutOff = mOuterCutOffRad;
}

void CPSpotLight::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& pWriter)
{
	pWriter.StartObject();

	pWriter.Key("Type");
	pWriter.Int((int)Game::Utils::ComponentType::SpotLight);

	pWriter.Key("Color");
	pWriter.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatSingleLineArray);
	pWriter.StartArray();
		pWriter.Double(mColor.x);
		pWriter.Double(mColor.y);
		pWriter.Double(mColor.z);
	pWriter.EndArray();
	pWriter.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatDefault);

	pWriter.Key("Attenuation");
	pWriter.Double(mAttenuatuion);

	pWriter.Key("CutOff");
	pWriter.Double(mCutOff);

	pWriter.Key("OuterCutOff");
	pWriter.Double(mOuterCutOff);

	pWriter.EndObject();
}