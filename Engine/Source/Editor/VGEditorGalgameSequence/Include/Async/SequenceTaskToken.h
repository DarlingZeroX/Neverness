/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <atomic>
#include <cstdint>

namespace VisionGal::Editor
{
	/// Cancellation / generation guard for async editor tasks (Phase 6).
	class SequenceTaskToken
	{
	public:
		SequenceTaskToken()
			: m_id(s_nextId.fetch_add(1, std::memory_order_relaxed))
		{
		}

		void Cancel() { m_cancelled.store(true, std::memory_order_release); }
		[[nodiscard]] bool IsCancelled() const { return m_cancelled.load(std::memory_order_acquire); }
		[[nodiscard]] std::uint64_t Id() const { return m_id; }

	private:
		static std::atomic<std::uint64_t> s_nextId;
		std::uint64_t m_id = 0;
		std::atomic<bool> m_cancelled{false};
	};
}
