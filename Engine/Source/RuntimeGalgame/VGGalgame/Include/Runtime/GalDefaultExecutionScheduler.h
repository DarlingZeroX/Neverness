/*
 * GalDefaultExecutionScheduler — IExecutionScheduler 默认实现（Phase 8.3 / VGGalgame）
 *
 * 中文：当前将「剧情脚本系统 Update」纳入调度器 Tick，与 GalGameEngine::OnUpdate 中顺序对齐；
 * Wait 任务仍由 StoryScriptSystem 内部维护，SubmitWait 仅占位返回句柄，后续迁入统一队列。
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

	private:
		StoryScriptSystem* m_ScriptSystem = nullptr;
		bool m_GlobalPaused = false;
		GalExecutionHandle m_NextHandle = 1;
	};
}
