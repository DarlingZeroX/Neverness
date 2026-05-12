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
#include <vector>

namespace VisionGal::Editor
{
	/// UI-facing overlay derived from execution snapshots (no engine pointers).
	struct SequenceRuntimeOverlayState
	{
		bool ShowExecutionLine = false;
		uint32_t HighlightIndex = 0;
		bool ReachedTarget = false;
		std::string LastError;
		std::vector<uint32_t> BreakpointIndices;
	};
}
