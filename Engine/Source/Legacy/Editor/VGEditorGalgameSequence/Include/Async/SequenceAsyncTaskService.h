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
#include <mutex>
#include <thread>
#include <vector>

namespace VisionGal::Editor
{
	/// Detached worker jobs that deliver `std::function` completion on `PumpCompleted` (main / ImGui thread).
	class SequenceAsyncTaskService
	{
	public:
		~SequenceAsyncTaskService();

		void EnqueueValidation(
			std::function<std::vector<SequenceValidationIssue>()> produceOnWorker,
			std::function<void(std::vector<SequenceValidationIssue>)> mergeOnMainThread);

		void PumpCompleted();

		void DrainShutdown();

	private:
		std::mutex m_mutex;
		std::vector<std::function<void()>> m_pendingMain{};
		bool m_shuttingDown = false;
	};
}
