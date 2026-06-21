#include "NNAssetCache.h"

namespace NN::Runtime::Asset
{

NNAssetCache::NNAssetCache(std::uint64_t memoryBudget) noexcept
	: budget_(memoryBudget)
{
}

void NNAssetCache::SetMemoryBudget(std::uint64_t bytes) noexcept
{
	std::lock_guard<std::mutex> lock(mutex_);
	budget_ = bytes;
}

std::uint64_t NNAssetCache::GetCurrentUsage() const noexcept
{
	std::lock_guard<std::mutex> lock(mutex_);
	return currentUsage_;
}

std::uint32_t NNAssetCache::GetEntryCount() const noexcept
{
	std::lock_guard<std::mutex> lock(mutex_);
	return static_cast<std::uint32_t>(cache_.size());
}

void NNAssetCache::Touch(NNGuid guid, std::uint64_t memorySize, std::uint64_t handle)
{
	std::lock_guard<std::mutex> lock(mutex_);

	const auto it = cache_.find(guid.low);
	if (it != cache_.end())
	{
		/* 已存在：移至 LRU 鏈表前端 */
		lruList_.erase(it->second.lruIter);
		lruList_.push_front(guid);
		it->second.lruIter = lruList_.begin();
		it->second.entry.memorySize = memorySize;
		it->second.entry.handle = handle;
	}
	else
	{
		/* 新增 */
		lruList_.push_front(guid);
		NNCacheEntry entry{};
		entry.guid = guid;
		entry.memorySize = memorySize;
		entry.handle = handle;
		CacheSlot slot{};
		slot.entry = entry;
		slot.lruIter = lruList_.begin();
		cache_[guid.low] = std::move(slot);
		currentUsage_ += memorySize;
	}
}

void NNAssetCache::Pin(NNGuid guid)
{
	std::lock_guard<std::mutex> lock(mutex_);
	const auto it = cache_.find(guid.low);
	if (it != cache_.end())
		it->second.entry.pinned = true;
}

void NNAssetCache::Unpin(NNGuid guid)
{
	std::lock_guard<std::mutex> lock(mutex_);
	const auto it = cache_.find(guid.low);
	if (it != cache_.end())
		it->second.entry.pinned = false;
}

bool NNAssetCache::Remove(NNGuid guid)
{
	std::lock_guard<std::mutex> lock(mutex_);
	const auto it = cache_.find(guid.low);
	if (it == cache_.end())
		return false;

	currentUsage_ -= it->second.entry.memorySize;
	lruList_.erase(it->second.lruIter);
	cache_.erase(it);
	return true;
}

std::uint64_t NNAssetCache::Evict(std::uint64_t requestedBytes, EvictionCallback callback)
{
	std::lock_guard<std::mutex> lock(mutex_);

	std::uint64_t freed = 0;

	/* 從 LRU 鏈表尾端開始驅逐（最久未使用） */
	while (!lruList_.empty() && freed < requestedBytes)
	{
		/* 從尾端找一個非釘選的條目 */
		auto it = lruList_.end();
		bool found = false;
		while (it != lruList_.begin())
		{
			--it;
			const auto cit = cache_.find(it->low);
			if (cit != cache_.end() && !cit->second.entry.pinned)
			{
				found = true;
				break;
			}
		}

		if (!found)
			break; /* 所有條目都被釘選 */

		const NNGuid evictGuid = *it;
		const auto cit = cache_.find(evictGuid.low);
		if (cit == cache_.end())
			continue;

		const NNCacheEntry& entry = cit->second.entry;
		freed += entry.memorySize;

		/* 通知外部釋放 */
		if (callback)
			callback(evictGuid, entry.handle);

		/* 從快取移除 */
		currentUsage_ -= entry.memorySize;
		lruList_.erase(cit->second.lruIter);
		cache_.erase(cit);
	}

	return freed;
}

void NNAssetCache::Clear(EvictionCallback callback)
{
	std::lock_guard<std::mutex> lock(mutex_);

	/* 先通知非釘選條目 */
	if (callback)
	{
		for (auto& [key, slot] : cache_)
		{
			if (!slot.entry.pinned)
				callback(slot.entry.guid, slot.entry.handle);
		}
	}

	/* 移除非釘選條目 */
	auto it = cache_.begin();
	while (it != cache_.end())
	{
		if (!it->second.entry.pinned)
		{
			currentUsage_ -= it->second.entry.memorySize;
			lruList_.erase(it->second.lruIter);
			it = cache_.erase(it);
		}
		else
		{
			++it;
		}
	}
}

bool NNAssetCache::Contains(NNGuid guid) const
{
	std::lock_guard<std::mutex> lock(mutex_);
	return cache_.find(guid.low) != cache_.end();
}

} // namespace NN::Runtime::Asset
