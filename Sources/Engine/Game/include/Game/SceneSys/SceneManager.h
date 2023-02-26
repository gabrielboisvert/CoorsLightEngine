#pragma once
#include <list>
#include "Game/SceneSys/Scene.h"
#include "unordered_map"

namespace Game::SceneSys
{
	class SceneManager
	{
		public:
			Scene* mCurrentScene = nullptr;
			std::unordered_map<const char*, Scene*> mScenes;

			void loadScene(const char* pSceneName);
			void addScene(const char* pPath);
	};
}