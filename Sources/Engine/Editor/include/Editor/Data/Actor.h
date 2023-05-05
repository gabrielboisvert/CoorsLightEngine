#pragma once
#include "Game/Component/AComponent.h"
#include "Game/Utils/ComponentType.h"
#include "Game/Component/CPTransform.h"

#include <list>
#include <unordered_map>
#include "Rendering/Buffers/VK/VKUniformBuffer.h"
#include "Rendering/Data/UniformData.h"
#include "Game/Utils/ComponentType.h"
#include "EngineCore/EventSystem/Event.h"
#include "Maths/FMatrix4.h"
#include "Game/Data/Actor.h"
#include "Rendering/Data/Material.h"


namespace Editor::Data
{
	class Actor
	{
		public:
			std::unordered_map<Game::Utils::ComponentType, std::list<Game::Component::AComponent*>>& mComponents;
			EngineCore::EventSystem::Event<>& mValueChanged;
			EngineCore::EventSystem::Event<Maths::FMatrix4>& mValueChangedFromEditor;
			Rendering::Buffers::VK::VKUniformBuffer<Rendering::Data::UniformData> mUniformBuferOutLine;
			Rendering::Buffers::VK::VKUniformBuffer<Rendering::Data::UniformDataWithId> mUniformBufferid;
			std::mutex& mMutex;
			unsigned int& id;
			void* mItemOwner = nullptr;

			Actor(Game::Data::Actor& pActor, void* pOwner);
			Game::Component::CPTransform* getTransform(unsigned pIndex = 0);
			void draw(void* pCmd, Rendering::Data::Material* pPipeLine = nullptr);

			void updateParticle(float pDeltaTime, Maths::FVector3& pCamPos);
			void drawParticle(void* pCmd, Rendering::Data::Material& pPipeLine, Maths::FMatrix4& pViewProj, Maths::FMatrix4& pView);
		
		#ifdef NSHIPPING
			void drawCamera(void* pCmd);
			void drawCameraOffscreen(void* pCmd);
		#endif

			void Actor::updateProj(Maths::FMatrix4& pProj);
			~Actor() = default;
	};
}