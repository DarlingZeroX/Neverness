#include "Lua/RenderLuaInterface.h"
#include "Render/TransitionManager.h"

namespace VisionGal
{
	namespace Lua
	{
		void TransitionManagerLuaInterface::Initialize(sol::state& lua)
		{
			lua.new_usertype<TransitionManager>("TransitionManagerClass",
				"StartTransitionWithCommand", &TransitionManager::StartTransitionWithCommand,
				"开始转场命令", &TransitionManager::StartTransitionWithCommand
			);

			lua["TransitionManager"] = sol::make_object(lua, TransitionManager::GetInstance());
			lua["转场管理器"] = sol::make_object(lua, TransitionManager::GetInstance());

			//lua.set_function("GetTransitionManager", []() { return TransitionManager::GetInstance(); });
		}
	}

	void RenderLuaInterface::Initialize(sol::state& L)
	{
		Lua::TransitionManagerLuaInterface::Initialize(L);
	}
}


