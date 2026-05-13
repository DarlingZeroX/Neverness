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
}
