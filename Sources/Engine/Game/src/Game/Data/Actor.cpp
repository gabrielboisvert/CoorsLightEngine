#include "Game/Data/Actor.h"
#include "Game/Component/CPModel.h"
#include "Rendering/Buffers/VK/VKUniformBuffer.h"
#include "Game/Component/CPBoxCollider.h"

using namespace Game::Data;

Actor::Actor() : uniformBuffer(VK_SHADER_STAGE_VERTEX_BIT)
{
	mComponents[Utils::ComponentType::Transform].push_back(new Component::CPTransform());
	
	uniformBuffer.mData.mModel = Maths::FMatrix4::createTransformMatrix({}, { }, { Maths::FVector3::One });
	uniformBuffer.mData.mViewProjection = Maths::FMatrix4::createPerspective(-45, 1, 0.01f, 500.0f)
		* Maths::FMatrix4::lookAt({ 0, 0, -10 }, { 0, 0, 0 }, Maths::FVector3::Up);
	uniformBuffer.updateData();
}

Actor::Actor(const Actor& pOther) : uniformBuffer(VK_SHADER_STAGE_VERTEX_BIT)
{
	auto it = pOther.mComponents.begin();
	while (it != pOther.mComponents.end())
	{
		for (unsigned int i = 0; i < it->second.size(); i++)
			mComponents[it->first].push_back(it->second[i]->clone());

		++it;
	}
}

Actor::~Actor()
{
	auto it = mComponents.begin();
	while (it != mComponents.end())
	{
		for (unsigned int i = 0; i < it->second.size(); i++)
			delete it->second[i];

		++it;
	}
}

Game::Component::CPTransform* Actor::getTransform(unsigned pIndex)
{
	if (mComponents[Utils::ComponentType::Transform].size() <= pIndex)
		return nullptr;

	return (Game::Component::CPTransform*)mComponents[Utils::ComponentType::Transform][pIndex];
}

void Actor::addComponent(Utils::ComponentType pType, Component::AComponent* pComponent)
{
	if (pComponent == nullptr)
		return;

	mComponents[pType].push_back(pComponent);
}

void Actor::draw(void* pCmd)
{
	if (mComponents.find(Utils::ComponentType::MeshRenderer) == mComponents.end())
		return;

	std::vector<Game::Component::AComponent*>& models = mComponents[Utils::ComponentType::MeshRenderer];
	for (unsigned int i = 0; i < models.size(); i++)
		((Game::Component::CPModel*)models[i])->draw(pCmd);
}

void Actor::updateWorldTransform(Component::CPTransform* pTransform)
{
	if (mComponents.find(Utils::ComponentType::BoxCollider) == mComponents.end())
		return;

	Game::Component::CPBoxCollider* body = ((Game::Component::CPBoxCollider*)mComponents[Utils::ComponentType::BoxCollider][0]);

	
	pTransform->mLocalPosition = body->getPositionFromPhysics();
	pTransform->updateLocalMatrix();

	uniformBuffer.mData.mModel = pTransform->getWorldMatrix();
	uniformBuffer.updateData();
}

void Game::Data::Actor::updateWorldTransformEditor(Component::CPTransform* pTransform)
{
	Game::Component::CPBoxCollider* body = nullptr;
	if (mComponents.find(Utils::ComponentType::BoxCollider) != mComponents.end())
		body = ((Game::Component::CPBoxCollider*)mComponents[Utils::ComponentType::BoxCollider][0]);

	if (body != nullptr)
		body->setTransform(pTransform->getLocalPosition(), pTransform->getLocalRotation(), pTransform->getLocalScale());
	
	pTransform->updateLocalMatrix();

	uniformBuffer.mData.mModel = pTransform->getWorldMatrix();
	uniformBuffer.updateData();
}