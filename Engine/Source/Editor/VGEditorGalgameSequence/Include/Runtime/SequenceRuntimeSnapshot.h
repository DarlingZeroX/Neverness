/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <cstdint>
#include <limits>
#include <string>

namespace VisionGal::Editor
{
	/// Value-type view of runtime execution state after a controller operation (no engine pointers).
	struct SequenceRuntimeSnapshot
	{
		uint32_t CurrentIndex = 0;
		/// 单步前索引（`StepOnce` 填充；未步进时为 `UINT_MAX`）。
		uint32_t PreviousIndex = (std::numeric_limits<uint32_t>::max)();
		std::string CurrentComponentType;
		bool Waiting = false;
		bool HasValidDebugInfo = false;
		bool ReachedTarget = false;
		bool BreakpointHit = false;
		bool StoppedOnPause = false;
		bool StalledNoAdvance = false;
		std::string LastError;
	};
}
