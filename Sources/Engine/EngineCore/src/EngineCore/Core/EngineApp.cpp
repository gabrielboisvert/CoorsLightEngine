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
#include "Game/SceneSys/SceneManager.h"
#include "Game/Data/Actor.h"
#include "Physics/Core/BulPhysicsEngine.h"
#include "Tools/Time/Clock.h"
using namespace EngineCore::Core;

EngineApp::EngineApp(unsigned int pWidth, unsigned int pHeight) : mWindow("", pWidth, pHeight)
{
	Rendering::Renderer::VK::VKRenderer* rend = new Rendering::Renderer::VK::VKRenderer();
	rend->init(mWindow);
}

void EngineApp::run()
{
	Rendering::Renderer::VK::VKRenderer& renderer = service(Rendering::Renderer::VK::VKRenderer);

	Rendering::Data::Material mat(Rendering::Renderer::Resources::VK::PipeLineBuilder()
		.initPipeLine("Resources/Editor/Shaders/ModelVertex.vert.spv", "Resources/Editor/Shaders/ModelFrag.frag.spv", renderer.mRenderPass, false));

	

	Rendering::Resources::Texture text("Resources/Editor/Textures/default.png");
	mat.bindDescriptor("texSampler", text.mTextureSets);

	std::list<Game::Data::Actor*>& mActors = service(Game::SceneSys::SceneManager).mCurrentScene->mActors;
	Tools::Time::Clock clock;
	while (!mWindow.shouldClose())
	{
		clock.update();
		mWindow.pollEvents();

		if(mPlaying)
			service(Physics::Core::BulPhysicsEngine).update(clock.getDeltaTime());
		
		renderer.beginFrame();
		
		{
			service(Game::SceneSys::SceneManager).mCurrentScene->lockActors();
			for (Game::Data::Actor* actor : mActors)
			{
				if (mPlaying)
					actor->updateWorldTransform(actor->getTransform());

				mat.bindDescriptor("ubo", actor->uniformBuffer.mDescriptorSets);
				mat.bindPipeLine(renderer.getCurrentCommandBuffer());
				actor->draw(renderer.getCurrentCommandBuffer());
			}

			//service(Physics::Core::BulPhysicsEngine).mWorld.getDebugDrawer()->drawLine({ 0, -1, 0 }, { 0, 1, 0 }, { 0, 1, 0 });

			service(Physics::Core::BulPhysicsEngine).mWorld.debugDrawWorld();
		}

		renderer.endFrame();
	}

	renderer.waitForCleanUp();
}

void EngineApp::close()
{
	mWindow.close();
}