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
	/// Value-type view of runtime execution state after a controller operation (no engine pointers).
	/// 控制器操作后的运行时执行状态值类型视图（不含引擎指针）。
	struct SequenceRuntimeSnapshot
	{
		uint32_t CurrentIndex = 0;
		bool HasValidDebugInfo = false;
		bool ReachedTarget = false;
		std::string LastError;
	};
}
