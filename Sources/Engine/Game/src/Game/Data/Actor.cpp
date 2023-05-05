#include "Game/Data/Actor.h"
#include "Game/Component/CPModel.h"
#include "Game/Component/CPUI.h"
#include "Game/Component/CPCamera.h"
#include "Game/Component/CPParticle.h"
#include "Game/Component/ACollider.h"
#include "EngineCore/Core/EngineApp.h"

using namespace Game::Data;

int Actor::counterID = 0;

Actor::Actor()
{
	id = ++counterID;
	mComponents[Utils::ComponentType::Transform].push_back(new Component::CPTransform());
}

Actor::Actor(const Actor& pOther)
{
	id = ++counterID;

	auto it = pOther.mComponents.begin();
	while (it != pOther.mComponents.end())
	{
		for (Game::Component::AComponent* Components : it->second)
			mComponents[it->first].push_back(Components->clone());

		++it;
	}
}

Actor::~Actor()
{
	if (mComponents.find(Utils::ComponentType::Transform) != mComponents.end()) //delete notifHandler BUGFIX with destoying child then parent
	{
		for (Game::Component::AComponent* Transform : mComponents[Utils::ComponentType::Transform])
		{
			if (((Game::Component::CPTransform*)Transform)->hasParent())
				((Game::Component::CPTransform*)Transform)->mParent->notifier.removeNotificationHandler(((Game::Component::CPTransform*)Transform)->notificationHandlerID);
		}
	}

	auto it = mComponents.begin();
	while (it != mComponents.end())
	{
		for (Game::Component::AComponent* Component : it->second)
		{
			if (Game::Component::CPCamera::mWorldCamera == Component)
				Game::Component::CPCamera::mWorldCamera = nullptr;
			delete Component;
		}

		++it;
	}
}

Game::Component::CPTransform* Actor::getTransform(unsigned pIndex)
{
	if (mComponents[Utils::ComponentType::Transform].size() <= pIndex)
		return nullptr;

	return (Game::Component::CPTransform*)mComponents[Utils::ComponentType::Transform].front() + pIndex;
}

Game::Component::CPTransform* Game::Data::Actor::setTransform(unsigned pIndex)
{
	if (mComponents[Utils::ComponentType::Transform].size() <= pIndex)
		return nullptr;

	mShouldUpdatePhysics = true;
	return (Game::Component::CPTransform*)mComponents[Utils::ComponentType::Transform].front() + pIndex;
}

Game::Component::CPModel* Actor::getModel(unsigned pIndex)
{
	if (mComponents[Utils::ComponentType::MeshRenderer].size() <= pIndex)
		return nullptr;

	return (Game::Component::CPModel*)mComponents[Utils::ComponentType::MeshRenderer].front() + pIndex;
}

Game::Component::ACollider* Game::Data::Actor::getCollider(unsigned pIndex)
{
	if (mComponents[Utils::ComponentType::Collider].size() <= pIndex)
		return nullptr;

	return (Game::Component::ACollider*)mComponents[Utils::ComponentType::Collider].front() + pIndex;
}

Game::Component::CPCamera* Game::Data::Actor::getCamera(unsigned pIndex)
{
	if (mComponents[Utils::ComponentType::Camera].size() <= pIndex)
		return nullptr;

	return (Game::Component::CPCamera*)mComponents[Utils::ComponentType::Camera].front() + pIndex;
}

sol::table Game::Data::Actor::getScript(std::string pScriptName)
{
	std::list<Game::Component::AComponent*> mScripts = mComponents[Utils::ComponentType::Script];
	std::list<Game::Component::AComponent*>::iterator it;
	for (it = mScripts.begin(); it != mScripts.end(); ++it)
	{
		Game::Component::CPScript* Script = (Game::Component::CPScript*)*it;
		if (pScriptName.compare(Script->mName) == 0)
		{
			return Script->mLuaTable;
		}
	}
	return sol::nil;
}

Game::Component::CPSound* Game::Data::Actor::getSoundByName(std::string pName)
{
	if (mComponents[Utils::ComponentType::Sound].size() <= 0)
		return nullptr;
	std::list<Game::Component::AComponent*> Sounds = mComponents[Utils::ComponentType::Sound];

	for (Game::Component::AComponent* sound : Sounds)
	{
		Game::Component::CPSound* Casted = (Game::Component::CPSound*)sound;
		if (Casted->mName.compare(pName) == 0)
		{
			return Casted;
		}
	}

	return nullptr;
}

Game::Component::CPSound* Game::Data::Actor::getSound(unsigned pIndex)
{
	if (mComponents[Utils::ComponentType::Sound].size() <= pIndex)
		return nullptr;

	return (Game::Component::CPSound*)mComponents[Utils::ComponentType::Sound].front() + pIndex;
}

