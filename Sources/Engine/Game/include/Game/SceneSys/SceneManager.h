#pragma once
#include <list>
#include "Game/SceneSys/Scene.h"
#include <unordered_map>
#include <rapidjson/document.h>

namespace Game::SceneSys
{
	class SceneManager
	{
		public:
			~SceneManager();

			Scene* mCurrentScene = nullptr;
			std::string mProjectPath;
			std::unordered_map<std::string, Scene*> mScenes;

			void loadScene(const char* pSceneName);
			void loadScene(const char* pSceneName, std::string pProjectPath);
			void loadSceneLua(const char* pSceneName);
			void addScene(const char* pPath);

			void parseMap(const char* pPath);
			void deserializeObjChild(void* pParent, const rapidjson::Value& pObject);
	};
}