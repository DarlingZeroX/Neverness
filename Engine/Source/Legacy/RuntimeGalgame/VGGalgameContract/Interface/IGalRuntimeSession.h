/*
 * IGalRuntimeSession — Gal 运行时会话（Phase 8.2 Contract）
 *
 * 中文：统一 Scene / UI / Script / Resource / Scheduler 的生命周期边界，取代分散在 GalGameEngine
 * 与 MainSceneChanged 回调中的隐式顺序；Start/Stop 与引擎 Initialize/Reset 对齐。
 *
 * GetRuntimeState / GetResourceContext：当前均映射到同一 IGalGameContext（GalGameContext），
 * 后续将「可序列化运行态」与「执行栈 / 等待点」拆到独立 **RuntimeSession** 缓冲（见 Phase 8D 文档）。
 */

#pragma once

namespace VisionGal::GalGame
{
	struct ISubsystemBus;
	struct IExecutionScheduler;
	struct IGalGameContext;
	struct IRuntimeEventPipeline;

	struct IGalRuntimeSession
	{
		virtual ~IGalRuntimeSession() = default;

		virtual void Start() = 0;
		virtual void Stop() = 0;
		virtual void Pause() = 0;
		virtual void Resume() = 0;

		virtual void Tick(float deltaTime) = 0;

		virtual ISubsystemBus* GetSubsystemBus() noexcept = 0;
		virtual IExecutionScheduler* GetExecutionScheduler() noexcept = 0;

		/// 中文：聚合运行时状态（对白行号、模式位等），实现类型为 GalGameContext。
		virtual IGalGameContext* GetRuntimeState() noexcept = 0;

		/// 中文：资源与变量容器视图；当前与 GetRuntimeState 返回同一上下文指针。
		virtual IGalGameContext* GetResourceContext() noexcept = 0;

		/// 中文：事件管线；未接入时可返回 nullptr。
		virtual IRuntimeEventPipeline* GetEventPipeline() noexcept = 0;
	};
}
