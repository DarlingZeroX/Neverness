/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <cstdint>
#include <string>

namespace VisionGal::Editor
{
	enum class SequenceRuntimeEventKind : uint8_t
	{
		Unknown = 0,
		Step,
		BreakpointHit,
		Continue_,
		Pause,
		OverlaySync,
		Custom
	};

	/// 结构化运行时事件一帧（为 scrub / replay / time-travel 预留）。
	struct SequenceRuntimeEventFrame
	{
		uint64_t sequenceNumber = 0;
		double timestampSeconds = 0.0;
		SequenceRuntimeEventKind kind = SequenceRuntimeEventKind::Unknown;
		unsigned entryIndex = 0;
		std::string payload;
	};
}
