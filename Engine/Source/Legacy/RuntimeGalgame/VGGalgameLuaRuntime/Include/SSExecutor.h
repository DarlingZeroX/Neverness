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

#pragma once
#include "../VGGalgameScriptLuaConfig.h"
#include <sol/state.hpp>
#include <sol/coroutine.hpp>
#include "NNRuntimeCore/Include/Core/Core.h"
#include "VGGalgameCore/Interface/IGalGameContext.h"
#include "VGGalgameCore/Interface/ISubsystemBus.h"
#include "VGGalgameCore/Interface/IGameEngine.h"
#include "VGGalgameCore/Interface/IStoryScript.h"

namespace VisionGal::GalGame
{
	/// 中文：Lua 剧情脚本执行期注入聚合（Phase 8）；后续 LuaBinding 应只读此结构而非 GalGameEngineAccess。
	struct ScriptExecutionContext
	{
		ISubsystemBus* Bus = nullptr;
		IGalGameContext* Context = nullptr;
		IGalGameEngine* HostEngine = nullptr;
	};

	class VG_GALGAME_SCRIPT_LUA_API LuaStoryScript : public IStoryScriptExecutor
	{ 
	public:
		LuaStoryScript();
		~LuaStoryScript() override = default;

		static Ref<LuaStoryScript> LoadFromFile(const String& file);
		bool Run(ISubsystemBus* bus, IGalGameContext* gameContext) override;
		void Tick(float deltaTime) override;
		IRuntimeInterface* QueryInterface(InterfaceID id) override;

		sol::coroutine GetCoroutine() { return m_Coroutine; }
		void PreLoadScriptResource() override;
		std::filesystem::file_time_type GetScriptLastWriteTime() const override { return m_ScriptLastWriteTime; }

		void ContinueDialogue() override;
		void OnChoiceSelected(const std::string& id,const std::vector<std::string>& options,int currentChoice) override; 
		void OnInputSubmitted(const std::string& id, const std::string& text) override; 
	private:
		bool LoadScript(const String& file);
	private:
		ScriptExecutionContext m_ScriptExecution;
		ISubsystemBus* m_Bus = nullptr;
		IGalGameEngine* m_Engine = nullptr;
		String m_ScriptCode;
		sol::state m_LuaState;
		sol::coroutine m_Coroutine;
		std::filesystem::file_time_type m_ScriptLastWriteTime;
	};
}
