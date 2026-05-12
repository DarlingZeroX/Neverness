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
	/// 运行时属性变化一帧（文本化取值便于 Dock / 日志；与引擎强类型解耦）。
	struct SequenceRuntimePropertySnapshot
	{
		unsigned EntryIndex = 0;
		std::string PropertyPath;
		std::string OldValueText;
		std::string NewValueText;
		double TimestampSeconds = 0.0;
	};
}
