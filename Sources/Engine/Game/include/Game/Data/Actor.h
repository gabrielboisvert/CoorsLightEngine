#pragma once

#include "Game/Component/AComponent.h"
#include "Game/Component/CPTransform.h"
#include <list>
#include <unordered_map>
#include "Rendering/Buffers/VK/VKUniformBuffer.h"
#include "Rendering/Data/UniformData.h"
#include "Game/Utils/ComponentType.h"

namespace Game::Data
{
	class Actor
	{
	public:
		std::unordered_map<Utils::ComponentType, std::vector<Component::AComponent*>> mComponents;

		Actor();
		Actor(const Actor& pOther);
		~Actor();

		Component::CPTransform* getTransform(unsigned pIndex = 0);
		
		void addComponent(Utils::ComponentType pType, Component::AComponent* pComponent);

		void draw(void* pCmd);

		void updateWorldTransform(Component::CPTransform* pTransform);

		void updateWorldTransformEditor(Component::CPTransform* pTransform);

		Rendering::Buffers::VK::VKUniformBuffer<Rendering::Data::UniformData> uniformBuffer;
	};
}