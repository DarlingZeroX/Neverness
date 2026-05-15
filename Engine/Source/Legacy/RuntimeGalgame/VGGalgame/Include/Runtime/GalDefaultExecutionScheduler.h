/*
 * GalDefaultExecutionScheduler — IExecutionScheduler 默认实现（Phase 8.3 / VGGalgame）
 *
 * 中文：将「剧情脚本系统 Update」纳入调度器 Tick，与 **GalRuntimeCoordinator::TickFrame** 顺序对齐；
 * **SubmitYield** 将 **GalYieldKind::WaitSeconds** 映射到 **StoryScriptSystem::Wait**；**Signal*** 类为 Phase 8C 统一信号恢复预留句柄。
 */

#pragma once
#include "../VGGalgameConfig.h"
#include "VGGalgameContract/Interface/IExecutionScheduler.h"

namespace VisionGal::GalGame
{
	class StoryScriptSystem;

	struct VG_GALGAME_API GalDefaultExecutionScheduler final : public IExecutionScheduler
	{
		explicit GalDefaultExecutionScheduler(StoryScriptSystem* scriptSystem) noexcept;

		GalExecutionHandle SubmitWait(float seconds) override;
		GalExecutionHandle SubmitYield(const GalYieldInstruction& instruction) override;
		void Cancel(GalExecutionHandle handle) override;
		void Pause(GalExecutionHandle handle) override;
		void Resume(GalExecutionHandle handle) override;
		void PauseAll() override;
		void ResumeAll() override;
		void Tick(float deltaTime) override;

		/// 中文：**GalRuntimeCoordinator::ResetRuntime** 时调用；与 **PauseAll** 解耦，避免残留全局暂停。
		void Reset() noexcept;

	private:
		StoryScriptSystem* m_ScriptSystem = nullptr;
		bool m_GlobalPaused = false;
		GalExecutionHandle m_NextHandle = 1;
	};
}
