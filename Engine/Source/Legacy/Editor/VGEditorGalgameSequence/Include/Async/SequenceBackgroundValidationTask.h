/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include "Validation/SequenceValidationIssue.h"

#include <functional>
#include <vector>

namespace VisionGal::Editor
{
	class SequenceAsyncTaskService;

	/// Schedules `RunAll`-style work off-thread; merge callback runs on `PumpCompleted`.
	struct SequenceBackgroundValidationTask
	{
		static void Schedule(
			SequenceAsyncTaskService& async,
			std::function<std::vector<SequenceValidationIssue>()> produceOnWorker,
			std::function<void(std::vector<SequenceValidationIssue>)> mergeOnMainThread)
		{
			async.EnqueueValidation(std::move(produceOnWorker), std::move(mergeOnMainThread));
		}
	};
}