Game::Component::CPUI* Actor::getUI(unsigned pIndex)
{
	if (mComponents[Utils::ComponentType::UI].size() <= pIndex)
		return nullptr;

	return (Game::Component::CPUI*)mComponents[Utils::ComponentType::UI].front() + pIndex;
}

Game::Component::CPParticle* Actor::getParticle(unsigned pIndex)
{
	if (mComponents[Utils::ComponentType::Particle].size() <= pIndex)
		return nullptr;

	return (Game::Component::CPParticle*)mComponents[Utils::ComponentType::Particle].front() + pIndex;
}

Game::Component::CPAnimator* Actor::getAnimator(unsigned pIndex)
{
	if (mComponents[Utils::ComponentType::Animator].size() <= pIndex)
		return nullptr;

	return (Game::Component::CPAnimator*)mComponents[Utils::ComponentType::Animator].front() + pIndex;
}

void Actor::addComponent(Utils::ComponentType pType, Component::AComponent* pComponent)
{
	if (pComponent == nullptr)
		return;

	pComponent->mOwner = this;

	if (pType == Utils::ComponentType::Camera)
		if (Game::Component::CPCamera::mWorldCamera == nullptr)
			Game::Component::CPCamera::mWorldCamera = (Component::CPCamera*)pComponent;

	std::unique_lock lock(mMutex);
	mComponents[pType].push_back(pComponent);
}

void Game::Data::Actor::removeComponent(Utils::ComponentType pType, Component::AComponent* pComponent)
{
	std::unique_lock lock(mMutex);
	mComponents[pType].remove(pComponent);

	if (pType == Utils::ComponentType::Camera)
		if (Game::Component::CPCamera::mWorldCamera != nullptr)
			Game::Component::CPCamera::mWorldCamera = nullptr;

	delete pComponent;
}

void Actor::draw(void* pCmd, Rendering::Data::Material& pPipeLine)
{
	if (mComponents.find(Utils::ComponentType::MeshRenderer) == mComponents.end())
		return;

	for (Game::Component::AComponent* model : mComponents[Utils::ComponentType::MeshRenderer])
		((Game::Component::CPModel*)model)->draw(pCmd, &pPipeLine);
}

void Actor::drawUI(void* pCmd, Rendering::Data::Material& pPipeLine, Rendering::Data::Material& pPipeLineText)
{
	if (mComponents.find(Utils::ComponentType::UI) == mComponents.end())
		return;

	for (Game::Component::AComponent* ui : mComponents[Utils::ComponentType::UI])
		((Game::Component::CPUI*)ui)->draw(pCmd, &pPipeLine, &pPipeLineText);
}

void Actor::drawParticle(void* pCmd, Rendering::Data::Material& pPipeLine, Maths::FMatrix4& pViewProj, Maths::FMatrix4& pView)
{
	if (mComponents.find(Utils::ComponentType::Particle) == mComponents.end())
		return;

	for (Game::Component::AComponent* particle : mComponents[Utils::ComponentType::Particle])
		((Game::Component::CPParticle*)particle)->draw(pCmd, pPipeLine, pViewProj, pView);
}

void Actor::containMouse(Maths::FVector2 p)
{
	if (mComponents.find(Utils::ComponentType::UI) == mComponents.end())
		return;

	for (Game::Component::AComponent* ui : mComponents[Utils::ComponentType::UI])
		((Game::Component::CPUI*)ui)->contain(p);
}

Rendering::Resources::UIResource::IUIResource* Actor::containMouseClick(Maths::FVector2 p)
{
	if (mComponents.find(Utils::ComponentType::UI) == mComponents.end())
		return nullptr;

	Rendering::Resources::UIResource::IUIResource* fin = nullptr;
	for (Game::Component::AComponent* ui : mComponents[Utils::ComponentType::UI])
	{
		Rendering::Resources::UIResource::IUIResource* tmp = ((Game::Component::CPUI*)ui)->containClick(p);
		if (tmp)
			fin = tmp;
	}
	return fin;
}

void Actor::updateUIProj(Maths::FMatrix4 uiProj, int width, int height)
{
	if (mComponents.find(Utils::ComponentType::UI) == mComponents.end())
		return;

	for (Game::Component::AComponent* ui : mComponents[Utils::ComponentType::UI])
		((Game::Component::CPUI*)ui)->updateUI(uiProj, width, height);
}

