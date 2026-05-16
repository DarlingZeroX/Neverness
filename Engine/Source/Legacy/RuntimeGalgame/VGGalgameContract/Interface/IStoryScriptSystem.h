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
 * CORE ABI STABLE
 * DO NOT MODIFY WITHOUT VERSION BUMP
 *
 * 中文（Phase 8）：`SaveArchive` 仅前向声明；完整类型定义在 **VGGalgameRuntimeCore**。
 */

#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <NNRuntimeCore/Include/Data/DataVariant.h>
#include <NNRuntimeCore/Interface/Interface.h>
#include <NNRuntimeCore/Interface/SceneInterface.h>

namespace VisionGal::GalGame
{
	struct ISubsystemBus;
	struct SaveArchive;

	struct IStoryExecutionInstance
	{
		virtual ~IStoryExecutionInstance() = default;

		/// @param bus 子系统总线（由宿主注入）；内核与 RuntimeSystem 经执行上下文访问，不得再持有 God Engine。
		virtual void Tick(float deltaTime, ISubsystemBus* bus) = 0;
		virtual void Continue(ISubsystemBus* bus) = 0;
		virtual IRuntimeInterface* QueryInterface(InterfaceID id) = 0;

		template<typename T>
		T* ExecutionQuery()
		{
			/// 中文：当前仍基于 `typeid`；后续可改为稳定 InterfaceID 表以减少 RTTI 依赖。
			return static_cast<T*>(QueryInterface(typeid(T)));
		}
	};

	struct IStoryScriptSystem
	{
		virtual ~IStoryScriptSystem() = default;

		// 剧情脚本相关接口
		virtual IStoryExecutionInstance* GetExecutionInstance(unsigned int id = 0) const = 0;
		virtual bool ReloadStoryScript() = 0;	/// 重新加载剧情脚本
		virtual bool LoadStoryScript(const std::string& path) = 0;	/// 加载指定路径的剧情脚本
		virtual bool LoadStoryScriptOnUpdate(const std::string& path) = 0;	/// 在更新时加载指定路径的剧情脚本

		virtual std::string GetCurrentStoryScriptPath() = 0;
		virtual std::filesystem::file_time_type GetScriptLastWriteTime() const = 0;

		virtual void DoChoice(const std::string& id, const std::vector<std::string>& options) = 0;	/// 处理剧情选择事件。
		virtual void DoInput(const std::string& id, const std::string& title, const std::string& button) = 0;	/// 处理输入事件。

		virtual bool LoadSceneStoryScript(IScene* scene) = 0;
		virtual bool LoadSceneStoryScriptOnUpdate(IScene* scene) = 0;

		virtual void Wait(float duration) = 0;	/// 等待指定的时间长度。

		virtual bool LoadArchive(const SaveArchive& archive) = 0;
	};
}
