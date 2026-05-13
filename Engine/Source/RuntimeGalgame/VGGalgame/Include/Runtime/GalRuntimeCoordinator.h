/*
 * GalRuntimeCoordinator — 运行时协调器（Phase 8A / VGGalgame）
 *
 * 中文：将原先散落在 **GalGameEngine** / **GalRuntimeSessionHost** 中的「每帧 Tick 顺序」「主场景变更」
 * 「全量 Reset」「Shutdown 反序收尾」集中到单一类，作为后续 **RuntimeCoordinator**（文档命名）的宿主实现。
 *
 * 设计要点：
 * - **不**替代 **IGalRuntimeSession**；Session 仍负责 Start/Stop/Pause 与 **IExecutionScheduler** 持有。
 * - **Initialize 顺序**（8A-2）：在 **GalGameEngine::CreateSubsystem** 内以注释与代码块固定为
 *   Context →（Context 内 EventBus）→ 各业务 Systems → Script（StoryScriptSystem）→
 *   最后在 **Initialize** 末尾装配 **RuntimeSession / Execution**。
 * - **Shutdown**（8A-2 反向）：见 **Shutdown**，子系统若无独立 `Shutdown` 则仅做可安全重复调用的清理。
 */

#pragma once
#include "../../VGGalgameConfig.h"
#include "GalRuntimePhase.h"
#include <VGCore\Include\Core\Events.h>

namespace VisionGal::GalGame
{
	class GalGameEngine;
	class GalDefaultExecutionScheduler;

	class VG_GALGAME_API GalRuntimeCoordinator final
	{
	public:
		GalRuntimeCoordinator() = default;

		/// 中文：在 **GalGameEngine::Initialize** 入口尽早调用，绑定宿主指针。
		void Attach(GalGameEngine* host) noexcept;

		GalRuntimePhase GetPhase() const noexcept { return m_Phase; }

		/// 中文：**CreateSubsystem** 开始时置 **Initializing**。
		void BeginSubsystemConstruction() noexcept;

		/// 中文：**CreateSubsystem** 全部 Ref 创建并 **Initialise** 完成后调用（仍在 **Initialize** 前半段）。
		void EndSubsystemConstruction() noexcept;

		/// 中文：**Initialize** 末尾、**RuntimeSession::Start** 之前置 **Running**。
		void MarkHostRunning() noexcept;

		/// 中文：一帧逻辑体（原 **GalRuntimeSessionHost::Tick** 内三段顺序）。
		void TickFrame(float deltaTime, GalDefaultExecutionScheduler& scheduler);

		/// 中文：引擎事件 **MainSceneChanged** 的集中实现（原 **GalGameEngine::OnMainSceneChanged**）。
		void HandleMainSceneChanged(const EngineEvent& evt);

		/// 中文：全量清理（8A-4）：执行器/对话/场景/角色/UI/RuntimeState/延迟回调队列等。
		void ResetRuntime();

		/// 中文：占位钩子，供 Phase 8D 快照系统接入；当前不读写磁盘。
		void SaveRuntimeState();

		/// 中文：占位钩子，与 **SaveRuntimeState** 成对预留。
		void RestoreRuntimeState();

		/// 中文：**GalGameEngine** 析构或将来显式 **Dispose** 时调用；反序停止 Tick 与脚本。
		void Shutdown();

	private:
		void SetPhase(GalRuntimePhase phase) noexcept;

		GalGameEngine* m_Host = nullptr;
		GalRuntimePhase m_Phase = GalRuntimePhase::Uninitialized;
	};
}
