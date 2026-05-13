/*
 * GalRuntimeCoordinator 实现（Phase 8A）
 */

#include "Runtime/GalRuntimeCoordinator.h"

#include "GalGameEngine.h"
#include "Runtime/GalDefaultExecutionScheduler.h"
#include "ScriptSystem/StoryScriptSystem.h"
#include "DialogueSystem/DialogueSystem.h"
#include "SceneSystem/LayeredSceneSystem.h"
#include "UISystem/GalUISystem.h"
#include "VGEngine/Include/Scene/Scene.h"
#include "VGCore/Include/Core/EventBus.h"
#include "VGEngine/Include/Engine/Manager.h"
#include "VGGalgameRuntimeCore/Include/ArchiveDataContainer.h"
#include "VGGalgameRuntimeCore/Include/GalGameContext.h"
#include "VGGalgameRuntimeCore/Include/GalGameRuntimeState.h"

namespace VisionGal::GalGame
{
	void GalRuntimeCoordinator::Attach(GalGameEngine* host) noexcept
	{
		m_Host = host;
	}

	void GalRuntimeCoordinator::SetPhase(GalRuntimePhase phase) noexcept
	{
		m_Phase = phase;
	}

	void GalRuntimeCoordinator::BeginSubsystemConstruction() noexcept
	{
		SetPhase(GalRuntimePhase::Initializing);
	}

	void GalRuntimeCoordinator::EndSubsystemConstruction() noexcept
	{
		/// 中文：子系统 Ref 已创建；**RuntimeSession** 仍在 **Initialize** 后半段创建，阶段保持 **Initializing**。
	}

	void GalRuntimeCoordinator::MarkHostRunning() noexcept
	{
		SetPhase(GalRuntimePhase::Running);
	}

	void GalRuntimeCoordinator::TickFrame(float deltaTime, GalDefaultExecutionScheduler& scheduler)
	{
		if (!m_Host || m_Phase != GalRuntimePhase::Running)
			return;

		if (!m_Host->m_IsEngineEnable)
			return;

		/// 中文：与 **GalRuntimeSessionHost::Tick** 历史顺序严格一致，后续迁入 **ExecutionScheduler** 统一队列时再收敛注释。
		m_Host->m_LayeredSceneManager->OnUpdate();
		m_Host->m_DialogueSystem->Update();
		scheduler.Tick(deltaTime);
	}

	void GalRuntimeCoordinator::HandleMainSceneChanged(const EngineEvent& evt)
	{
		if (!m_Host)
			return;

		if (m_Phase == GalRuntimePhase::ShuttingDown)
			return;

		SetPhase(GalRuntimePhase::Transitioning);

		m_Host->m_LayeredSceneManager->ClearAll();

		m_Host->m_Scene = dynamic_cast<Scene*>(evt.Scene);
		m_Host->m_RenderPipeline->SetScene(m_Host->m_Scene);
		m_Host->m_DialogueSystem->Clear();

		if (GetSceneManager()->IsPlayMode())
			m_Host->m_IsEngineEnable = m_Host->m_StoryScriptSystem->LoadSceneStoryScriptOnUpdate(evt.Scene);

		SetPhase(GalRuntimePhase::Running);
	}

	void GalRuntimeCoordinator::ResetRuntime()
	{
		if (!m_Host)
			return;

		SetPhase(GalRuntimePhase::Resetting);

		/// 中文：先停会话，避免 **Tick** 与 **Reset** 并发。
		if (m_Host->m_RuntimeSession)
			m_Host->m_RuntimeSession->Stop();

		if (m_Host->m_StoryScriptSystem)
			m_Host->m_StoryScriptSystem->ResetExecutionPipeline();

		if (m_Host->m_DialogueSystem)
			m_Host->m_DialogueSystem->Clear();

		if (m_Host->m_LayeredSceneManager)
			m_Host->m_LayeredSceneManager->ClearAll();

		if (m_Host->m_GalGameUISystem)
			m_Host->m_GalGameUISystem->ResetTransientUIState();

		/// 中文：**GalDefaultExecutionScheduler** 无独立句柄表时仍重置暂停旗与句柄种子，保证 Reset 后可重复 **PauseAll/ResumeAll** 语义。
		if (m_Host->m_RuntimeSession)
			m_Host->m_RuntimeSession->ResetSchedulerStateForHost();

		if (m_Host->m_GalGameContext)
		{
			m_Host->m_GalGameContext->runtimeState = GalGameRuntimeState{};
			m_Host->m_GalGameContext->archiveData = MakeRef<ArchiveDataContainer>();
		}

		/// 中文：与旧版 **Reset** 一致，不自动 **Start**；由上层重新 **Initialize** 场景或会话策略决定。
		SetPhase(GalRuntimePhase::Running);
	}

	void GalRuntimeCoordinator::SaveRuntimeState()
	{
		/// 中文：Phase 8D **State Snapshot** 接入点；当前仅占位，避免调用方无处挂载。
		(void)m_Host;
	}

	void GalRuntimeCoordinator::RestoreRuntimeState()
	{
		(void)m_Host;
	}

	void GalRuntimeCoordinator::Shutdown()
	{
		if (!m_Host)
			return;

		SetPhase(GalRuntimePhase::ShuttingDown);

		if (m_Host->m_RuntimeSession)
			m_Host->m_RuntimeSession->Stop();

		/// 中文：反序与 **Initialize** 相对：**Execution（脚本）** → **UI** → **Resource**（无独立 Shutdown）→ **Archive** → **Render** → **Dialogue** → **Scene**。
		if (m_Host->m_StoryScriptSystem)
			m_Host->m_StoryScriptSystem->ResetExecutionPipeline();

		if (m_Host->m_GalGameUISystem)
			m_Host->m_GalGameUISystem->ResetTransientUIState();

		if (m_Host->m_DialogueSystem)
			m_Host->m_DialogueSystem->Clear();

		if (m_Host->m_LayeredSceneManager)
			m_Host->m_LayeredSceneManager->ClearAll();

		if (m_Host->m_GalGameContext)
		{
			m_Host->m_GalGameContext->runtimeState = GalGameRuntimeState{};
			m_Host->m_GalGameContext->archiveData = MakeRef<ArchiveDataContainer>();
		}

		SetPhase(GalRuntimePhase::Uninitialized);
		m_Host = nullptr;
	}
}
