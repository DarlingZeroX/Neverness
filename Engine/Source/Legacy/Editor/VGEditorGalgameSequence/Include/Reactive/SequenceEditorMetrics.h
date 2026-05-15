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
	/// Lightweight counters for editor profiling (Phase 7).
	struct SequenceEditorMetrics
	{
		uint64_t PresentationTickCount = 0;
		uint64_t LastProjectionPassMicros = 0;
		uint64_t LastDerivedPassMicros = 0;
		uint64_t LastSearchRebuildMicros = 0;
	};
}
