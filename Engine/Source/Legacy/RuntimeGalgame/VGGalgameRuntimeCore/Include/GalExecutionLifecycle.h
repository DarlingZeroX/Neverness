/*
 * ExecutionLifecycle — 与 Sequence ESS 状态映射的抽象生命周期（Phase 7.4）
 */

#pragma once

#include <cstdint>

namespace VisionGal::GalGame
{
	/** 中文：跨 Lua / Sequence / Runtime 的统一语义；与 ESSSequenceExecutorState 非一一对应，见文档映射表。 */
	enum class ExecutionLifecycle : std::uint8_t
	{
		Created = 0,
		Running,
		Waiting,
		Stopped,
		Finished
	};
}
