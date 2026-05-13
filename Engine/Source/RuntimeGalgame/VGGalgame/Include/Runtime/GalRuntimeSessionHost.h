/*
 * GalRuntimeSessionHost — IGalRuntimeSession 宿主实现（Phase 8.2 / VGGalgame）
 *
 * 中文：由 GalGameEngine 在 Initialize 末尾创建；**Tick** 委托 **GalRuntimeCoordinator::TickFrame**，
 * 将会话级节拍与 **IExecutionScheduler** 持有职责保留在本类，与 Phase 8A 生命周期协调器解耦。
 */

#pragma once

#include "VGGalgameContract/Interface/IGalRuntimeSession.h"
#include "VGGalgameContract/Interface/IRuntimeEventPipeline.h"
#include "Runtime/GalDefaultExecutionScheduler.h"
#include <memory>

namespace VisionGal::GalGame
{
	class GalGameEngine;

	/// 中文：空事件管线占位，避免 GetEventPipeline 返回悬空；后续替换为真管线实现。
	struct VG_GALGAME_API GalNoopRuntimeEventPipeline final : public IRuntimeEventPipeline
	{
	};

	struct VG_GALGAME_API GalRuntimeSessionHost final : public IGalRuntimeSession
	{
		explicit GalRuntimeSessionHost(GalGameEngine* engine);

		void Start() override;
		void Stop() override;
		void Pause() override;
		void Resume() override;
		void Tick(float deltaTime) override;

		/// 中文：由 **GalRuntimeCoordinator::ResetRuntime** 调用，重置调度器上的暂停与句柄计数。
		void ResetSchedulerStateForHost() noexcept;

		ISubsystemBus* GetSubsystemBus() noexcept override;
		IExecutionScheduler* GetExecutionScheduler() noexcept override;
		IGalGameContext* GetRuntimeState() noexcept override;
		IGalGameContext* GetResourceContext() noexcept override;
		IRuntimeEventPipeline* GetEventPipeline() noexcept override;

	private:
		GalGameEngine* m_Engine = nullptr;
		GalDefaultExecutionScheduler m_Scheduler;
		GalNoopRuntimeEventPipeline m_EventPipeline;
		bool m_Started = false;
		bool m_Paused = false;
	};
}
