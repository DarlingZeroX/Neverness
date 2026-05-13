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
 * - 实现 **`IStoryScriptSystem`**：负责从场景 **`GalGameEngineComponent`** 读取脚本路径、
 *   通过 **`GalGameScriptExecutorFactory`** 按资产类型加载 **`IStoryScriptExecutor`**、
 *   驱动 **`Run` / `Tick` / `ContinueDialogue`**，并把 UI 选择/输入与存档回放桥接到执行器。
 * - **`Initialise` / `Update`**：由 **`GalGameEngine`** 装配与每帧调用；内部用延迟回调队列
 *   处理「在 Update 阶段再加载脚本」等需求，避免在事件中间重入加载。
 * - **`LoadArchive`**：与 **`SaveArchive`** 协作，在加载存档时快进对白行直至目标行号。
 */

#pragma once

#include "../../VGGalgameConfig.h"
#include <functional>

#include "StoryExecutionInstance.h"
#include "VGGalgameCore/Interface/IStoryScript.h"
#include "VGCore/Include/Core/Core.h"
#include "VGCore/Include/Utils/TransitionHelper.h"
#include "VGGalgameCore/Interface/IStoryScriptSystem.h"
#include "VGGalgameCore/Interface/IGameEngine.h"
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

		void Initialise(const Ref<GalGameContext>& galCtx, IGameEngineContext* context);
		void Update();
		void SetEngine(IGalGameEngine* engine);
	private:
		void OnChoiceSelected(const std::string& id, const std::vector<std::string>& options, int currentChoice);
		void OnInputSubmitted(const std::string& id, const std::string& text);
		void ContinueDialogue();

		void AddUpdateCallback(const std::function<void()>& callback);
		void UpdateWaitState();
	private:
		Ref<GalGameContext> m_GalGameContext;
		IScene* m_Scene;
		IGameEngineContext* m_GameEngineContext = nullptr;
		IGalGameEngine* m_GalGameEngine = nullptr;

		Ref<IStoryScriptExecutor> m_StoryScript = nullptr;
		Ref<StoryExecutionInstance> m_ExecutionInstance = nullptr;

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
