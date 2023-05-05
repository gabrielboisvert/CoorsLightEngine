#include "EngineCore/Core/EngineApp.h"
#include "EngineCore/Service/ServiceLocator.h"
#include "EngineCore/Thread/ThreadPool.h"
#include "Rendering/Renderer/VK/VKRenderer.h"
#include "Rendering/Data/Material.h"
#include "Rendering/Resources/VK/PipeLineBuilder.h"
#include "Rendering/Buffers/VK/VKUniformBuffer.h"
#include "Rendering/Data/UniformData.h"
#include "Rendering/Resources/Texture.h"
#include "EngineCore/Service/ServiceLocator.h"


#include "Game/Component/CPScript.h"
#include "Physics/Core/BulPhysicsEngine.h"

#include "EngineCore/EventSystem/InputManager.h"
#include "EngineCore/EventSystem/Key.h"
#include "Maths/FVector2.h"
#include "Scripting/ScriptInterpreter.h"
#include <Rendering/Resources/Font.h>


using namespace EngineCore::Core;

EngineApp::EngineApp() : mWindow("", 0, 0)
{
	Game::Component::CPCamera::mWorldCamera = nullptr;
}

EngineApp::EngineApp(unsigned int pWidth, unsigned int pHeight) : mWindow("Game", pWidth, pHeight)
{
	Game::Component::CPCamera::mWorldCamera = nullptr;
	provideService(EngineCore::Core::EngineApp, *this);

#ifdef SHIPPING
	initRenderer();
	provideService(Rendering::Renderer::VK::VKRenderer, *rend);
#endif
}

void EngineApp::run()
{	
#ifdef NSHIPPING
	initRenderer();
#endif
	initSceneMat();

	mLightManager = new Light::LightManager();
	initScene();
#ifdef SHIPPING
	initInput();
	initScripting();
#endif

	while (!mWindow.shouldClose())
	{
		eventUpdate();

		if (mScene == nullptr)
		{
			initScene();
			drawScene();
			continue;
		}

		checkResizeWindow();
		updateComponentOnce();
		updateComponent();
		updatePhysique();
		drawScene();
	}

	cleanupScene();
}

void EngineApp::initRenderer()
{
	rend = new Rendering::Renderer::VK::VKRenderer();
	rend->init(mWindow);
}

void EngineApp::initSceneMat()
{
	mat = new Rendering::Data::Material(Rendering::Renderer::Resources::VK::PipeLineBuilder()
		.initPipeLine("Resources/Engine/Shaders/ModelVertex.vert.spv", "Resources/Engine/Shaders/ModelFrag.frag.spv", rend->mRenderPass, false));

	mUIMat = new Rendering::Data::Material(Rendering::Renderer::Resources::VK::PipeLineBuilder()
		.initPipeLine("Resources/Engine/Shaders/UIVertex.vert.spv", "Resources/Engine/Shaders/UIFrag.frag.spv", rend->mRenderPass, false, false, false, false));

	mTextMat = new Rendering::Data::Material(Rendering::Renderer::Resources::VK::PipeLineBuilder()
		.initPipeLine("Resources/Engine/Shaders/TextVertex.vert.spv", "Resources/Engine/Shaders/TextFrag.frag.spv", rend->mRenderPass, false, false, false, false));

	sky = new Rendering::Data::Material(Rendering::Renderer::Resources::VK::PipeLineBuilder()
		.initPipeLine("Resources/Engine/Shaders/SkyVertex.vert.spv", "Resources/Engine/Shaders/SkyFrag.frag.spv", rend->mRenderPass, false));

	particleMat = new Rendering::Data::Material(Rendering::Renderer::Resources::VK::PipeLineBuilder()
		.initPipeLine("Resources/Engine/Shaders/ParticleVertex.vert.spv", "Resources/Engine/Shaders/ParticleFrag.frag.spv", rend->mRenderPass, false, false, true, false, true));
}

void EngineApp::initInput()
{
	mInput = &service(EngineCore::EventSystem::InputManager);
	mWindow.setInputManager(mInput);
}

void EngineApp::initScene()
{
	mScene = service(Game::SceneSys::SceneManager).mCurrentScene;
	mActors = &mScene->mActors;

	if (mLightManager != nullptr)
	{
		if (mScene != nullptr)
			mLightManager->mLights = &mScene->mLights;
	}
}

void EngineApp::initScripting()
{
	service(Scripting::ScriptInterpreter).RefreshAll();
}

void EngineApp::eventUpdate()
{
	mainThreadAction.flush();
	clock.update();
	mWindow.pollEvents();

#ifdef SHIPPING
	mInput->processInput();
#endif
}

void EngineApp::checkResizeWindow()
{
	if (rend->mFramebufferResized)
	{
		int width, height;
		mWindow.getFramebufferSize(&width, &height);

		if (Game::Component::CPCamera::mWorldCamera != nullptr)
			Game::Component::CPCamera::mWorldCamera->updateProjection(width, height);

		rend->mFramebufferResized = false;

		uiProj = Maths::FMatrix4::createOrthographic(0, width, 0, height, -100, 100);
		mWidth = width;
		mHeight = height;
		for (Game::Data::Actor* actor : *mActors)
			actor->updateUIProj(uiProj, width, height);

		int x, y;
		glfwGetWindowPos(mWindow.mWindow, &x, &y);

		mScreenCenter = Maths::FVector2(mWidth / 2, mHeight / 2);
		mScreenCenter.x += x;
		mScreenCenter.y += y;
	}
}

