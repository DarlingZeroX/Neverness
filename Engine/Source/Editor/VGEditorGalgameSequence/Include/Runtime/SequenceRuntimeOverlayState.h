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
	/// UI-facing overlay derived from execution snapshots (no engine pointers).
	/// 由执行快照派生的 UI 叠加层（无引擎指针）。
	struct SequenceRuntimeOverlayState
	{
		bool ShowExecutionLine = false;
		uint32_t HighlightIndex = 0;
		bool ReachedTarget = false;
		std::string LastError;
	};
}
