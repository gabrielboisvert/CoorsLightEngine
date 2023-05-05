#include "Scripting/ScriptInterpreter.h"



#include "EngineCore/Service/ServiceLocator.h"

#ifdef NSHIPPING
#include "Editor/Widget/WidgetConsole.h"
#endif
using namespace Scripting;

ScriptInterpreter::ScriptInterpreter()
{
	mIsOk = false;
}

ScriptInterpreter::~ScriptInterpreter()
{
	DestroyLuaContext();
}

void ScriptInterpreter::CreateLuaContextAndBindGlobals()
{
	if (!mLuaState)
	{
		mLuaState = std::make_unique<sol::state>();
		mLuaState->open_libraries(sol::lib::base, sol::lib::math, sol::lib::coroutine);

		mLuaBinder.CallBinders(*mLuaState);

		mIsOk = true;

		for (Game::Component::CPScript* behaviour : mBehaviours)
		{
			if(!behaviour->registerToLua(*mLuaState))
				mIsOk = false;
		}

		if (!mIsOk)
		{
#ifdef NSHIPPING
			service(Editor::Widget::WidgetConsole).errorPrint("Script interpreter failed to register scripts. Check your lua scripts.");	
#endif
		}
			
	}
}

void ScriptInterpreter::DestroyLuaContext()
{
	if (mLuaState)
	{
		for (Game::Component::CPScript* behaviour : mBehaviours)
		{
			behaviour->unregisterFromLua();
		}

		mLuaState.reset();
		mIsOk = false;
	}
}

void ScriptInterpreter::RefreshAll()
{
	DestroyLuaContext();
	CreateLuaContextAndBindGlobals();
}

bool ScriptInterpreter::IsInitialized()
{
	return mIsOk;
}

void ScriptInterpreter::unregisterScript(Game::Component::CPScript* pScript)
{
	auto Index = std::find(mBehaviours.begin(), mBehaviours.end(), pScript);
	if(Index != mBehaviours.end())
		mBehaviours.erase(Index);
}