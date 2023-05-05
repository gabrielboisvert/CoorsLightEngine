#include "Game/Component/ACollider.h"
#include "Scripting/ScriptInterpreter.h"
void Game::Component::ACollider::OnCollisionEnter(void* pOtherActor, ACollider* pOtherCollider)
{
	if (service(Scripting::ScriptInterpreter).mLuaBinder.mComponentBinder.mPhysicsFunctions.OnCollisionEnter)
	{
		service(Scripting::ScriptInterpreter).mLuaBinder.mComponentBinder.mPhysicsFunctions.OnCollisionEnter(pOtherActor);
	}
}

void Game::Component::ACollider::OnTriggerEnter(void* pOtherActor, ACollider* pOtherCollider)
{
	if(service(Scripting::ScriptInterpreter).mLuaBinder.mComponentBinder.mPhysicsFunctions.OnTriggerEnter)
		service(Scripting::ScriptInterpreter).mLuaBinder.mComponentBinder.mPhysicsFunctions.OnTriggerEnter(pOtherActor);
}

void Game::Component::ACollider::OnCollisionStay(void* pOtherActor, ACollider* pOtherCollider)
{
	if(service(Scripting::ScriptInterpreter).mLuaBinder.mComponentBinder.mPhysicsFunctions.OnCollisionStay)
		service(Scripting::ScriptInterpreter).mLuaBinder.mComponentBinder.mPhysicsFunctions.OnCollisionStay(pOtherActor);
}

void Game::Component::ACollider::OnTriggerStay(void* pOtherActor, ACollider* pOtherCollider)
{
	if(service(Scripting::ScriptInterpreter).mLuaBinder.mComponentBinder.mPhysicsFunctions.OnTriggerStay)
		service(Scripting::ScriptInterpreter).mLuaBinder.mComponentBinder.mPhysicsFunctions.OnTriggerStay(pOtherActor);

}

void Game::Component::ACollider::OnCollisionExit(void* pOtherActor, ACollider* pOtherCollider)
{
	if(service(Scripting::ScriptInterpreter).mLuaBinder.mComponentBinder.mPhysicsFunctions.OnCollisionExit)
		service(Scripting::ScriptInterpreter).mLuaBinder.mComponentBinder.mPhysicsFunctions.OnCollisionExit(pOtherActor);

}

void Game::Component::ACollider::OnTriggerExit(void* pOtherActor, ACollider* pOtherCollider)
{
	if(service(Scripting::ScriptInterpreter).mLuaBinder.mComponentBinder.mPhysicsFunctions.OnTriggerExit)
		service(Scripting::ScriptInterpreter).mLuaBinder.mComponentBinder.mPhysicsFunctions.OnTriggerExit(pOtherActor);

}
