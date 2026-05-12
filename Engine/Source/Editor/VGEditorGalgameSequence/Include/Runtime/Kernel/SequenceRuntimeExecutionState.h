/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#pragma once

#include <cstdint>

namespace VisionGal::Editor
{
	/// Phase 9：内核视角下的执行阶段（与 `SequenceDebuggerSessionState` 互补；供扩展/遥测使用）。
	enum class SequenceRuntimeExecutionState : uint8_t
	{
		Idle = 0,
		Running,
		Paused,
		BreakpointHit,
		Finished,
		Error,
	};
}
