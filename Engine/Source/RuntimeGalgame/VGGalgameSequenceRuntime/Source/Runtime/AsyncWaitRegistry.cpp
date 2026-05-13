/*
 * AsyncWaitRegistry — 实现
 */

#include "Runtime/AsyncWaitRegistry.h"

#include <vector>

namespace VisionGal::GalGame
{
	WaitToken AsyncWaitRegistry::CreateWait(const std::string& reason)
	{
		const std::uint64_t id = m_NextTokenId.fetch_add(1ull, std::memory_order_acq_rel);
		m_ActiveWaits.insert(id);
		WaitToken t;
		t.TokenID = id;
		t.Reason = reason;
		return t;
	}

	bool AsyncWaitRegistry::IsResolved(const std::uint64_t token) const noexcept
	{
		if (token == 0ull)
			return true;
		return m_ActiveWaits.find(token) == m_ActiveWaits.end();
	}

	void AsyncWaitRegistry::Resolve(const std::uint64_t token) noexcept
	{
		if (token == 0ull)
			return;
		m_ActiveWaits.erase(token);
	}

	bool AsyncWaitRegistry::IsActive(const std::uint64_t token) const noexcept
	{
		return token != 0ull && m_ActiveWaits.find(token) != m_ActiveWaits.end();
	}

	std::vector<std::uint64_t> AsyncWaitRegistry::GetActiveTokensForSave() const
	{
		return std::vector<std::uint64_t>(m_ActiveWaits.begin(), m_ActiveWaits.end());
	}

	void AsyncWaitRegistry::RestoreNextTokenIdFromSave(const std::uint64_t nextExclusive) noexcept
	{
		std::uint64_t v = nextExclusive;
		if (v < 1ull)
			v = 1ull;
		m_NextTokenId.store(v, std::memory_order_release);
	}

	void AsyncWaitRegistry::ClearAllActive() noexcept
	{
		m_ActiveWaits.clear();
	}

	void AsyncWaitRegistry::RestoreFromSnapshot(const std::uint64_t nextExclusive, const std::vector<std::uint64_t>& activeIds) noexcept
	{
		RestoreNextTokenIdFromSave(nextExclusive);
		m_ActiveWaits.clear();
		for (const std::uint64_t id : activeIds)
		{
			if (id != 0ull)
				m_ActiveWaits.insert(id);
		}
	}
}
