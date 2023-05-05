#include "Game/SceneSys/Scene.h"
#include "EngineCore/Service/ServiceLocator.h"
#include "EngineCore/Core/EngineApp.h"
using namespace Game::SceneSys;

Game::SceneSys::Scene::~Scene()
{
	for (auto actor : mActors)
		delete actor;
}

void Scene::addActor(Data::Actor* mActor)
{
	service(EngineCore::Core::EngineApp).mainThreadAction.pushFunction([this, mActor]
		{
		mActors.push_back(mActor); 
		});
}

void Scene::removeActor(Data::Actor* mActor)
{
	service(EngineCore::Core::EngineApp).mainThreadAction.pushFunction([this, mActor]
	{
		mActors.remove(mActor); 
		delete mActor;
	});
}

std::vector<Game::Data::Actor*> Game::SceneSys::Scene::getActorsByTag(std::string pTag)
{
	std::list<Game::Data::Actor*>::iterator it;
	std::vector<Game::Data::Actor*> CurrentActors;
	for (it = mActors.begin(); it != mActors.end(); ++it) 
	{
		std::vector<std::string> Tags = (*it)->mTags;
		for (int i = 0; i < Tags.size(); ++i)
		{
			if (Tags[i].compare(pTag) == 0)
			{
				Game::Data::Actor* actor = *it;
				CurrentActors.push_back(actor);
			}
		}
	}
	return sol::as_table(CurrentActors);
}
