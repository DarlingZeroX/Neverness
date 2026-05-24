#include "NNAssetHandle.h"

namespace NN::Runtime::Asset
{

std::uint64_t NNHandleTable::Allocate(void* data, std::uint64_t typeId)
{
	std::lock_guard<std::mutex> lock(mutex_);

	std::uint32_t index;
	if (!freeList_.empty())
	{
		index = freeList_.back();
		freeList_.pop_back();
	}
	else
	{
		index = static_cast<std::uint32_t>(slots_.size());
		slots_.emplace_back();
	}

	Slot& slot = slots_[index];
	slot.data = data;
	slot.typeId = typeId;
	slot.refCount.store(1, std::memory_order_relaxed);
	slot.alive = true;
	/* generation 在 slot 建構時為 0，每次重用時自增 */

	++allocatedCount_;
	return MakeHandle(index, slot.generation);
}

void NNHandleTable::Free(std::uint64_t handle)
{
	std::lock_guard<std::mutex> lock(mutex_);

	const std::uint32_t idx = HandleToIndex(handle);
	if (idx >= slots_.size())
		return;

	Slot& slot = slots_[idx];
	if (!slot.alive)
		return;
	if (HandleToGen(handle) != slot.generation)
		return;

	slot.data = nullptr;
	slot.alive = false;
	slot.refCount.store(0, std::memory_order_relaxed);
	++slot.generation; /* 使舊 Handle 失效 */

	freeList_.push_back(idx);
	--allocatedCount_;
}

void* NNHandleTable::Resolve(std::uint64_t handle) const
{
	if (handle == 0)
		return nullptr;

	std::lock_guard<std::mutex> lock(mutex_);

	const std::uint32_t idx = HandleToIndex(handle);
	if (idx >= slots_.size())
		return nullptr;

	const Slot& slot = slots_[idx];
	if (!slot.alive)
		return nullptr;
	if (HandleToGen(handle) != slot.generation)
		return nullptr;

	return slot.data;
}

std::uint64_t NNHandleTable::GetTypeId(std::uint64_t handle) const
{
	std::lock_guard<std::mutex> lock(mutex_);

	const std::uint32_t idx = HandleToIndex(handle);
	if (idx >= slots_.size() || !slots_[idx].alive)
		return 0;
	if (HandleToGen(handle) != slots_[idx].generation)
		return 0;

	return slots_[idx].typeId;
}

void NNHandleTable::AddRef(std::uint64_t handle)
{
	std::lock_guard<std::mutex> lock(mutex_);
	const std::uint32_t idx = HandleToIndex(handle);
	if (idx < slots_.size() && slots_[idx].alive && HandleToGen(handle) == slots_[idx].generation)
	{
		slots_[idx].refCount.fetch_add(1, std::memory_order_relaxed);
	}
}

bool NNHandleTable::Release(std::uint64_t handle)
{
	std::lock_guard<std::mutex> lock(mutex_);
	const std::uint32_t idx = HandleToIndex(handle);
	if (idx >= slots_.size() || !slots_[idx].alive)
		return false;
	if (HandleToGen(handle) != slots_[idx].generation)
		return false;

	const auto prev = slots_[idx].refCount.fetch_sub(1, std::memory_order_acq_rel);
	return prev == 1; /* 回傳 true 表示計數降至 0 */
}

std::uint32_t NNHandleTable::GetRefCount(std::uint64_t handle) const
{
	std::lock_guard<std::mutex> lock(mutex_);
	const std::uint32_t idx = HandleToIndex(handle);
	if (idx >= slots_.size() || !slots_[idx].alive)
		return 0;
	if (HandleToGen(handle) != slots_[idx].generation)
		return 0;

	return slots_[idx].refCount.load(std::memory_order_relaxed);
}

std::uint32_t NNHandleTable::GetAllocatedCount() const
{
	std::lock_guard<std::mutex> lock(mutex_);
	return allocatedCount_;
}

} // namespace NN::Runtime::Asset
