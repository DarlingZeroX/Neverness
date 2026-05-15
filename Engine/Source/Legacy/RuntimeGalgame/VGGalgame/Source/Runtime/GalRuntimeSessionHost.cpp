#include "Runtime/GalRuntimeSessionHost.h"

#include "GalGameEngine.h"

namespace VisionGal::GalGame
{
	GalRuntimeSessionHost::GalRuntimeSessionHost(GalGameEngine* engine)
		: m_Engine(engine)
		, m_Scheduler(engine ? engine->m_StoryScriptSystem.get() : nullptr)
	{
	}

	void GalRuntimeSessionHost::Start()
	{
		m_Started = true;
		m_Paused = false;
	}

	void GalRuntimeSessionHost::Stop()
	{
		m_Started = false;
		m_Paused = false;
	}

	void GalRuntimeSessionHost::Pause()
	{
		m_Paused = true;
		m_Scheduler.PauseAll();
	}

	void GalRuntimeSessionHost::Resume()
	{
		m_Paused = false;
		m_Scheduler.ResumeAll();
	}

	void GalRuntimeSessionHost::Tick(float deltaTime)
	{
		if (!m_Started || m_Engine == nullptr || !m_Engine->m_IsEngineEnable)
			return;

		if (m_Paused)
			return;

		m_Engine->m_RuntimeCoordinator.TickFrame(deltaTime, m_Scheduler);
	}

	void GalRuntimeSessionHost::ResetSchedulerStateForHost() noexcept
	{
		m_Scheduler.Reset();
	}

	ISubsystemBus* GalRuntimeSessionHost::GetSubsystemBus() noexcept
	{
		return m_Engine ? m_Engine->GetSubsystemBus() : nullptr;
	}

	IExecutionScheduler* GalRuntimeSessionHost::GetExecutionScheduler() noexcept
	{
		return &m_Scheduler;
	}

	IGalGameContext* GalRuntimeSessionHost::GetRuntimeState() noexcept
	{
		return m_Engine ? m_Engine->GetContext() : nullptr;
	}

	IGalGameContext* GalRuntimeSessionHost::GetResourceContext() noexcept
	{
		return GetRuntimeState();
	}

	IRuntimeEventPipeline* GalRuntimeSessionHost::GetEventPipeline() noexcept
	{
		return &m_EventPipeline;
	}
}
