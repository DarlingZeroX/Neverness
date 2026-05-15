/*
 * IStoryExecutionAdapter — Sequence / Lua 与执行实例之间的薄桥（Phase 8：Contract）
 *
 * 仅转发到 IStoryExecutionInstance，避免上层模块直接依赖具体 Sequence 内核类型。
 */

#pragma once

namespace VisionGal::GalGame
{
	struct IStoryExecutionInstance;
	struct ISubsystemBus;

	/** 中文：不拥有 IStoryExecutionInstance 生命周期；由宿主工厂/执行器持有。 */
	struct IStoryExecutionAdapter
	{
		virtual ~IStoryExecutionAdapter() = default;

		[[nodiscard]] virtual IStoryExecutionInstance* GetExecution() const noexcept = 0;

		virtual void Tick(float deltaTime, ISubsystemBus* bus) = 0;
		virtual void Continue(ISubsystemBus* bus) = 0;
	};
}
