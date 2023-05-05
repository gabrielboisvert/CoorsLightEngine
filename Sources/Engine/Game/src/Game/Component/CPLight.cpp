#pragma once
#include "Game/Component/CPLight.h"
#include <Game/Utils/ComponentType.h>
#include "EngineCore/Service/ServiceLocator.h"
#include "Game/SceneSys/SceneManager.h"

using namespace Game::Component;

CPLight::CPLight()
{
	service(Game::SceneSys::SceneManager).mCurrentScene->mLights.push_back(this);
}

CPLight::~CPLight()
{
	service(Game::SceneSys::SceneManager).mCurrentScene->mLights.remove(this);
}

AComponent* CPLight::clone()
{
	return new CPLight(*this);
}

void CPLight::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& pWriter)
{
	pWriter.StartObject();

	pWriter.Key("Type");
	pWriter.Int((int)Game::Utils::ComponentType::Light);

	pWriter.EndObject();
}