#pragma once
#include "Game/Component/CPDirLight.h"
#include <Game/Utils/ComponentType.h>
#include "Game/Data/Actor.h"

using namespace Game::Component;

AComponent* CPDirLight::clone()
{
	return new CPDirLight(*this);
}

void CPDirLight::fillUniform(const int& pIdx, Rendering::Data::UniformDataLight& pData)
{
	Game::Data::Actor* actor = (Game::Data::Actor*)mOwner;
	if (actor == nullptr)
		return;

	pData.light[pIdx].mLightType = Rendering::Data::DIR_LIGHT;
	pData.light[pIdx].mDirection = (actor->getTransform()->mWorldRotation * Maths::FVector3::Bottom).normalize();
	pData.light[pIdx].mColor = mColor;
	pData.light[pIdx].mAtenuation = mAttenuatuion;
}

void CPDirLight::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& pWriter)
{
	pWriter.StartObject();

	pWriter.Key("Type");
	pWriter.Int((int)Game::Utils::ComponentType::DirLight);

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

	pWriter.EndObject();
}