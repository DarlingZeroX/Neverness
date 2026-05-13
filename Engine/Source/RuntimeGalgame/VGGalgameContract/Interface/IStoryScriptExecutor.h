/*
 * IStoryScriptExecutor — 剧情脚本执行器契约（Phase 8：VGGalgameContract）
 *
 * 与资产工厂 GalGameScriptExecutorFactory 分离，供 VGGalgameSequenceRuntime 等模块
 * 仅依赖契约头，不依赖上层 Galgame 宿主实现库。
 */

#pragma once

// CORE ABI STABLE
// DO NOT MODIFY WITHOUT VERSION BUMP
// 中文：对外二进制/脚本绑定若有变更，须同步提升存档版本号并更新 Lua 绑定说明。

#include "../VGGalCoreConfig.h"
#include "IGalGameContext.h"
#include "ISubsystemBus.h"
#include "VGCore/Include/Core/Core.h"
#include "VGCore/Interface/Interface.h"

#include <filesystem>
#include <string>
#include <vector>

namespace VisionGal::GalGame
{
	/**
	 * @brief 剧情脚本执行器抽象：由 Lua / Sequence 等后端实现。
	 *
	 * 启动时仅依赖子系统总线与 Gal 上下文，不直接持有 IGalGameEngine（SubsystemBus 架构）。
	 */
	struct IStoryScriptExecutor : public VGEngineResource
	{
		~IStoryScriptExecutor() override = default;

		/// @param bus 子系统总线（宿主注入）；不得为 nullptr 时调用 Run（调用方负责校验）。
		virtual bool Run(ISubsystemBus* bus, IGalGameContext* gameContext) = 0;
		virtual void Tick(float deltaTime) = 0;
		virtual IRuntimeInterface* QueryInterface(InterfaceID id) = 0;

		virtual void PreLoadScriptResource() = 0;
		virtual std::filesystem::file_time_type GetScriptLastWriteTime() const = 0;

		virtual void ContinueDialogue() = 0;
		virtual void OnChoiceSelected(const std::string& id, const std::vector<std::string>& options, int currentChoice) = 0;
		virtual void OnInputSubmitted(const std::string& id, const std::string& text) = 0;
	};

	/** 由具体模块提供的「从资产路径构造执行器」工厂契约。 */
	struct IStoryScriptExecutorCreator
	{
		virtual ~IStoryScriptExecutorCreator() = default;

		virtual Ref<IStoryScriptExecutor> LoadFromAsset(const String& path) = 0;
	};
}
