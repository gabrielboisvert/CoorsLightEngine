#pragma once

#include <iostream>
#include "Game/Component/CPModel.h"
#include "EngineCore/Service/ServiceLocator.h"
#include "EngineCore/ResourceManagement/ResourceManager.h"
#include <Game/Utils/ComponentType.h>
#include "Tools/Utils/PathParser.h"
#include <filesystem>
#include "Game/SceneSys/SceneManager.h"
#ifdef NSHIPPING
#include "Editor/Widget/WidgetEditor.h"
#include "EngineCore/Service/ServiceLocator.h"
#include "Editor/Utils/Utils.h"
#endif

using namespace Game::Component;

CPModel::CPModel()
{
	Rendering::Resources::Material* master = 
		service(EngineCore::ResourcesManagement::ResourceManager).create<Rendering::Resources::Material>("Resources/Engine/Material/WorldMaterial.mat", "Resources/Engine/Material/WorldMaterial.mat");

	mMat = master->getInstance();
#ifdef NSHIPPING
	mMatEditor = master->getInstance();
#endif

	mNameMat = "WorldMaterial";
	mPathMat = "Resources/Engine/Material/WorldMaterial.mat";
}

AComponent* CPModel::clone()
{
	return new CPModel(*this);
}

void CPModel::setModel(const std::string& pName, const char* pModel)
{
	mModel = service(EngineCore::ResourcesManagement::ResourceManager).create<Rendering::Resources::Model>(pModel, pModel);
	mName = pName;
	mPath = pModel;

	if (mModel == nullptr)
	{
		mName = "Unknown";
		mPath = "";
		return;
	}
}

void CPModel::setModelWithPath(const char* pModel)
{
	if (pModel == nullptr)
	{
		mModel = nullptr;
		mName = "";
		mPath = "";
		return;
	}

	mModel = service(EngineCore::ResourcesManagement::ResourceManager).create<Rendering::Resources::Model>(pModel, pModel);
	mPath = pModel;

	if (mModel == nullptr)
		mPath = "";
}

void Game::Component::CPModel::setModelWithPathLua(const char* pPath)
{
	std::string currentPath = service(Game::SceneSys::SceneManager).mProjectPath + "/";
	mModel = service(EngineCore::ResourcesManagement::ResourceManager).create<Rendering::Resources::Model>(currentPath + pPath, currentPath + pPath);
	mPath = currentPath + pPath;

	if (mModel == nullptr)
		mPath = "";
}

Rendering::Resources::Model* CPModel::getModel() const
{
	return mModel;
}

void CPModel::setMat(const std::string& pName, const char* pMat)
{
	if (mNameMat == pName)
		return;

	Rendering::Resources::Material* master = service(EngineCore::ResourcesManagement::ResourceManager).create<Rendering::Resources::Material>(pMat, pMat);

	mMat = master->getInstance();
#ifdef NSHIPPING
	mMatEditor = master->getInstance();
#endif

	mNameMat = pName;
	mPathMat = pMat;

	if (mMat == nullptr)
	{
		mNameMat = "Unknown";
		mPathMat = "";
	}

	defaultMat = false;
}

void CPModel::setMat(const char* pMat)
{
	if (pMat == nullptr)
	{
		if (defaultMat)
			return;

		Rendering::Resources::Material* master =
			service(EngineCore::ResourcesManagement::ResourceManager).create<Rendering::Resources::Material>("Resources/Engine/Material/WorldMaterial.mat", "Resources/Engine/Material/WorldMaterial.mat");

		mMat = master->getInstance();
#ifdef NSHIPPING
		mMatEditor = master->getInstance();
#endif

		mNameMat = "WorldMaterial";
		mPathMat = "Resources/Engine/Material/WorldMaterial.mat";
		defaultMat = true;
		return;
	}

	Rendering::Resources::Material* master = service(EngineCore::ResourcesManagement::ResourceManager).create<Rendering::Resources::Material>(pMat, pMat);

	mMat = master->getInstance();
#ifdef NSHIPPING
	mMatEditor = master->getInstance();
#endif

	mPathMat = pMat;

	if (mMat == nullptr)
		mPathMat = "";

	defaultMat = false;
}

void CPModel::setMatWithPathLua(const char* pPath)
{
	std::string currentPath = service(Game::SceneSys::SceneManager).mProjectPath + "/";
	mPath = currentPath + pPath;

	Rendering::Resources::Material* master = service(EngineCore::ResourcesManagement::ResourceManager).create<Rendering::Resources::Material>(mPath, mPath);

	mMat = master->getInstance();
#ifdef NSHIPPING
	mMatEditor = master->getInstance();
#endif
}

Rendering::Resources::Material* CPModel::getMat() const
{
	return mMat;
}

void CPModel::updateMat(Maths::FMatrix4& pVP, Maths::FMatrix4& pModel)
{
	if (mMat == nullptr)
		return;

	mMat->setModelData(pVP, pModel);
}

void CPModel::updateMatEditor(Maths::FMatrix4& pVP, Maths::FMatrix4& pModel)
{
#ifdef NSHIPPING
	if (mMatEditor == nullptr)
		return;

	mMatEditor->setModelData(pVP, pModel);
#endif
}

void CPModel::draw(void* pCmd, Rendering::Data::Material* pPipeLine)
{
	if (mMat != nullptr && pPipeLine != nullptr)
	{
		mMat->bindMat(*pPipeLine);
		pPipeLine->bindPipeLine((VkCommandBuffer)pCmd);
	}

	if (mModel != nullptr)
		mModel->draw(pCmd);
}

void CPModel::drawEditor(void* pCmd, Rendering::Data::Material* pPipeLine)
{
#ifdef NSHIPPING
	if (mMatEditor != nullptr && pPipeLine != nullptr)
	{
		mMatEditor->bindMat(*pPipeLine);
		pPipeLine->bindPipeLine((VkCommandBuffer)pCmd);
	}

	if (mModel != nullptr)
		mModel->draw(pCmd);
#endif
}

void CPModel::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& pWriter)
{
	pWriter.StartObject();

	pWriter.Key("Type");
	pWriter.Int((int)Game::Utils::ComponentType::MeshRenderer);

	pWriter.Key("Name");
	pWriter.String(mName.c_str());
	
	#ifdef NSHIPPING
		std::string currentPath = Editor::Utils::qStringToStdString(service(Editor::Widget::WidgetEditor).mLocation.mFolder);
		pWriter.Key("Path");
		pWriter.String(mPath.empty() ? "": mPath.substr(currentPath.length()).c_str());
	#endif

	pWriter.Key("DefaultMat");
	pWriter.Bool(defaultMat);

	pWriter.Key("Mat");
	pWriter.String(mNameMat.c_str());

	#ifdef NSHIPPING
		pWriter.Key("MatPath");
		if (!defaultMat)
			pWriter.String(mPathMat.empty() ? "" : mPathMat.substr(currentPath.length()).c_str());
		else
			pWriter.String(mPathMat.c_str());
	#endif

	pWriter.EndObject();
}