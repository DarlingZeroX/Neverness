/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#pragma once

#include <string>

namespace VisionGal::Editor
{
	struct SequenceRuntimeSnapshot;

	/// Phase 9：`SequenceRuntimeKernel::Step` 等操作的归一化结果。
	struct SequenceRuntimeStepResult
	{
		bool Ok = false;
		std::string Error;
		SequenceRuntimeSnapshot* Snapshot = nullptr;
	};
}
