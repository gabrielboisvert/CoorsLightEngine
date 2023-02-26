#include "EngineCore/Thread/ThreadPool.h"
#include "EngineCore/Service/ServiceLocator.h"
#include "Game/SceneSys/SceneManager.h"
#include "Editor/Widget/WidgetConsole.h"

using namespace Game::SceneSys;

void SceneManager::loadScene(const char* pSceneName)
{
	if (mScenes.find(pSceneName) == mScenes.end())
		addScene(pSceneName);

	mScenes[pSceneName] = new Scene;
	mCurrentScene = mScenes[pSceneName];
}

void SceneManager::addScene(const char* pPath)
{
	static std::mutex sceneMutex;
	std::unique_lock lock(sceneMutex);
	
	mScenes[pPath] = nullptr;
}