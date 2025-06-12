#pragma	once

#include "Game/Component/AComponent.h"

#include "EngineCore/Service/ServiceLocator.h"

#include <sol.hpp>
#include <string>

namespace Game::Component
{
	class CPScript : public AComponent
	{
	public:
		sol::state* mLuaState = nullptr;

		bool temp = true;

		AComponent* clone() override;
		~CPScript() override;

		void setScript(const std::string& pName, const char* pScript);
		void setScript(const char* pPath);

		bool registerToLua(sol::state& pLuaState);
		void unregisterFromLua();

		template<typename... Args>
		void luaCall(const std::string& pFunctionName, Args&&... pArgs);

		void OnStart() override;
		void OnUpdate(float pDeltaTime) override;

		void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& pWriter) override;

		void printError(const char* error);

	public:
		sol::table mLuaTable = sol::nil;

		std::string mPath;
		std::string mName = "NewClass";
	};
}

#include "CPScript.inl"