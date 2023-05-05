#pragma once
#include "Game/Component/CPPointLight.h"
#include <Game/Utils/ComponentType.h>
#include "Game/Data/Actor.h"

using namespace Game::Component;

AComponent* CPPointLight::clone()
{
	return new CPPointLight(*this);
}

void CPPointLight::fillUniform(const int& pIdx, Rendering::Data::UniformDataLight& pData)
{
	Game::Data::Actor* actor = (Game::Data::Actor*)mOwner;
	if (actor == nullptr)
		return;

	pData.light[pIdx].mLightType = Rendering::Data::POINT_LIGHT;
	pData.light[pIdx].mColor = mColor;
	pData.light[pIdx].mAtenuation = mAttenuatuion;
	pData.light[pIdx].mRadius = mRadius;
	pData.light[pIdx].mBrightness = mBrightness;
	pData.light[pIdx].mPos = actor->getTransform()->mWorldPosition;
}

void CPPointLight::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& pWriter)
{
	pWriter.StartObject();

	pWriter.Key("Type");
	pWriter.Int((int)Game::Utils::ComponentType::PointLight);

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

	pWriter.Key("Radius");
	pWriter.Double(mRadius);

	pWriter.Key("Brightness");
	pWriter.Double(mBrightness);

	pWriter.EndObject();
}