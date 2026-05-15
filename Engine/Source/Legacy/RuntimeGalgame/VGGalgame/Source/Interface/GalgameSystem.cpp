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

#include "GalgameSystem.h"
#include "VGGalgameCore/Include/Components.h"
#include "GalGameEngine.h"
#include "VGAsset/Interface/SceneSerializerFactory.h"
#include "VGEngine/Include/Scene/GameActorFactory.h"
#include "VGEngine/Interface/CoreLua.h"
#include "VGGalgameLuaRuntime/Interface/LuaBinding.h"
#include "VGGalgameLuaRuntime/Module.h"
#include "VGGalgameSequenceRuntime/Module.h"

namespace VisionGal
{
	struct GalGameEngineGameActorBuilder : IGameActorBuilder
	{
		std::string GetType() const override { return "GalGameEngine"; }
		IGameActor* BuildActor(IGameActor* emptyActor) override
		{
			emptyActor->SetLabel(Horizon::GetTranslateText("GalGame Engine"));

			auto com = emptyActor->AddComponent<GalGame::GalGameEngineComponent>();
			return emptyActor;
		};
	};

	void GalGameSystem::Initialize(CoreGameEngine& engine)
	{		
		// 创建GalGame子引擎，因为有些系统还没初始化，比如UI系统
		auto galgameEngine = MakeRef<GalGame::GalGameEngine>();
		galgameEngine->Initialize(engine.GetContext());
		engine.AddSubGameEngine(galgameEngine);

		// 注册GalGameEngine的Actor Builder
		GetGameActorFactory()->AddGameActorCreator(MakeRef<GalGameEngineGameActorBuilder>());

		// 注册默认的场景序列化器
		SceneSerializerRegistry::RegisterSegmentSerializer(
			GalGame::GalGameEngineComponentSerializer{}.GetSegmentType() , 
			MakeRef<GalGame::GalGameEngineComponentSerializer>()
		);

		// 注册LuaAPI
		CoreLua::RegisterGlobalAPI([](sol::state* luaState)
		{
			// 在这里注册GalGame相关的Lua API
			GalGame::GalGameLuaBinding::Register(*luaState);
		});

		// 注册Lua脚本模块
		GalGame::GalGameLuaScriptModule::MountEngineRuntime();
		GalGame::GalGameSequenceScriptModule::MountEngineRuntime();
	}
}
