/*
 * IExecutionScheduler — Gal 运行时任务调度（Phase 8.3 Contract）
 *
 * 中文：统一 Wait / Continue / Auto / Skip / Transition / Choice 等「可调度单元」的入口；
 * 当前为骨架 API，默认实现由 VGGalgame 内 GalDefaultExecutionScheduler 提供，逐步从 StoryScriptSystem 迁移逻辑。
 */

#pragma once
#include "../VGGalCoreConfig.h"
#include <cstdint>

namespace VisionGal::GalGame
{
	/**
	 * 中文：opaque 任务句柄；0 表示无效。后续可换为稳定 ID 或池化索引。
	 */
	using GalExecutionHandle = std::uint64_t;

	struct IExecutionScheduler
	{
		virtual ~IExecutionScheduler() = default;

		/// 中文：提交异步/延迟任务（占位）；返回句柄供 Cancel/Pause。
		virtual GalExecutionHandle SubmitWait(float seconds) = 0;

		virtual void Cancel(GalExecutionHandle handle) = 0;
		virtual void Pause(GalExecutionHandle handle) = 0;
		virtual void Resume(GalExecutionHandle handle) = 0;

		virtual void PauseAll() = 0;
		virtual void ResumeAll() = 0;

		/// 中文：每帧调用一次；驱动剧情执行实例、等待计时等。
		virtual void Tick(float deltaTime) = 0;
	};
}
