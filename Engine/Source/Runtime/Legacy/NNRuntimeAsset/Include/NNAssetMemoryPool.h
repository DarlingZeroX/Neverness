#pragma once

/**
 * @file NNAssetMemoryPool.h
 * @brief 线程安全的块内存池（Phase 8 性能优化）。
 *
 * 预分配固定大小 block 的内存池，减少资产数据加载时的 malloc 碎片。
 * 小于等于 block 大小的分配从池中获取，大于 block 的直接走系统分配。
 *
 * 设计原则：
 *   - 固定 block 大小（默认 4KB），free list 管理
 *   - 池耗尽时 fallback 到 malloc，不阻塞
 *   - std::mutex 保护 free list
 *   - 统计接口供调试
 */

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <vector>

namespace NN::Runtime::Asset
{

/**
 * @brief 线程安全的块内存池。
 *
 * 使用方法：
 *   NNAssetMemoryPool pool(4096, 256);  // 4KB blocks, 初始 256 个
 *   void* p = pool.Allocate(2048);      // 从池中获取一个 block
 *   pool.Free(p);                       // 归还池中
 */
class NNAssetMemoryPool
{
public:
	/** @brief 构造内存池。
	 *  @param blockSize    每个 block 的大小（字节），默认 4096
	 *  @param initialCount 初始预分配的 block 数量，默认 256
	 */
	explicit NNAssetMemoryPool(
		std::size_t blockSize = 4096,
		std::size_t initialCount = 256);

	~NNAssetMemoryPool();

	NNAssetMemoryPool(const NNAssetMemoryPool&) = delete;
	NNAssetMemoryPool& operator=(const NNAssetMemoryPool&) = delete;

	/** @brief 分配内存。大小 <= blockSize 从池中取，否则 fallback 到 malloc。 */
	void* Allocate(std::size_t size);

	/** @brief 释放内存。池分配的归还池中，系统分配的 free。 */
	void Free(void* ptr);

	/** @brief 取得 block 大小。 */
	[[nodiscard]] std::size_t GetBlockSize() const noexcept { return m_BlockSize; }

	/** @brief 取得已分配 block 数量（含池外分配）。 */
	[[nodiscard]] std::size_t GetAllocatedCount() const noexcept;

	/** @brief 取得池中空闲 block 数量。 */
	[[nodiscard]] std::size_t GetFreeCount() const noexcept;

	/** @brief 取得池总容量（已分配 + 空闲）。 */
	[[nodiscard]] std::size_t GetPoolCapacity() const noexcept;

	/** @brief 取得池使用量（字节）。 */
	[[nodiscard]] std::uint64_t GetPoolUsage() const noexcept;

private:
	/** @brief 向池中补充 blocks。 */
	void GrowPool(std::size_t count);

	/** @brief 判断指针是否属于池分配。 */
	[[nodiscard]] bool IsPoolPointer(void* ptr) const noexcept;

	const std::size_t m_BlockSize;
	std::vector<void*> m_FreeList;          // 空闲 block 列表
	std::vector<std::uint8_t*> m_Pages;     // 大块页（用于析构释放）
	std::size_t m_AllocatedCount{0};        // 当前已借出的 block 数
	mutable std::mutex m_Mutex;
};

} // namespace NN::Runtime::Asset
