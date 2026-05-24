#pragma once

/**
 * @file NNAssetCache.h
 * @brief LRU 資產快取（按最後存取時間驅逐）。
 *
 * 當記憶體使用量超過預算時，自動驅逐最少使用的資產。
 * 使用 intrusive doubly-linked list 實現 O(1) 驅逐。
 */

#include <cstdint>
#include <functional>
#include <list>
#include <mutex>
#include <unordered_map>

#include "NNNativeEngineAPI/Include/EngineTypes.h"

namespace NN::Runtime::Asset
{

/**
 * @brief 快取條目。
 */
struct NNCacheEntry
{
	NNGuid       guid{};
	std::uint64_t memorySize{0};     /* 此資產佔用的記憶體（位元組） */
	std::uint64_t handle{0};         /* 對應的 NNAssetHandle 原始值 */
	bool         pinned{false};      /* 釘選：不被自動驅逐 */
};

/**
 * @brief LRU 資產快取。
 */
class NNAssetCache
{
public:
	/** @brief 驅逐回調：當資產被驅逐時呼叫，由 AssetManager 負責釋放。 */
	using EvictionCallback = std::function<void(NNGuid guid, std::uint64_t handle)>;

	explicit NNAssetCache(std::uint64_t memoryBudget = 512ull * 1024 * 1024) noexcept;

	/** @brief 設定記憶體預算（位元組）。 */
	void SetMemoryBudget(std::uint64_t bytes) noexcept;

	/** @brief 取得當前記憶體使用量。 */
	std::uint64_t GetCurrentUsage() const noexcept;

	/** @brief 取得記憶體預算。 */
	std::uint64_t GetBudget() const noexcept { return budget_; }

	/** @brief 取得快取條目數量。 */
	std::uint32_t GetEntryCount() const noexcept;

	/** @brief 新增或更新快取條目（觸發 LRU 標記）。 */
	void Touch(NNGuid guid, std::uint64_t memorySize, std::uint64_t handle);

	/** @brief 釘選資產（不被驅逐）。 */
	void Pin(NNGuid guid);

	/** @brief 解除釘選。 */
	void Unpin(NNGuid guid);

	/** @brief 移除指定資產。 */
	bool Remove(NNGuid guid);

	/** @brief 驅逐最少使用之資產直到釋放 requestedBytes。
	 *  @return 實際釋放的位元組數。 */
	std::uint64_t Evict(std::uint64_t requestedBytes, EvictionCallback callback);

	/** @brief 清除所有非釘選條目。 */
	void Clear(EvictionCallback callback);

	/** @brief 查詢資產是否在快取中。 */
	bool Contains(NNGuid guid) const;

private:
	using ListIterator = std::list<NNGuid>::iterator;

	std::uint64_t budget_;
	std::uint64_t currentUsage_{0};

	/* LRU 鏈表：front = 最近使用，back = 最久未使用 */
	std::list<NNGuid>           lruList_;

	/* GUID → (快取條目, LRU 鏈表迭代器) */
	struct CacheSlot
	{
		NNCacheEntry   entry;
		ListIterator   lruIter;
	};
	std::unordered_map<std::uint64_t, CacheSlot> cache_;

	mutable std::mutex mutex_;
};

} // namespace NN::Runtime::Asset