void EngineApp::updateComponentOnce()
{
	if (mStart && !mHasPlayed)
	{
		for (Game::Data::Actor* actor : *mActors)
		{
			actor->OnAwake();
			actor->updateWorldTransform(actor->getTransform());
			actor->OnStart();
		}
		mStart = false;
		mHasPlayed = true;
		mPaused = false;
	}
	else if (mPaused)
	{
		for (Game::Data::Actor* actor : *mActors)
			actor->OnPaused();

		mPaused = false;
	}
	else if (mStop)
	{
		for (Game::Data::Actor* actor : *mActors)
			actor->OnDestroy();

		mStop = false;
	}
}

void EngineApp::updateComponent()
{
	for (Game::Data::Actor* actor : *mActors)
	{
		if (mPlaying)
		{
			actor->updateWorldTransform(actor->getTransform());
			actor->OnUpdate(clock.getDeltaTime());

			if (mWindow.press)
			{
#ifdef  NSHIPPING
				mMouseLock = true;	
#endif 


				Rendering::Resources::UIResource::IUIResource* tmp = actor->containMouseClick(mWindow.mousePos);
				if (tmp)
					mUIPressed = tmp;

				mMouseClick = true;
			}
			else
				actor->containMouse(mWindow.mousePos);

			if (!mWindow.press && mMouseClick)
			{
				mMouseClick = false;
				if (mUIPressed)
				{
					mUIPressed->CallLuaDelegatePress();
					mUIPressed = nullptr;
				}
			}
		}
	}
}

void EngineApp::updatePhysique()
{
	if (mPlaying)
		service(Physics::Core::BulPhysicsEngine).update(clock.getDeltaTime());
}

void EngineApp::drawScene()
{
	rend->beginFrame();
	if (Game::Component::CPCamera::mWorldCamera != nullptr && mScene != nullptr)
	{	
		drawSky();
		drawActors();
		drawParticle();
		drawUI();
	}
	rend->endFrame();
}

void EngineApp::bindLights()
{
	if (mLightManager == nullptr)
		return;

	if (mLightManager->mLights == nullptr)
	{
		initScene();
		return;
	}

	mLightManager->mUniformBuffer.mData.mNumLight = mLightManager->mLights->size();
	
	{
		int i = 0;
		for (auto& it : *mLightManager->mLights)
			it->fillUniform(i, mLightManager->mUniformBuffer.mData);
		i++;
	}

	mLightManager->mUniformBuffer.mData.mEyePos = Game::Component::CPCamera::mWorldCamera->mPosition;
	mLightManager->mUniformBuffer.updateData();
		
	mat->bindDescriptor("uboFrag", mLightManager->mUniformBuffer.mDescriptorSets);
}

void EngineApp::drawActors()
{
	bindLights();
	for (Game::Data::Actor* actor : *mActors)
	{
		actor->updateProj(Game::Component::CPCamera::mWorldCamera->viewProj());
		actor->draw(rend->getCurrentCommandBuffer(), *mat);
	}
}

void EngineApp::drawSky()
{
	Game::Component::CPCamera::mWorldCamera->drawSkyBox(rend->getCurrentCommandBuffer(), *sky);
}

void EngineApp::drawUI()
{
	for (Game::Data::Actor* actor : *mActors)
		actor->drawUI(rend->getCurrentCommandBuffer(), *mUIMat, *mTextMat);
}

void EngineApp::drawParticle()
{
	if (Game::Component::CPCamera::mWorldCamera == nullptr)
		return;

	for (Game::Data::Actor* actor : *mActors)
		actor->drawParticle(rend->getCurrentCommandBuffer(), *particleMat,
			Game::Component::CPCamera::mWorldCamera->mVp, 
			Game::Component::CPCamera::mWorldCamera->mView);
}

void EngineApp::cleanupScene()
{
	rend->waitForCleanUp();

	std::unique_lock<std::mutex> lck(closeMut);
	closed = true;
	cv.notify_all();

	delete mat;
	delete mUIMat;
	delete mTextMat;
	delete sky;
	delete particleMat;

	if (mLightManager != nullptr)
		delete mLightManager;

#ifdef NSHIPPING
	delete rend;
#endif
}

void EngineApp::playFrame()
{
	float delta = clock.getDeltaTime();
	service(Physics::Core::BulPhysicsEngine).update(delta);

	std::list<Game::Data::Actor*>& mActors = service(Game::SceneSys::SceneManager).mCurrentScene->mActors;
	for (Game::Data::Actor* actor : mActors)
	{
		actor->updateWorldTransform(actor->getTransform());
		actor->OnUpdate(delta);
	}
}

void EngineApp::close()
{
	mWindow.close();
}
