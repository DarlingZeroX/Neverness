#include "Runtime/GalDefaultExecutionScheduler.h"
#include "ScriptSystem/StoryScriptSystem.h"

namespace VisionGal::GalGame
{
	GalDefaultExecutionScheduler::GalDefaultExecutionScheduler(StoryScriptSystem* scriptSystem) noexcept
		: m_ScriptSystem(scriptSystem)
	{
	}

	GalExecutionHandle GalDefaultExecutionScheduler::SubmitWait(float /*seconds*/)
	{
		/// 中文：占位；Wait 逻辑仍在 StoryScriptSystem::Wait / UpdateWaitState，Phase 8.6 前后迁入调度队列。
		return ++m_NextHandle;
	}

	GalExecutionHandle GalDefaultExecutionScheduler::SubmitYield(const GalYieldInstruction& instruction)
	{
		if (m_ScriptSystem == nullptr)
			return 0;

		switch (instruction.kind)
		{
		case GalYieldKind::WaitSeconds:
			m_ScriptSystem->Wait(instruction.seconds);
			break;
		case GalYieldKind::WaitDialogueContinue:
			/// 中文：对白行推进仍由 **IStoryScriptExecutor::ContinueDialogue** / UI 事件触发；此处不启动 **StoryScriptSystem::Wait** 计时。
			break;
		default:
			/// 中文：**Signal*** 系列为 **ExecutionSignal** 预留（Choice / Input / Animation 等）；当前仅分配句柄，避免执行器私自 **Wait**。
			break;
		}

		return ++m_NextHandle;
	}

	void GalDefaultExecutionScheduler::Cancel(GalExecutionHandle /*handle*/)
	{
	}

	void GalDefaultExecutionScheduler::Pause(GalExecutionHandle /*handle*/)
	{
	}

	void GalDefaultExecutionScheduler::Resume(GalExecutionHandle /*handle*/)
	{
	}

	void GalDefaultExecutionScheduler::PauseAll()
	{
		m_GlobalPaused = true;
	}

	void GalDefaultExecutionScheduler::ResumeAll()
	{
		m_GlobalPaused = false;
	}

	void GalDefaultExecutionScheduler::Tick(float /*deltaTime*/)
	{
		if (m_GlobalPaused || m_ScriptSystem == nullptr)
			return;

		m_ScriptSystem->Update();
	}

	void GalDefaultExecutionScheduler::Reset() noexcept
	{
		m_GlobalPaused = false;
		m_NextHandle = 1;
	}
}
