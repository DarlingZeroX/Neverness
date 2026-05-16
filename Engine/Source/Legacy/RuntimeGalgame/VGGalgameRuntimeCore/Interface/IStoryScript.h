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
 * 中文说明（剧情脚本执行器工厂）：
 * - `IStoryScriptExecutor` / `IStoryScriptExecutorCreator` 的契约位于同模块的
 *   `IStoryScriptExecutor.h`（Phase 7 已迁入 VGGalgameCore）。
 * - 本头文件提供单例 **`GalGameScriptExecutorFactory`**：按「资产类型 ID」注册创建器，
 *   在 `StoryScriptSystem::LoadStoryScript` 中通过 `LoadAssetExecutor(type, path)` 实例化
 *   Lua、Sequence 等具体执行器；Lua / Sequence 模块在 `MountEngineRuntime` 时调用
 *   `RegisterAssetExecutor` 完成注册。
 * - 仅依赖 VGGalgameCore + VGCore 基础类型（`Ref` / `String`），不依赖 VGGalgame 宿主实现。
 */

#pragma once
#include "../VGGRCExport.h"
#include "VGGalgameContract/Interface/IStoryScriptExecutor.h"
#include "NNRuntimeCore/Include/Core/Core.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace VisionGal::GalGame
{
	/**
	 * 中文：全局剧情脚本执行器工厂（进程内单例）。
	 * - `RegisterAssetExecutor`：由各脚本后端模块在启动时注册 type → Creator。
	 * - `LoadAssetExecutor`：由 `StoryScriptSystem` 在加载脚本时按路径解析出的 type 创建执行器实例。
	 */
	struct VG_RUNTIME_GALCORE_API GalGameScriptExecutorFactory
	{
		virtual Ref<IStoryScriptExecutor> LoadAssetExecutor(const String& type, const String& path);

		virtual void RegisterAssetExecutor(const String& type, Ref<IStoryScriptExecutorCreator> factoryFunction);

		static GalGameScriptExecutorFactory& Get();

		std::vector<std::string> GetRegisterTypes();

		bool HasExecutor(const String& type);
	private:
		std::unordered_map<String, Ref<IStoryScriptExecutorCreator>> m_FactoryFunctions;
	};

}
