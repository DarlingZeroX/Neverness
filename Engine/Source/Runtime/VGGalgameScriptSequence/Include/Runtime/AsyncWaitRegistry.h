/*
 * AsyncWaitRegistry — WaitToken 分配与解析登记处
 *
 * 语义约定：
 * - CreateWait：分配单调递增 TokenID，加入「未解析」集合；
 * - Resolve：从集合移除；之后 IsResolved 为 true；
 * - IsResolved：从未加入或已 Resolve 的 id 均视为 true（避免存档后外部丢失句柄导致永久死等）。
 *
 * 线程：假定仅在游戏线程访问，无互斥。
 */
#pragma once

#include "WaitToken.h"

#include <atomic>
#include <cstdint>
#include <string>
#include <unordered_set>
#include <vector>

#include "../../GSSExport.h"

namespace VisionGal::GalGame
{
	class VG_GSS_API AsyncWaitRegistry
	{
	public:
		WaitToken CreateWait(const std::string& reason);

		[[nodiscard]] bool IsResolved(std::uint64_t token) const noexcept;

		void Resolve(std::uint64_t token) noexcept;

		[[nodiscard]] bool IsActive(std::uint64_t token) const noexcept;

		/// 存档：列出仍为 Active 的 token（顺序未定义；测试若需确定性可自行排序 JSON）。
		[[nodiscard]] std::vector<std::uint64_t> GetActiveTokensForSave() const;

		/// 存档：下一将分配的 TokenID（等于已分配最大 id + 1，或 1）。
		[[nodiscard]] std::uint64_t GetNextTokenIdForSave() const noexcept { return m_NextTokenId.load(std::memory_order_acquire); }

		/// 读档：恢复计数器，避免新 token 与存档冲突。
		void RestoreNextTokenIdFromSave(std::uint64_t nextExclusive) noexcept;

		void ClearAllActive() noexcept;

		/// 读档：恢复未解析 token 集合与下一分配序号（由 SequenceStateSerializer 调用）。
		void RestoreFromSnapshot(std::uint64_t nextExclusive, const std::vector<std::uint64_t>& activeIds) noexcept;

	private:
		std::atomic<std::uint64_t> m_NextTokenId{1};
		std::unordered_set<std::uint64_t> m_ActiveWaits;
	};
}
