#pragma once

#include "CPScript.h"

namespace Game::Component
{
	template<typename... Args>
	void CPScript::luaCall(const std::string& pFunctionName, Args&&... pArgs)
	{
		if (mLuaTable.valid())
		{
			if (mLuaTable[pFunctionName].valid())
			{
				sol::protected_function func = mLuaTable[pFunctionName];

				auto funcRes = func.call(mLuaTable, std::forward<Args>(pArgs)...);
				if (!funcRes.valid())
				{
					sol::error err = funcRes;
#ifdef NSHIPPING
					service(Editor::Widget::WidgetConsole).errorPrint(err.what());
#endif
				}
			}
		}
	}
}