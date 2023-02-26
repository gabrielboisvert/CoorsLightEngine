#pragma once
#include "Game/Component/CPModel.h"
#include "EngineCore/Service/ServiceLocator.h"
#include "EngineCore/ResourceManagement/ResourceManager.h"

using namespace Game::Component;

AComponent* CPModel::clone()
{
	return new CPModel(*this);
}


void CPModel::setModel(const std::string& pName, const char* pModel)
{
	mModel = service(EngineCore::ResourcesManagement::ResourceManager).get<Rendering::Resources::Model>(pModel);
	mName = pName;
	mPath = pModel;

	if (mModel == nullptr)
	{
		mName = "Unknown";
		mPath = "";
	}
}

void CPModel::draw(void* pCmd)
{
	if (mModel != nullptr)
		mModel->draw(pCmd);
}