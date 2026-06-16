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

#include "CoreLua.h"
#include "NNRuntimeCore/Include/Core/EventBus.h"
#include "Lua/LuaInterface.h"
#include "NNRuntimeRmlui/Include/Sol/Sol.h"
#include "NNRuntimeRmlui/Source/Sol/SolPlugin.h"

namespace NN::Runtime
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
			g_CoreLuaState = MakeRef<sol::state>();

			g_CoreLuaState->open_libraries(sol::lib::base,
				sol::lib::math,
				sol::lib::string,
				sol::lib::table); // 默认已加载这些库

			// 绑定默认API
			RmlSol::Initialise(g_CoreLuaState);
			VGLuaInterface::Initialise(*g_CoreLuaState);

			// 绑定注册 API
			for (auto& api : g_GlobalLuaAPIs)
			{
				api(g_CoreLuaState.get());
			}
		}

		void ResetLuaState()
		{
			g_CoreLuaState = MakeRef<sol::state>();

			g_CoreLuaState->open_libraries(sol::lib::base,
				sol::lib::math,
				sol::lib::string,
				sol::lib::table); // 默认已加载这些库

			// 重新绑定默认API
			RmlSol::SolPlugin::RebindLuaState(g_CoreLuaState);
			VGLuaInterface::Initialise(*g_CoreLuaState);

			// 重新绑定注册 API
			for (auto& api : g_GlobalLuaAPIs)
			{
				api(g_CoreLuaState.get());
			}
		}

		Ref<sol::state> g_CoreLuaState = nullptr;
		std::vector<std::function<void(sol::state*)>> g_GlobalLuaAPIs;
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

	void CoreLua::RegisterGlobalAPI(const std::function<void(sol::state*)>& api)
	{
		CoreLuaImp::Get()->g_GlobalLuaAPIs.push_back(api);
		// 立即绑定到当前的 Lua 状态
		if (CoreLuaImp::Get()->g_CoreLuaState != nullptr)
		{
			api(CoreLuaImp::Get()->g_CoreLuaState.get());
		}

	}
}
