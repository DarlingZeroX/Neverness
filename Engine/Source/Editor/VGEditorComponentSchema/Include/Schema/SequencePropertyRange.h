/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <optional>

namespace VisionGal::Editor
{
	/// 数值滑条与校验共用的 inclusive 范围（未设置表示无界）。
	struct SequencePropertyRange
	{
		std::optional<double> Min;
		std::optional<double> Max;
	};
}
