#include "Editor/Data/Actor.h"
#include "Game/Component/CPModel.h"
#include "Game/Component/CPCamera.h"
#include "Game/Component/CPParticle.h"

using namespace Editor::Data;

Actor::Actor(Game::Data::Actor& pActor, void* pOwner)
	: mComponents(pActor.mComponents), mValueChanged(pActor.mValueChanged),
	mValueChangedFromEditor(pActor.mValueChangedFromEditor), mMutex(pActor.mMutex), 
	id(pActor.id), mUniformBufferid(VK_SHADER_STAGE_VERTEX_BIT), mItemOwner(pOwner), mUniformBuferOutLine(VK_SHADER_STAGE_VERTEX_BIT)
{

}

Game::Component::CPTransform* Actor::getTransform(unsigned pIndex)
{
	if (mComponents[Game::Utils::ComponentType::Transform].size() <= pIndex)
		return nullptr;

	return (Game::Component::CPTransform*)mComponents[Game::Utils::ComponentType::Transform].front() + pIndex;
}

void Actor::draw(void* pCmd, Rendering::Data::Material* pPipeLine)
{
	if (mComponents.find(Game::Utils::ComponentType::MeshRenderer) == mComponents.end())
		return;

	for (Game::Component::AComponent* model : mComponents[Game::Utils::ComponentType::MeshRenderer])
		((Game::Component::CPModel*)model)->drawEditor(pCmd, pPipeLine);
}

void Actor::updateParticle(float pDeltaTime, Maths::FVector3& pCamPos)
{
	if (mComponents.find(Game::Utils::ComponentType::Particle) == mComponents.end())
		return;

	for (Game::Component::AComponent* particle : mComponents[Game::Utils::ComponentType::Particle])
		((Game::Component::CPParticle*)particle)->OnUpdateEditor(pDeltaTime, pCamPos);
}

void Actor::drawParticle(void* pCmd, Rendering::Data::Material& pPipeLine, Maths::FMatrix4& pViewProj, Maths::FMatrix4& pView)
{
	if (mComponents.find(Game::Utils::ComponentType::Particle) == mComponents.end())
		return;

	for (Game::Component::AComponent* particle : mComponents[Game::Utils::ComponentType::Particle])
		((Game::Component::CPParticle*)particle)->drawEditor(pCmd, pPipeLine, pViewProj, pView);
}

#ifdef NSHIPPING
void Actor::drawCamera(void* pCmd)
{
	if (mComponents.find(Game::Utils::ComponentType::Camera) == mComponents.end())
		return;

	for (Game::Component::AComponent* camera : mComponents[Game::Utils::ComponentType::Camera])
		((Game::Component::CPCamera*)camera)->draw(pCmd);
}

void Actor::drawCameraOffscreen(void* pCmd)
{
	if (mComponents.find(Game::Utils::ComponentType::Camera) == mComponents.end())
		return;

	for (Game::Component::AComponent* camera : mComponents[Game::Utils::ComponentType::Camera])
		((Game::Component::CPCamera*)camera)->drawOffScreen(pCmd);
}
#endif

void Actor::updateProj(Maths::FMatrix4& pProj)
{
	for (Game::Component::AComponent* model : mComponents[Game::Utils::ComponentType::MeshRenderer])
		((Game::Component::CPModel*)model)->updateMatEditor(pProj, getTransform()->getWorldMatrix());

#ifdef NSHIPPING
	std::unique_lock lock(mMutex);
	if (mComponents.find(Game::Utils::ComponentType::Camera) != mComponents.end() && mComponents.find(Game::Utils::ComponentType::Camera)->second.size() != 0)
		((Game::Component::CPCamera*)mComponents[Game::Utils::ComponentType::Camera].front())->updateProjection(pProj);
#endif
}