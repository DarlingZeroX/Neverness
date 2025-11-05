#include "Core/CoreLua.h"

#include "Galgame/GameLua.h"
#include "Lua/LuaInterface.h"
#include "UI/Sol/Sol.h"

namespace VisionGal
{
	struct CoreLuaImp
	{
		CoreLuaImp()
		{
		}

		static CoreLuaImp* Get()
		{
			static CoreLuaImp imp;
			return &imp;
		}

		void Initialize()
		{
			g_CoreLuaState = new sol::state();

			g_CoreLuaState->open_libraries(sol::lib::base,
				sol::lib::math,
				sol::lib::string,
				sol::lib::table); // 默认已加载这些库

			// 创建命名空间
			sol::table galgameNS = g_CoreLuaState->create_named_table("GalGame");

			RmlSol::Initialise(g_CoreLuaState);
			//sol::state* solState = RmlSol::SolPlugin::GetLuaState();
			VGLuaInterface::Initialise(*g_CoreLuaState);
			GalGame::GalGameLuaInterface::Initialise(galgameNS);
		}

		sol::state* g_CoreLuaState = nullptr;
	};

	void CoreLua::Initialize()
	{
		CoreLuaImp::Get()->Initialize();
	}

	sol::state* CoreLua::GetCoreLuaState()
	{
		return CoreLuaImp::Get()->g_CoreLuaState;
	}
}
