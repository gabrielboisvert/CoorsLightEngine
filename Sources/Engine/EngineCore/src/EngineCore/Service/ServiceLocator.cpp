#include "EngineCore/Service/ServiceLocator.h"
#include "EngineCore/Thread/ThreadPool.h"
#include "EngineCore/ResourceManagement/ResourceManager.h"
#include "Rendering/Renderer/VK/VKRenderer.h"
#include "Physics/Core/BulPhysicsEngine.h"

EngineCore::Service::ServiceLocator EngineCore::Service::ServiceLocator::instance;

EngineCore::Service::ServiceLocator::~ServiceLocator()
{
	remove<EngineCore::Thread::ThreadPool>();
	remove<EngineCore::ResourcesManagement::ResourceManager>();
	remove<Physics::Core::BulPhysicsEngine>();
	remove<Rendering::Renderer::VK::VKRenderer>();
}