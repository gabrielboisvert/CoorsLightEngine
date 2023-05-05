#include "Scripting/Binders/LuaGlobalsBinder.h"
#include "EngineCore/Service/ServiceLocator.h"

#ifdef NSHIPPING
#include "Editor/Widget/WidgetConsole.h" // Logging in the editor console
#include "Editor/Widget/WidgetEditor.h"
#endif
#include "Maths/Utils.h" // Maths utils

#include "Physics/Core/BulPhysicsEngine.h" //Physics --raycast

#include "Game/Data/Actor.h"

#include<iostream>

#define stringify( name ) #name

#include "Game/SceneSys/SceneManager.h"

#include "EngineCore/Core/EngineApp.h"

#include "EngineCore/Thread/ThreadPool.h"



using namespace Scripting::Binder;

std::map<EngineCore::EventSystem::Key, bool> LuaGlobalsBinder::mKeys;
std::map<std::string, EngineCore::EventSystem::Key> LuaGlobalsBinder::mKeyStrings;

void LuaGlobalsBinder::bind(sol::state& pLuaState)
{

	// Bind the Debug global table
	pLuaState.create_named_table("Debug");
	pLuaState["Debug"]["Log"] = [](const char* pMessage) 
	{ 
#ifdef NSHIPPING
		service(Editor::Widget::WidgetConsole).debugPrint(pMessage); 
#endif
	};
	pLuaState["Debug"]["LogInfo"] = [](const char* pMessage) 
	{ 
#ifdef NSHIPPING
		service(Editor::Widget::WidgetConsole).infoPrint(pMessage); 
#endif
};
	pLuaState["Debug"]["LogWarning"] = [](const char* pMessage)
	{ 
#ifdef NSHIPPING
		service(Editor::Widget::WidgetConsole).warningPrint(pMessage); 
#endif
	};
	pLuaState["Debug"]["LogError"] = [](const char* pMessage) 
	{
#ifdef NSHIPPING
		service(Editor::Widget::WidgetConsole).errorPrint(pMessage); 
#endif
	};

	// Bind the Math global table
	pLuaState.create_named_table("Math");
	pLuaState["Math"]["RandomInt"] = [](int pMin, int pMax) { return Maths::randomInt(pMin, pMax); };
	pLuaState["Math"]["RandomFloat"] = [](float pMin, float pMax) { return Maths::randomFloat(pMin, pMax); };
	pLuaState["Math"]["Lerp"] = [](float pA, float pB, float pT) { return Maths::lerp(pA, pB, pT); };

	pLuaState.create_named_table("Physics");
	pLuaState["Physics"]["Raycast"] = [](Maths::FVector3 pStartPos, Maths::FVector3 pEndPos, Physics::Core::CollisionGroups pGroups, int pLayermask) {return Physics::Core::BulPhysicsEngine::Raycast(pStartPos, pEndPos, pGroups, pLayermask); };
	pLuaState["Physics"]["GetActor"] = [](void* Actor) {return (Game::Data::Actor*)Actor; };
	pLuaState.new_usertype<Physics::Data::RaycastCallback>("RaycastCallback", 
		sol::constructors<void()>(),
		"CollisionPoint", &Physics::Data::RaycastCallback::mCollisionPoint,
		"CollisionNormal", &Physics::Data::RaycastCallback::mCollisionNormal,
		"CollisionObject", &Physics::Data::RaycastCallback::mCollisionObject);


	//INPUTS
	SetKeys();
	pLuaState.create_named_table("Input");
	pLuaState["Input"]["GetKeyPress"] = [this](const char* Input)
	{
#ifdef NSHIPPING
		if (!service(EngineCore::Core::EngineApp).mMouseLock)
			return false;
#endif
		std::string string = Input;
		if (mKeyStrings.find(string) == mKeyStrings.end())
			return false;
		EngineCore::EventSystem::Key Key = mKeyStrings[string];
		return mKeys[Key];
	};
	pLuaState["Input"]["GetMousePosition"] = [this]()
	{
#ifdef NSHIPPING
		service(EngineCore::EventSystem::InputManager).processMousePosition();
#endif
		return service(EngineCore::EventSystem::InputManager).mMouse.mPosition;
	};
	pLuaState["Input"]["GetMouseDifference"] = [this]()
	{
#ifdef NSHIPPING
		if (!service(EngineCore::Core::EngineApp).mMouseLock)
			return Maths::FVector2(0);
#endif

#ifdef NSHIPPING
		service(EngineCore::EventSystem::InputManager).processMousePosition();
#endif
#ifdef SHIPPING
		if (service(EngineCore::Core::EngineApp).mWindow.IsActiveWindow())
		{
			SetCursorPos(service(EngineCore::Core::EngineApp).mScreenCenter.x , service(EngineCore::Core::EngineApp).mScreenCenter.y);
			return service(EngineCore::EventSystem::InputManager).mMouse.getPositionDifferenceLua(service(EngineCore::Core::EngineApp).mScreenCenter);
		}
		return Maths::FVector2(0);
#else

		Maths::FVector2 ScreenCenter = service(Editor::Widget::WidgetGameApp).calculateCursorPos();
		SetCursorPos(ScreenCenter.x, ScreenCenter.y);
		return service(EngineCore::EventSystem::InputManager).mMouse.getPositionDifferenceLua(ScreenCenter);
#endif
	};
	pLuaState["Input"]["UnlockMouse"] = [this]()
	{
		service(EngineCore::Core::EngineApp).mMouseLock = false;
	};
	pLuaState["Input"]["LockMouse"] = [this]()
	{
		service(EngineCore::Core::EngineApp).mMouseLock = true;
	};

	pLuaState["Input"]["ShowMouse"] = [this]()
	{
		service(EngineCore::Core::EngineApp).mWindow.changeCursorMode(true);
	};

	pLuaState["Input"]["HideMouse"] = [this]()
	{
		service(EngineCore::Core::EngineApp).mWindow.changeCursorMode(false);
	};

	pLuaState.create_named_table("Scenes");
	pLuaState["Scenes"]["GetCurrentScene"] = [this]()
	{
		return service(Game::SceneSys::SceneManager).mCurrentScene;
	};

	pLuaState["Scenes"]["LoadScene"] = [this](std::string pNewScene)
	{
		std::string path = pNewScene + ".map";

#ifdef NSHIPPING
		QMetaObject::invokeMethod(&service(Editor::Widget::WidgetEditor), [this, path] {service(Editor::Widget::WidgetEditor).openSceneWithPath(QString(service(Editor::Widget::WidgetEditor).mLocation.mFolder + path.c_str())); });
#else
		service(Game::SceneSys::SceneManager).loadSceneLua(path.c_str());
		service(EngineCore::Core::EngineApp).initScene();
#endif
	};



	pLuaState.create_named_table("Timer");
	pLuaState["Timer"]["WaitUntil"] = [&pLuaState, this](float SecondsToWait, std::string pFunctionName)->void
	{
		if (pLuaState[pFunctionName].valid() && std::find(CalledFunctionsInLua.begin(), CalledFunctionsInLua.end(), pFunctionName) == CalledFunctionsInLua.end())
		{
			CalledFunctionsInLua.emplace_back(pFunctionName);
			service(EngineCore::Thread::ThreadPool).queueJob([this, &pLuaState, pFunctionName, SecondsToWait]
			{
					std::this_thread::sleep_for(std::chrono::duration<float>(SecondsToWait));

					pLuaState[pFunctionName]();
					CalledFunctionsInLua.erase(std::find(CalledFunctionsInLua.begin(), CalledFunctionsInLua.end(), pFunctionName));
			});
		}
	};
}

