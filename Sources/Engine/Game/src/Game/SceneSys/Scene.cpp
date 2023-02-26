#include "Game/SceneSys/Scene.h"

using namespace Game::SceneSys;

void Scene::addActor(Data::Actor& mActor)
{
	std::unique_lock lock(mMutex);
	mActors.push_back(&mActor);
}

void Scene::lockActors()
{
	std::unique_lock lock(mMutex);
}