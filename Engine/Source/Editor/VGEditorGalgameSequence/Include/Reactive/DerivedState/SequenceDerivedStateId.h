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
	/// 与 `ReactiveCore::DerivedStateId` 数值对齐的业务派生状态 ID（Phase 8）。
	enum class SequenceDerivedStateId : uint32_t
	{
		Validation = 1,
		RuntimeOverlay = 2,
		SearchResults = 3,
	};
}
