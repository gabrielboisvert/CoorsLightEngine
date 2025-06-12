#include "Scripting/Binders/LuaComponentBinder.h"
#include "EngineCore/Service/ServiceLocator.h"

#include "Game/Component/CPTransform.h"
#include "Game/Component/CPModel.h"

#include "Game/Component/ACollider.h"
#include <Game/Component/CPCamera.h>

#include "Game/Component/CPSound.h"
#include "Game/Component/CPUI.h"

#include "Game/Component/CPParticle.h"

#include "Game/Component/CPAnimator.h"

#ifdef NSHIPPING
#include "Editor/Widget/WidgetConsole.h"
#endif
#include "Rendering/Resources/UI/Text.h"
#include "Rendering/Resources/UI/Button.h"

using namespace Scripting::Binder;
using namespace Game::Component;
using namespace Maths;

void LuaComponentBinder::bind(sol::state& pLuaState)
{
	mLuaState = &pLuaState;

	// Bind the transform component
	pLuaState.new_usertype<CPTransform>("Transform",
		sol::base_classes, sol::bases<AComponent>(),
		"SetPosition", &CPTransform::setLocalPosition,
		"SetRotation", &CPTransform::setLocalRotation,
		"SetScale", &CPTransform::setLocalScale,
		"SetLocalPosition", &CPTransform::setLocalPosition,
		"SetLocalRotation", &CPTransform::setLocalRotation,
		"SetLocalScale", &CPTransform::setLocalScale,
		"SetWorldPosition", &CPTransform::setWorldPosition,
		"SetWorldRotation", &CPTransform::setWorldRotation,
		"SetWorldScale", &CPTransform::setWorldScale,
		"AddPosition", &CPTransform::addLocalPosition,
		"AddRotation", &CPTransform::addLocalRotation,
		"AddScale", &CPTransform::addLocalScale,
		"AddLocalPosition", &CPTransform::addLocalPosition,
		"AddLocalRotation", &CPTransform::addLocalRotation,
		"AddLocalScale", &CPTransform::addLocalScale,
		"GetPosition", [](CPTransform& pThis)->Maths::FVector3 {return pThis.getWorldPosition(); },
		"GetRotation", [](CPTransform& pThis)->Maths::FQuaternion {return pThis.getWorldRotation(); },
		"GetScale", [](CPTransform& pThis)->Maths::FVector3 {return pThis.getWorldScale(); },
		"GetLocalPosition", [](CPTransform& pThis)->Maths::FVector3 {return pThis.getLocalPosition(); },
		"GetLocalRotation", [](CPTransform& pThis)->Maths::FQuaternion {return pThis.getLocalRotation(); },
		"GetLocalScale", [](CPTransform& pThis)->Maths::FVector3 {return pThis.getLocalScale(); },
		"GetWorldPosition", [](CPTransform& pThis)->Maths::FVector3 {return pThis.getWorldPosition(); },
		"GetWorldRotation", [](CPTransform& pThis)->Maths::FQuaternion {return pThis.getWorldRotation(); },
		"GetWorldScale", [](CPTransform& pThis)->Maths::FVector3 {return pThis.getWorldScale(); },
		"GetLocalForward", [](CPTransform& pThis)->Maths::FVector3 {return pThis.getLocalForward(); }
	);

	// Bind the model component
	pLuaState.new_usertype<CPModel>("Model",
		sol::base_classes, sol::bases<AComponent>(),
		"SetModel", &CPModel::setModelWithPathLua
		);

	//Physics
	pLuaState.new_usertype<ACollider>("Collider",
		sol::base_classes, sol::bases<AComponent>(),
		"Delegate", [&pLuaState, this](ACollider& pCollider, std::string DelegateString)->void
		{
			DelegatePhysics(pLuaState, DelegateString);
		},
		"AddForce", &ACollider::addForce,
			"ClearForces", &ACollider::clearForces,
			"SetLinearVelocity", &ACollider::setLinearVelocity,
			"SetAngularVelocity", &ACollider::setAngularVelocity,
			"ClearLinearVelocity", &ACollider::clearLinearVelocity
			);

	//	DelegatePhysics(pLuaState);

		//Camera
	pLuaState.new_usertype<CPCamera>("Camera",
		sol::base_classes, sol::bases<AComponent>(),
		"GetForward", &CPCamera::getForward,
		"GetUp", &CPCamera::getUp,
		"GetRight", &CPCamera::getRight,
		"AddYaw", &CPCamera::AddYaw,
		"AddPitch", &CPCamera::AddPitch
		);

	//Sound
	pLuaState.new_usertype<CPSound>("Sound",
		sol::base_classes, sol::bases<AComponent>(),
		"Play", &CPSound::play,
		"Pause", &CPSound::pause,
		"SetVolume", &CPSound::setVolume,
		"SetMute", &CPSound::setMute,
		"SetLoop", &CPSound::setLoop,
		"SetMinDist", &CPSound::setMinDist,
		"Set3DSound", &CPSound::set3DSound,
		"SetClip", &CPSound::setClipWithLua
		);

	//UI
	pLuaState.new_usertype<CPUI>("UI",
		sol::base_classes, sol::bases<AComponent>(),
		"DelegateUI", [&pLuaState, this](CPUI& pThis, unsigned int pPressHold, std::string pFunctionName, std::string pName)->void
		{
			if (DelegateFromString(pLuaState, pFunctionName))
			{
				pThis.DelegateLuaFunctionCP(pPressHold, (std::function<void()>)pLuaState[pFunctionName], pName);
			}
		},
		"SetText", [](CPUI& pThis, std::string pUIName, std::string pNewText)->void
		{
			Rendering::Resources::UIResource::IUIResource* Resource = pThis.ui->findResource(pUIName);
			if (Resource)
			{
				Rendering::Resources::UIResource::Text* text = dynamic_cast<Rendering::Resources::UIResource::Text*>(Resource);
				if (text)
				{
					text->setText(pNewText);
					return;
				}
				Rendering::Resources::UIResource::Button* butt = dynamic_cast<Rendering::Resources::UIResource::Button*>(Resource);
				if (butt)
				{
					butt->setText(pNewText);
					return;
				}
			}
		},

			"SetColor", [](CPUI& pThis, std::string pUIName, Maths::FVector4 pNewColor)->void
		{
			Rendering::Resources::UIResource::IUIResource* Resource = pThis.ui->findResource(pUIName);
			if (Resource)
			{
				Resource->setColor(pNewColor);
			}
		},

			"SetVisibility", [](CPUI& pThis, std::string pUIName, bool pVisibility)->void
		{
			Rendering::Resources::UIResource::IUIResource* Resource = pThis.ui->findResource(pUIName);
			if (Resource)
			{
				Resource->setVisible(pVisibility);
			}
		},

			"GetVisibility", [](CPUI& pThis, std::string pUIName)->bool
		{
			Rendering::Resources::UIResource::IUIResource* Resource = pThis.ui->findResource(pUIName);
			if (Resource)
			{
				return !Resource->isHidden;
			}

			return false;
		},
			"SetScale", [](CPUI& pThis, std::string pUIName, Maths::FVector2 pNewScale)->void
			{
				Rendering::Resources::UIResource::IUIResource* Resource = pThis.ui->findResource(pUIName);
				if (Resource)
				{
					Resource->setScale(pNewScale);
				}
			},
			"GetScale", [](CPUI& pThis, std::string pUIName)->void
			{
				Rendering::Resources::UIResource::IUIResource* Resource = pThis.ui->findResource(pUIName);
				if (Resource)
				{
					Resource->getScale();
				}
			}
		);

	//Particles
	pLuaState.new_usertype<CPParticle>("Particle",
		sol::base_classes, sol::bases<AComponent>(),
		"SetParticle", &CPParticle::setParticleWithLua,
		"Play", &CPParticle::play,
		"Pause", &CPParticle::pause,
		"SetLoop", [this](CPParticle& pThis, bool pTrue) {pThis.loop = pTrue; },
		"Stop", &CPParticle::stop
		);

	//Animator

	//pLuaState.new_usertype<CPAnimator>("Animator",
	//	sol::base_classes, sol::bases<AComponent>(),


	//	);
}

