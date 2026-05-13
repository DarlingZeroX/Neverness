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

		/// 中文：Tick 管线顺序（Phase 8 文档化，后续与 IRuntimeScheduler 对齐）：
		/// 1) LayeredSceneManager::OnUpdate — 场景与分层对象；
		/// 2) DialogueSystem::Update — 打字机 / 自动播放 / 快进节拍；
		/// 3) IExecutionScheduler::Tick — 剧情脚本与将来 Sequence / Lua 统一队列。
		m_Engine->m_LayeredSceneManager->OnUpdate();
		m_Engine->m_DialogueSystem->Update();
		m_Scheduler.Tick(deltaTime);
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
