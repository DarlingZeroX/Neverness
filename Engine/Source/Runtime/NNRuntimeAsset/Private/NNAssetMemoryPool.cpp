/**
 * @file NNAssetMemoryPool.cpp
 * @brief 线程安全的块内存池实现（Phase 8）。
 */

#include "NNAssetMemoryPool.h"

#include <cstdlib>
#include <cstring>

namespace NN::Runtime::Asset
{

NNAssetMemoryPool::NNAssetMemoryPool(std::size_t blockSize, std::size_t initialCount)
	: m_BlockSize(blockSize == 0 ? 4096 : blockSize)
{
	GrowPool(initialCount);
}

NNAssetMemoryPool::~NNAssetMemoryPool()
{
	/* 释放所有 page（大块内存） */
	for (auto* page : m_Pages)
	{
		std::free(page);
	}
	m_Pages.clear();
	m_FreeList.clear();
}

void* NNAssetMemoryPool::Allocate(std::size_t size)
{
	if (size == 0) return nullptr;

	/* 大于 block 大小的分配直接走系统 malloc */
	if (size > m_BlockSize)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		++m_AllocatedCount;
		return std::malloc(size);
	}

	std::lock_guard<std::mutex> lock(m_Mutex);

	/* 池空则补充 */
	if (m_FreeList.empty())
	{
		GrowPool(m_FreeList.capacity() > 0 ? m_FreeList.capacity() : 256);
	}

	void* block = m_FreeList.back();
	m_FreeList.pop_back();
	++m_AllocatedCount;
	return block;
}

void NNAssetMemoryPool::Free(void* ptr)
{
	if (ptr == nullptr) return;

	std::lock_guard<std::mutex> lock(m_Mutex);

	if (IsPoolPointer(ptr))
	{
		m_FreeList.push_back(ptr);
	}
	else
	{
		std::free(ptr);
	}

	if (m_AllocatedCount > 0)
		--m_AllocatedCount;
}

std::size_t NNAssetMemoryPool::GetAllocatedCount() const noexcept
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	return m_AllocatedCount;
}

std::size_t NNAssetMemoryPool::GetFreeCount() const noexcept
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	return m_FreeList.size();
}

std::size_t NNAssetMemoryPool::GetPoolCapacity() const noexcept
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	return m_Pages.size() * (m_BlockSize > 0 ? (m_BlockSize > 0 ? m_BlockSize : 1) : 1);
}

std::uint64_t NNAssetMemoryPool::GetPoolUsage() const noexcept
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	return static_cast<std::uint64_t>(m_AllocatedCount) * static_cast<std::uint64_t>(m_BlockSize);
}

void NNAssetMemoryPool::GrowPool(std::size_t count)
{
	if (count == 0) count = 256;

	/* 分配一页大内存，切成 count 个 blocks */
	const std::size_t pageSize = m_BlockSize * count;
	auto* page = static_cast<std::uint8_t*>(std::malloc(pageSize));
	if (page == nullptr) return;

	m_Pages.push_back(page);

	/* 将每个 block 加入 free list */
	m_FreeList.reserve(m_FreeList.size() + count);
	for (std::size_t i = 0; i < count; ++i)
	{
		m_FreeList.push_back(page + i * m_BlockSize);
	}
}

bool NNAssetMemoryPool::IsPoolPointer(void* ptr) const noexcept
{
	const auto addr = reinterpret_cast<std::uintptr_t>(ptr);
	for (const auto* page : m_Pages)
	{
		const auto pageStart = reinterpret_cast<std::uintptr_t>(page);
		const auto pageEnd = pageStart + m_BlockSize * 256; /* 粗略检查 */
		if (addr >= pageStart && addr < pageEnd)
			return true;
	}
	return false;
}

} // namespace NN::Runtime::Asset