void Scripting::Binder::LuaComponentBinder::DelegatePhysics(sol::state& pLuaState, std::string DelegateString)
{
	if (DelegateFromString(pLuaState, DelegateString))
	{
		getFunctionFromString(DelegateString) = pLuaState[DelegateString];
	}
}

bool Scripting::Binder::LuaComponentBinder::DelegateFromString(sol::state& pLuaState, std::string DelegateString)
{
	if (pLuaState[DelegateString].valid())
	{
		return true;
	}
	else
	{
#ifdef NSHIPPING
		std::string err = DelegateString + " Does not exist";
		service(Editor::Widget::WidgetConsole).errorPrint(err.c_str());
#endif
	}
	return false;
}

std::function<void(void*)>& Scripting::Binder::LuaComponentBinder::getFunctionFromString(std::string pInput)
{
	if (pInput.compare("OnCollisionEnter") == 0)
		return mPhysicsFunctions.OnCollisionEnter;

	if (pInput.compare("OnCollisionStay") == 0)
		return mPhysicsFunctions.OnCollisionStay;

	if (pInput.compare("OnCollisionExit") == 0)
		return mPhysicsFunctions.OnCollisionExit;

	if (pInput.compare("OnTriggerEnter") == 0)
		return mPhysicsFunctions.OnTriggerEnter;

	if (pInput.compare("OnTriggerStay") == 0)
		return mPhysicsFunctions.OnTriggerStay;

	if (pInput.compare("OnTriggerExit") == 0)
		return mPhysicsFunctions.OnTriggerExit;

	return mPhysicsFunctions.OnCollisionEnter;
}