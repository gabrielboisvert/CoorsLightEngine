#pragma once
#include "Game/Data/Actor.h"
#include <list>

namespace Game::SceneSys
{
	class Scene
	{
		public:
			std::list<Data::Actor*> mActors;
			std::mutex mMutex;

			Scene() {};

			void addActor(Data::Actor& mActor);
			
			void lockActors();
	};
}