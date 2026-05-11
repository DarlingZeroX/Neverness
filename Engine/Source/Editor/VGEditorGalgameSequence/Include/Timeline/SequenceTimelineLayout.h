/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <cstddef>

namespace VisionGal::Editor
{
	/// Fixed row geometry for Timeline v1 (no zoom, no multi-track).
	/// 时间轴 v1 固定行几何（无缩放、无多轨道）。
	struct SequenceTimelineLayout
	{
		float RowHeight = 28.f;
		float IndexColumnWidth = 36.f;

		[[nodiscard]] int VisibleIndexFromLocalY(float localY) const
		{
			if (RowHeight <= 0.f)
				return 0;
			const int row = static_cast<int>(localY / RowHeight);
			return row < 0 ? 0 : row;
		}
	};
}
