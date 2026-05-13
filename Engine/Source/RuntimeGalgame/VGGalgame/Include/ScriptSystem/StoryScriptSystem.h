/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
 * GitHub page: https://github.com/DarlingZeroX/VisionGal
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 *
 * 中文说明（剧情脚本宿主实现）：
 * - 实现 **`IStoryScriptSystem`**：从场景 **`GalGameEngineComponent`** 读取脚本路径；加载路径由 **`GalRuntimeScriptLoader`**
 *   统一（**`GalScriptRuntimeRegistry`** 优先 → **`GalGameScriptExecutorFactory`**）；**`TryCreateStoryExecution`** 非空时直接使用内核 **IStoryExecutionInstance**。
 * - **`Initialise` / `Update`**：由 **`GalGameEngine`** 装配与每帧调用；**`ISubsystemBus*`** 由宿主注入，**禁止**再持有 **`IGalGameEngine*`**。
 * - **`LoadArchive`**：与 **`SaveArchive`** 协作，在加载存档时快进对白行直至目标行号。
 */

#pragma once

#include "../../VGGalgameConfig.h"
#include <functional>

#include "VGCore\Interface\GameEngineInterface.h"
#include "ScriptSystem/GalRuntimeScriptLoader.h"
#include "ScriptSystem/GalScriptRuntimeRegistry.h"
#include "StoryExecutionInstance.h"
#include "VGGalgameCore/Interface/IStoryScript.h"
#include "VGCore/Include/Core/Core.h"
#include "VGCore/Include/Utils/TransitionHelper.h"
#include "VGGalgameContract/Interface/ISubsystemBus.h"
#include "VGGalgameCore/Interface/IStoryScriptSystem.h"
#include "VGGalgameCore/Include/GalGameContext.h"

namespace VisionGal::GalGame
{
	class VG_GALGAME_API StoryScriptSystem : public IStoryScriptSystem
	{
	public:
		bool ReloadStoryScript() override;
		bool LoadStoryScript(const String& path) override;
		bool LoadStoryScriptOnUpdate(const String& path) override;

		IStoryExecutionInstance* GetExecutionInstance(unsigned int id = 0) const override;

		std::string GetCurrentStoryScriptPath() override;
		std::filesystem::file_time_type GetScriptLastWriteTime() const override;

		void DoChoice(const std::string& id, const std::vector<std::string>& options) override;
		void DoInput(const std::string& id, const std::string& title, const std::string& button) override;

		bool LoadSceneStoryScript(IScene* scene) override;
		bool LoadSceneStoryScriptOnUpdate(IScene* scene) override;

		void Wait(float duration) override;

		bool LoadArchive(const SaveArchive& archive) override;

		void Initialise(const Ref<GalGameContext>& galCtx, IGameEngineContext* context, ISubsystemBus* subsystemBus);
		void Update();

		/// 中文：供宿主或测试追加 **`IScriptRuntime`**；后注册者优先于先注册者匹配路径。
		GalScriptRuntimeRegistry& GetScriptRuntimeRegistry() noexcept { return m_RuntimeRegistry; }
		const GalScriptRuntimeRegistry& GetScriptRuntimeRegistry() const noexcept { return m_RuntimeRegistry; }

		/// 中文：由 **GalRuntimeCoordinator** 在 **ResetRuntime** / **Shutdown** 时调用：卸载执行器、清空延迟队列；**不**再写 **GalGameRuntimeState**（见协调器）。
		void ResetExecutionPipeline();
	private:
		void RegisterBuiltinScriptRuntimes();
		void OnChoiceSelected(const std::string& id, const std::vector<std::string>& options, int currentChoice);
		void OnInputSubmitted(const std::string& id, const std::string& text);
		void ContinueDialogue();

		void AddUpdateCallback(const std::function<void()>& callback);
		void UpdateWaitState();

		Ref<GalGameContext> m_GalGameContext;
		IScene* m_Scene;
		IGameEngineContext* m_GameEngineContext = nullptr;
		ISubsystemBus* m_SubsystemBus = nullptr;

		GalScriptRuntimeRegistry m_RuntimeRegistry;
		GalRuntimeScriptLoader m_ScriptLoader;

		Ref<IStoryScriptExecutor> m_StoryScript = nullptr;
		Ref<IStoryExecutionInstance> m_ExecutionInstance = nullptr;

		struct WaitStruct
		{
			bool IsWait = false;
			TransitionHelper Timer;
		};

		WaitStruct m_Wait;

		std::vector<std::function<void()>> m_UpdateCallback;
		std::vector<std::function<void()>> m_ArchiveLuaReadCallback;
	};
}