void Scripting::Binder::LuaGlobalsBinder::SetKeys()
{
	for (int i = 1; i <= (int)EngineCore::EventSystem::Key::LEFT_CTRL; i += 1)
	{
		service(EngineCore::EventSystem::InputManager).bindActionCallBack(EngineCore::EventSystem::KState::PRESS, (EngineCore::EventSystem::Key)i,
			[this, i] 
			{
				this->mKeys[(EngineCore::EventSystem::Key)i] = true;
			}
			);

		service(EngineCore::EventSystem::InputManager).bindActionCallBack(EngineCore::EventSystem::KState::RELEASE, (EngineCore::EventSystem::Key)i,
			[this, i]
			{
				this->mKeys[(EngineCore::EventSystem::Key)i] = false;
			}
			);
		mKeys[(EngineCore::EventSystem::Key)i] = false;
	}


	this->mKeyStrings.emplace("A", EngineCore::EventSystem::Key::A);
	this->mKeyStrings.emplace("B", EngineCore::EventSystem::Key::B);
	this->mKeyStrings.emplace("C", EngineCore::EventSystem::Key::C);
	this->mKeyStrings.emplace("D", EngineCore::EventSystem::Key::D);
	this->mKeyStrings.emplace("E", EngineCore::EventSystem::Key::E);
	this->mKeyStrings.emplace("F", EngineCore::EventSystem::Key::F);
	this->mKeyStrings.emplace("G", EngineCore::EventSystem::Key::G);
	this->mKeyStrings.emplace("H", EngineCore::EventSystem::Key::H);
	this->mKeyStrings.emplace("I", EngineCore::EventSystem::Key::I);
	this->mKeyStrings.emplace("J", EngineCore::EventSystem::Key::J);
	this->mKeyStrings.emplace("K", EngineCore::EventSystem::Key::K);
	this->mKeyStrings.emplace("L", EngineCore::EventSystem::Key::L);
	this->mKeyStrings.emplace("M", EngineCore::EventSystem::Key::M);
	this->mKeyStrings.emplace("N", EngineCore::EventSystem::Key::N);
	this->mKeyStrings.emplace("O", EngineCore::EventSystem::Key::O);
	this->mKeyStrings.emplace("P", EngineCore::EventSystem::Key::P);
	this->mKeyStrings.emplace("Q", EngineCore::EventSystem::Key::Q);
	this->mKeyStrings.emplace("R", EngineCore::EventSystem::Key::R);
	this->mKeyStrings.emplace("S", EngineCore::EventSystem::Key::S);
	this->mKeyStrings.emplace("T", EngineCore::EventSystem::Key::T);
	this->mKeyStrings.emplace("U", EngineCore::EventSystem::Key::U);
	this->mKeyStrings.emplace("V", EngineCore::EventSystem::Key::V);
	this->mKeyStrings.emplace("W", EngineCore::EventSystem::Key::W);
	this->mKeyStrings.emplace("X", EngineCore::EventSystem::Key::X);
	this->mKeyStrings.emplace("Y", EngineCore::EventSystem::Key::Y);
	this->mKeyStrings.emplace("Z", EngineCore::EventSystem::Key::Z);
	this->mKeyStrings.emplace("ESCAPE", EngineCore::EventSystem::Key::ESCAPE);
	this->mKeyStrings.emplace("1", EngineCore::EventSystem::Key::_1);
	this->mKeyStrings.emplace("2", EngineCore::EventSystem::Key::_2);
	this->mKeyStrings.emplace("3", EngineCore::EventSystem::Key::_3);
	this->mKeyStrings.emplace("4", EngineCore::EventSystem::Key::_4);
	this->mKeyStrings.emplace("5", EngineCore::EventSystem::Key::_5);
	this->mKeyStrings.emplace("6", EngineCore::EventSystem::Key::_6);
	this->mKeyStrings.emplace("7", EngineCore::EventSystem::Key::_7);
	this->mKeyStrings.emplace("8", EngineCore::EventSystem::Key::_8);
	this->mKeyStrings.emplace("9", EngineCore::EventSystem::Key::_9);
	this->mKeyStrings.emplace("TAB", EngineCore::EventSystem::Key::TAB);
	this->mKeyStrings.emplace("ENTER", EngineCore::EventSystem::Key::ENTER);
	this->mKeyStrings.emplace("LEFT_SHIFT", EngineCore::EventSystem::Key::LEFT_SHIFT);
	this->mKeyStrings.emplace("LEFT_CTRL", EngineCore::EventSystem::Key::LEFT_CTRL);
}
