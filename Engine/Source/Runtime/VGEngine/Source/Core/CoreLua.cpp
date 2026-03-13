/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*
* Copyright (c) 2025-present 梦旅缘心
*
* See the LICENSE file in the project root for details.
*/

#include "Core/CoreLua.h"

#include "Core/EventBus.h"
#include "Galgame/Lua/GameLua.h"
#include "Lua/LuaInterface.h"
#include "UI/Sol/Sol.h"
#include "VGEngine/Source/UI/Sol/SolPlugin.h"

namespace VisionGal
{
	struct CoreLuaImp
	{
		CoreLuaImp()
		{
		}

		~CoreLuaImp()
		{
			
		}

		static CoreLuaImp* Get()
		{
			static CoreLuaImp imp;
			return &imp;
		}

		void Initialize()
		{
			InitializeLuaState();
			static Ref<sol::state> lastState = g_CoreLuaState;

			// 当进入游戏播放模式时，我们需要一个新的干净的 Lua 状态，以防止脚本污染以及数据遗留问题
			EngineEventBus::Get().OnEngineEvent.Subscribe([this](const EngineEvent& evt)
				{
					switch (evt.EventType)
					{
					case EngineEventType::EnterScenePlayMode:
						lastState = g_CoreLuaState;
						ResetLuaState();
						break;
					}
				});
		}

		void InitializeLuaState()
		{
			g_CoreLuaState = CreateRef<sol::state>();

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

		void ResetLuaState()
		{
			g_CoreLuaState = CreateRef<sol::state>();

			g_CoreLuaState->open_libraries(sol::lib::base,
				sol::lib::math,
				sol::lib::string,
				sol::lib::table); // 默认已加载这些库

			// 创建命名空间
			sol::table galgameNS = g_CoreLuaState->create_named_table("GalGame");

			RmlSol::SolPlugin::RebindLuaState(g_CoreLuaState);
			VGLuaInterface::Initialise(*g_CoreLuaState);
			GalGame::GalGameLuaInterface::Initialise(galgameNS);
		}

		Ref<sol::state> g_CoreLuaState = nullptr;
	};

	void CoreLua::Initialize()
	{
		CoreLuaImp::Get()->Initialize();
	}

	sol::state* CoreLua::GetCoreLuaState()
	{
		return CoreLuaImp::Get()->g_CoreLuaState.get();
	}

	void CoreLua::Update()
	{
	}
}