void Actor::updateWorldTransform(Component::CPTransform* pTransform)
{
	std::unique_lock lock(mMutex);
	if (mShouldUpdatePhysics)
		SetPhysicsPosition();

	if (mComponents.find(Utils::ComponentType::Collider) != mComponents.end() && mComponents.find(Utils::ComponentType::Collider)->second.size() != 0)
	{
		Game::Component::ACollider* body = ((Game::Component::ACollider*)mComponents[Utils::ComponentType::Collider].front());
		pTransform->mLocalPosition = body->getPhysicsPosition();
		pTransform->mLocalRotation = body->getPhysicsRotation();
	}
	pTransform->updateLocalMatrix();

	mValueChanged.dispatch();
	mValueChangedFromEditor.dispatch(pTransform->getWorldMatrix());
}

void Game::Data::Actor::SetPhysicsPosition()
{
	if (mComponents.find(Utils::ComponentType::Collider) != mComponents.end())
	{
		Game::Component::ACollider* body = ((Game::Component::ACollider*)mComponents[Utils::ComponentType::Collider].front());
		body->UpdateTransform(*getTransform());
	}
}

void Game::Data::Actor::updateWorldTransformEditor(Component::CPTransform* pTransform)
{
	std::unique_lock lock(mMutex);

	if (mComponents.find(Utils::ComponentType::Collider) != mComponents.end())
		for (Game::Component::AComponent* Collider : mComponents[Utils::ComponentType::Collider])
			((Game::Component::ACollider*)Collider)->UpdateTransform(*pTransform);

	pTransform->updateLocalMatrix();
	mValueChangedFromEditor.dispatch(pTransform->getWorldMatrix());
}

void Actor::OnAwake()
{
	// Call Awake on all components
	auto it = mComponents.begin();

	while (it != mComponents.end())
	{
		for (Game::Component::AComponent* Component : it->second)
		{
			Component->OnAwake();
		}

		++it;
	}
}

void Actor::OnStart()
{
	// Call Start on all components
	auto it = mComponents.begin();

	while (it != mComponents.end())
	{
		for (Game::Component::AComponent* Component : it->second)
		{
			Component->OnStart();
		}

		++it;
	}
}

void Actor::OnPaused()
{
	// Call Awake on all components
	auto it = mComponents.begin();

	while (it != mComponents.end())
	{
		for (Game::Component::AComponent* Component : it->second)
		{
			Component->OnPaused();
		}

		++it;
	}
}

void Actor::OnDestroy()
{
	// Call Destroy on all components
	auto it = mComponents.begin();
	while (it != mComponents.end())
	{
		for (Game::Component::AComponent* Component : it->second)
		{
			Component->OnDestroy();
		}

		++it;
	}
}

void Actor::OnUpdate(float pDeltaTime)
{
	// Call Update on all components
	auto it = mComponents.begin();

	while (it != mComponents.end())
	{
		for (Game::Component::AComponent* Component : it->second)
		{
			Component->OnUpdate(pDeltaTime);
		}

		++it;
	}
}

void Actor::updateProj(Maths::FMatrix4& pProj)
{
	for (Game::Component::AComponent* model : mComponents[Utils::ComponentType::MeshRenderer])
		((Game::Component::CPModel*)model)->updateMat(pProj, getTransform()->getWorldMatrix());
}

void Actor::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& pWriter)
{
	pWriter.Key("Tags");
	pWriter.StartArray();
	for (auto it = mTags.begin(); it != mTags.end(); it++)
		if (!it->empty())
			pWriter.String(it->c_str());
	pWriter.EndArray();

	pWriter.Key("Components");
	pWriter.StartArray();

	for (auto it = mComponents.begin(); it != mComponents.end(); it++)
		for (Game::Component::AComponent* Component : it->second)
			Component->serialize(pWriter);

	pWriter.EndArray();
}

void Game::Data::Actor::setTag(std::string pTag)
{
	if (std::find(mTags.begin(), mTags.end(), pTag) == mTags.end())
	{
		mTags.push_back(pTag);
	}
}

void Game::Data::Actor::changeTag(std::string pOldTag, std::string pNewTag)
{
	std::vector<std::string>::iterator it = std::find(mTags.begin(), mTags.end(), pOldTag);
	if (it == mTags.end())
	{
		mTags.push_back(pNewTag);
	}
	else if (std::find(mTags.begin(), mTags.end(), pNewTag) == mTags.end())
	{
		int Difference = it - mTags.begin();
		mTags[Difference] = pNewTag;
	}
}

void Game::Data::Actor::deleteTag(std::string pTag)
{
	std::vector<std::string>::iterator it = std::find(mTags.begin(), mTags.end(), pTag);
	if (it != mTags.end())
	{
		mTags.erase(it);
	}
}

std::string Game::Data::Actor::getTag(int pIndex)
{
	return mTags[pIndex];
}

int Game::Data::Actor::getNumtags()
{
	return mTags.size();
}

void Game::Data::Actor::clearTags()
{
	mTags.clear();
}