/*
 * GalRuntimeCoordinator 实现（Phase 8A / 8D / 8F 衔接）
 *
 * 中文：**ResetRuntime** 顺序设计目标：
 * 1) 停会话，避免 **Tick** 与清理并发；
 * 2) 中止全局 **TransitionManager** 队列，防止 Reset 后仍驱动已销毁层资源；
 * 3) 卸脚本执行管线（**不含** Context 状态 —— 由本类末尾单点写入）；
 * 4) 清对白 UI 数据面；
 * 5) **LayeredSceneSystem::ClearAll**（精灵/音频/视频/**GalCharacter**）；
 * 6) **ResourceSystem** 钩子（预留给 Actor 句柄 / 异步预载对齐）；
 * 7) UI 暂态、调度器 **Reset**；
 * 8) **GalGameRuntimeState** + **ArchiveDataContainer** 重建；
 * 9) 恢复 **m_IsEngineEnable**，使宿主在未切场景时仍可继续 Tick；
 * 10) 广播 **OnRuntimeLifecycleEvent(ResetCompleted)** 供调试器 / Editor 订阅。
 *
 * 与 **HandleMainSceneChanged** 的差异：主场景切换**不**中止转场队列（避免打断导演意图）、**不**强制 **m_IsEngineEnable=true**（仍由脚本加载结果决定）。
 */

#include "Runtime/GalRuntimeCoordinator.h"

#include "GalGameEngine.h"
#include "Runtime/GalDefaultExecutionScheduler.h"
#include "ScriptSystem/StoryScriptSystem.h"
#include "DialogueSystem/DialogueSystem.h"
#include "SceneSystem/LayeredSceneSystem.h"
#include "UISystem/GalUISystem.h"
#include "ResourceSystem.h"
#include "NNEngineLegacy/Include/Render/TransitionManager.h"
#include "NNEngineLegacy/Include/Scene/Scene.h"
#include "NNRuntimeCore/Include/Core/EventBus.h"
#include "NNEngineLegacy/Include/Engine/Manager.h"
#include "VGGalgameRuntimeCore/Include/ArchiveDataContainer.h"
#include "VGGalgameRuntimeCore/Include/GalGameContext.h"
#include "VGGalgameRuntimeCore/Include/GalGameEvent.h"
#include "VGGalgameRuntimeCore/Include/GalGameRuntimeState.h"
#include "VGGalgameRuntimeCore/Include/GalGameRuntimeStateSerializable.h"

#include <NNKernel/Include/File/nlohmann/json.hpp>

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

		/// 中文：与 **GalRuntimeSessionHost::Tick** 历史顺序严格一致；**ExecutionScheduler** 扩展队列后在此收敛。
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

		/// 中文：先停会话，避免 **Tick** 与 **Reset** 交错（尤其 **StoryScriptSystem::Update**）。
		if (m_Host->m_RuntimeSession)
			m_Host->m_RuntimeSession->Stop();

		/// 中文：丢弃未完成的屏幕转场，避免下一帧 **LayerTransition** 仍引用已 **ClearAll** 的渲染目标。
		VisionGal::TransitionManager::GetInstance()->AbortAllTransitions();

		if (m_Host->m_StoryScriptSystem)
			m_Host->m_StoryScriptSystem->ResetExecutionPipeline();

		if (m_Host->m_DialogueSystem)
			m_Host->m_DialogueSystem->Clear();

		if (m_Host->m_LayeredSceneManager)
			m_Host->m_LayeredSceneManager->ClearAll();

		if (m_Host->m_ResourceSystem)
			m_Host->m_ResourceSystem->NotifyRuntimeReset();

		if (m_Host->m_GalGameUISystem)
			m_Host->m_GalGameUISystem->ResetTransientUIState();

		/// 中文：**GalDefaultExecutionScheduler** 重置暂停旗与句柄种子，保证 **PauseAll** 后 **Reset** 可再次 **ResumeAll**。
		if (m_Host->m_RuntimeSession)
			m_Host->m_RuntimeSession->ResetSchedulerStateForHost();

		if (m_Host->m_GalGameContext)
		{
			m_Host->m_GalGameContext->runtimeState = GalGameRuntimeState{};
			m_Host->m_GalGameContext->archiveData = MakeRef<ArchiveDataContainer>();
		}

		/// 中文：与旧版 **GalGameEngine::Reset** 语义对齐 —— 全量清理后默认恢复 Tick；若需保持暂停由上层再调 **Stop**。
		m_Host->m_IsEngineEnable = true;

		if (m_Host->m_GalGameContext)
		{
			GalRuntimeLifecycleEvent life{};
			life.kind = GalRuntimeLifecycleKind::ResetCompleted;
			m_Host->m_GalGameContext->engineEventBus.OnRuntimeLifecycleEvent.Invoke(life);
		}

		/// 中文：不自动 **RuntimeSession::Start**；由宿主 **Initialize** / 场景加载策略决定是否重新 **Start**。
		SetPhase(GalRuntimePhase::Running);
	}

	void GalRuntimeCoordinator::SaveRuntimeState()
	{
		if (!m_Host || !m_Host->m_GalGameContext)
			return;

		nlohmann::json j;
		GalGameRuntimeStateSerializable ser(&m_Host->m_GalGameContext->runtimeState);
		ser.SaveToJson(j);
		m_LastRuntimeStateJsonBlob = j.dump();
	}

	void GalRuntimeCoordinator::RestoreRuntimeState()
	{
		if (!m_Host || !m_Host->m_GalGameContext || m_LastRuntimeStateJsonBlob.empty())
			return;

		const nlohmann::json j = nlohmann::json::parse(m_LastRuntimeStateJsonBlob, nullptr, false);
		if (j.is_discarded() || !j.is_object())
			return;

		GalGameRuntimeStateSerializable ser(&m_Host->m_GalGameContext->runtimeState);
		(void)ser.LoadFromJson(j);
	}

	void GalRuntimeCoordinator::Shutdown()
	{
		if (!m_Host)
			return;

		SetPhase(GalRuntimePhase::ShuttingDown);

		if (m_Host->m_RuntimeSession)
			m_Host->m_RuntimeSession->Stop();

		VisionGal::TransitionManager::GetInstance()->AbortAllTransitions();

		/// 中文：反序与 **Initialize** 相对：**Execution（脚本）** → **UI** → **Resource** 钩子 → **Archive**（无独立 Shutdown）→ **Render**（无）→ **Dialogue** → **Scene**。
		if (m_Host->m_StoryScriptSystem)
			m_Host->m_StoryScriptSystem->ResetExecutionPipeline();

		if (m_Host->m_GalGameUISystem)
			m_Host->m_GalGameUISystem->ResetTransientUIState();

		if (m_Host->m_ResourceSystem)
			m_Host->m_ResourceSystem->NotifyRuntimeReset();

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
