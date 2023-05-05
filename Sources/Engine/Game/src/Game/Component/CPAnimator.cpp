#include "Game/Component/CPAnimator.h"
#include <Game/Utils/ComponentType.h>
#include <EngineCore/Service/ServiceLocator.h>
#ifdef NSHIPPING
	#include "Editor/Widget/WidgetEditor.h"
#endif
#include "Game/SceneSys/SceneManager.h"
#include "Game/Data/Actor.h"
#include "EngineCore/ResourceManagement/ResourceManager.h"
#include <filesystem>
#include "Tools/Utils/PathParser.h"

using namespace Game::Component;

CPAnimator::~CPAnimator()
{
}

AComponent* CPAnimator::clone()
{
	return new CPAnimator(*this);
}

void CPAnimator::updateModel()
{
	Game::Data::Actor* actor = (Game::Data::Actor*)mOwner;
	if (actor == nullptr)
		return;


	if (actor->mComponents.find(Utils::ComponentType::MeshRenderer) == actor->mComponents.end())
	{
		cpModelIdx = -1;
		return;
	}

	int i = 1;
	for (Game::Component::AComponent* model : actor->mComponents[Utils::ComponentType::MeshRenderer])
	{
		if (i == cpModelIdx)
			mCPModel = (Game::Component::CPModel*)model;
		i++;
	}

	if (mCPModel == nullptr)
		cpModelIdx = -1;
}

void CPAnimator::updateAnimationFromModel()
{
	if (!mPath.empty())
		setAnimation(mPath.c_str());
}

void CPAnimator::setAnimation(const std::string& pName, const char* pPath)
{
	if (mCPModel == nullptr)
	{
		if (cpModelIdx != -1)
			updateModel();
		if (mCPModel == nullptr)
			return;
	}

	mCurrentAnimation = service(EngineCore::ResourcesManagement::ResourceManager).create<Rendering::Resources::Animation>("Anim" + std::string(pPath), pPath, mCPModel->mModel);
	setAnimationClip(mCurrentAnimation);
	mName = pName;
	mPath = pPath;
}

void CPAnimator::setAnimation(const char* pPath)
{
	if (pPath == nullptr)
	{
		mCurrentAnimation = nullptr;
		mName = "";
		mPath = "";
		return;
	}
	
	if (mCPModel == nullptr)
	{
		if (cpModelIdx != -1)
			updateModel();
		if (mCPModel == nullptr)
			return;
	}

	mCurrentAnimation = service(EngineCore::ResourcesManagement::ResourceManager).create<Rendering::Resources::Animation>("Anim" + std::string(pPath), pPath, mCPModel->mModel);
	setAnimationClip(mCurrentAnimation);
	mPath = pPath;
}

void CPAnimator::setAnimationWithLua(const char* pPath)
{
	if (mCPModel == nullptr)
	{
		if (cpModelIdx != -1)
			updateModel();
		if (mCPModel == nullptr)
			return;
	}

	std::string currentPath = service(Game::SceneSys::SceneManager).mProjectPath + "/";
	mPath = currentPath + pPath;

	mCurrentAnimation = service(EngineCore::ResourcesManagement::ResourceManager).create<Rendering::Resources::Animation>("Anim" + std::string(pPath), mPath, mCPModel->mModel);
	setAnimationClip(mCurrentAnimation);
}

void CPAnimator::OnStart()
{
	if (!playOnAwake || mCurrentAnimation == nullptr)
		return;
	reset();
	playing = true;
}

void CPAnimator::OnUpdate(float pDeltaTime)
{
	if (mCPModel == nullptr || !playing)
		return;

	updateAnimation(pDeltaTime, loop);


	for (int i = 0; i < mFinalBoneMatrices.size(); ++i)
	{
		mCPModel->mMat->mUnibuffer.mData.mFinalBonesMatrices[i] = mFinalBoneMatrices[i];
#ifdef NSHIPPING
		mCPModel->mMatEditor->mUnibuffer.mData.mFinalBonesMatrices[i] = mFinalBoneMatrices[i];
#endif
	}

	mCPModel->mMat->mUnibuffer.mData.mHasAnimation = 1;
#ifdef NSHIPPING
	mCPModel->mMatEditor->mUnibuffer.mData.mHasAnimation = 1;
#endif
}

void CPAnimator::OnPaused()
{
	playing = !playing;
}

void CPAnimator::OnDestroy()
{
	reset();
	OnUpdate(0);
	playing = false;
}

void CPAnimator::play()
{
	paused = false;
	playing = true;
	reset();
}

void CPAnimator::pause()
{
	paused = true;
	playing = false;
}

void CPAnimator::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& pWriter)
{
	pWriter.StartObject();

	pWriter.Key("Type");
	pWriter.Int((int)Game::Utils::ComponentType::Animator);

	pWriter.Key("Name");
	pWriter.String(mName.c_str());

#ifdef NSHIPPING
	std::string currentPath = Editor::Utils::qStringToStdString(service(Editor::Widget::WidgetEditor).mLocation.mFolder);
	pWriter.Key("Path");
	pWriter.String(mPath.empty() ? "" : mPath.substr(currentPath.length()).c_str());
#endif

	pWriter.Key("PlayOnAwake");
	pWriter.Bool(playOnAwake);

	pWriter.Key("Loop");
	pWriter.Bool(loop);

	pWriter.Key("CPModelIdx");
	pWriter.Int(cpModelIdx);

	pWriter.EndObject();
}