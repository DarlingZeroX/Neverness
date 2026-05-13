/*
 * IScriptRuntime — 剧情脚本「后端运行时」契约（Phase 8B / VGGalgameContract）
 *
 * 中文：将原先仅通过 **GalGameScriptExecutorFactory** 隐式分派的路径，提升为可注册的 **IScriptRuntime**，
 * 便于 Lua / Sequence / 未来 Ink 等多后端并存，以及宿主在 **MountEngineRuntime** 之外追加自定义运行时。
 *
 * 注意：当前阶段 **CreateScriptExecutor** 仍委托工厂加载 **IStoryScriptExecutor**；与文档中
 * **CreateExecution → IStoryExecutionInstance** 的终极形态兼容，可后续在宿主侧包装为执行实例而不改 ABI 名。
 */

#pragma once

#include "../VGGalCoreConfig.h"
#include "IStoryScriptExecutor.h"
#include "VGCore/Include/Core/Core.h"

namespace VisionGal::GalGame
{
	/**
	 * @brief 单一脚本后端（Lua / Sequence / …）的注册单元。
	 *
	 * 中文：**CanLoad** 与 **GetAssetTypeNameID**（或等价规则）对齐；**CreateScriptExecutor** 必须返回
	 * 可由 **StoryScriptSystem** 再经 **Run(ISubsystemBus*, IGalGameContext*)** 驱动的执行器。
	 */
	struct IScriptRuntime : public VGEngineResource
	{
		~IScriptRuntime() override = default;

		/// 中文：调试与日志用短名（如 "Lua"、"Sequence"），**不**用作资产类型 ID。
		virtual String GetRuntimeName() const = 0;

		/// 中文：给定 **VFS/资产路径**，判断本后端是否应承接加载（通常比较扩展名或资产类型 ID）。
		virtual bool CanLoad(const String& assetPath) const = 0;

		/// 中文：同步构造执行器；失败时返回 **nullptr**（宿主会回退到工厂直连或打日志）。
		virtual Ref<IStoryScriptExecutor> CreateScriptExecutor(const String& assetPath) = 0;
	};
}
