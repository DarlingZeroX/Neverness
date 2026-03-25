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
#include "StoryScript.h"
#include "../Interface/IStoryScriptSystem.h"
#include "../Interface/IGalGameEngine.h"
#include "VGCore/Include/Core/Core.h"
#include "VGCore/Include/Utils/TransitionHelper.h"
#include "../Core/GalGameContext.h"

namespace VisionGal::GalGame
{
	class VG_GALGAME_API StoryScriptSystem : public IStoryScriptSystem
	{
	public:
		// 剧情脚本相关接口
		bool ReloadStoryScript() override;	/// 重新加载剧情脚本
		bool LoadStoryScript(const String& path) override;	/// 加载指定路径的剧情脚本
		bool LoadStoryScriptOnUpdate(const String& path) override;	/// 在更新时加载指定路径的剧情脚本

		std::string GetCurrentStoryScriptPath() override;
		std::filesystem::file_time_type GetScriptLastWriteTime() const override;

		void DoChoice(const std::string& id, const std::vector<std::string>& options);	/// 处理剧情选择事件。
		void DoInput(const std::string& id, const std::string& title, const std::string& button);	/// 处理输入事件。

		bool LoadSceneStoryScript(IScene* scene);
		bool LoadSceneStoryScriptOnUpdate(IScene* scene);

		void Wait(float duration);	/// 等待指定的时间长度。

		bool LoadArchive(const SaveArchive& archive);

		void Initialise(const Ref<GalGameContext>& galCtx, IGameEngineContext* context);
		void Update();
		void SetEngine(IGalGameEngine* engine);
	private:
		void OnChoiceSelected(const std::string& id, const std::vector<std::string>& options, int currentChoice);	/// 处理剧情选择事件。
		void OnInputSubmitted(const std::string& id, const std::string& text);	/// 处理输入提交事件。
		void ContinueDialogue();

		void AddUpdateCallback(const std::function<void()>& callback);	/// 添加更新回调函数。
		void UpdateWaitState();		/// 更新等待状态。
	private:
		Ref<GalGameContext> m_GalGameContext;
		IScene* m_Scene;
		IGameEngineContext* m_GameEngineContext = nullptr;
		IGalGameEngine* m_GalGameEngine = nullptr;

		Ref<LuaStoryScript> m_StoryScript = nullptr;			// 当前加载的剧情脚本对象，使用 Lua 脚本语言编写。

		struct WaitStruct
		{
			bool IsWait = false;
			TransitionHelper Timer;
		};

		WaitStruct m_Wait;										// 等待结构体，用于处理等待状态和相关的时间管理。

		std::vector<std::function<void()>> m_UpdateCallback;	// 更新回调函数列表，用于在每帧更新时执行特定的操作。
		std::vector<std::function<void()>> m_ArchiveLuaReadCallback;
	};
}
