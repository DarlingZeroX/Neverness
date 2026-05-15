/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#include "Async/SequenceAsyncTaskService.h"

namespace VisionGal::Editor
{
	SequenceAsyncTaskService::~SequenceAsyncTaskService()
	{
		DrainShutdown();
	}

	void SequenceAsyncTaskService::EnqueueValidation(
		std::function<std::vector<SequenceValidationIssue>()> produceOnWorker,
		std::function<void(std::vector<SequenceValidationIssue>)> mergeOnMainThread)
	{
		if (m_shuttingDown)
			return;

		std::thread(
			[this,
			 produce = std::move(produceOnWorker),
			 merge = std::move(mergeOnMainThread)]() mutable
			{
				std::vector<SequenceValidationIssue> issues;
				if (produce)
					issues = produce();

				std::function<void()> thunk = [merge = std::move(merge), iss = std::move(issues)]() mutable
				{
					if (merge)
						merge(std::move(iss));
				};

				std::lock_guard<std::mutex> lock(m_mutex);
				if (!m_shuttingDown)
					m_pendingMain.push_back(std::move(thunk));
			})
			.detach();
	}

	void SequenceAsyncTaskService::PumpCompleted()
	{
		std::vector<std::function<void()>> batch;
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			batch.swap(m_pendingMain);
		}
		for (auto& job : batch)
		{
			if (job)
				job();
		}
	}

	void SequenceAsyncTaskService::DrainShutdown()
	{
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_shuttingDown = true;
		}
		PumpCompleted();
	}
}
