#pragma once

/**
 * @file NNAssetHandle.h
 * @brief 類型化資產 Handle 模板 + Handle Table。
 *
 * Handle 為 uint64 編碼：
 *   - 低 32 位：HandleTable 索引
 *   - 高 32 位：generation（防 ABA）
 *
 * 設計原則：
 *   - lightweight（8 位元組）
 *   - cache-friendly（連續儲存）
 *   - thread-safe（HandleTable 內部互斥）
 */

#include <atomic>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <vector>

#include "Engine/EngineTypes.h"
#include "Engine/EngineHandles.h"

namespace NN::Runtime::Asset
{

/* ======================== Handle Table ======================== */

/**
 * @brief 集中式 Handle 分配表（索引 + generation）。
 *
 * Slot 被釋放後進入 free list；重新分配時 generation++，
 * 使舊 Handle 自動失效（generation 不匹配 → Resolve 回傳 nullptr）。
 */
class NNHandleTable
{
public:
	static constexpr std::uint32_t INVALID_INDEX = 0xFFFFFFFFu;

	/** @brief 分配一個 slot，回傳編碼後的 Handle。 */
	std::uint64_t Allocate(void* data, std::uint64_t typeId);

	/** @brief 釋放 slot（進入 free list）。 */
	void Free(std::uint64_t handle);

	/** @brief 解析 Handle → 資料指標（generation 不匹配或已釋放回傳 nullptr）。 */
	void* Resolve(std::uint64_t handle) const;

	/** @brief 取得 slot 之型別 ID。 */
	std::uint64_t GetTypeId(std::uint64_t handle) const;

	/** @brief 增加引用計數。 */
	void AddRef(std::uint64_t handle);

	/** @brief 減少引用計數，回傳是否已降至 0（但不自動 Free）。 */
	bool Release(std::uint64_t handle);

	/** @brief 取得引用計數。 */
	std::uint32_t GetRefCount(std::uint64_t handle) const;

	/** @brief 取得已分配 slot 數量。 */
	std::uint32_t GetAllocatedCount() const;

private:
	struct Slot
	{
		void*          data{nullptr};
		std::uint64_t  typeId{0};
		std::atomic<std::uint32_t> refCount{0};
		std::uint32_t  generation{0};
		bool           alive{false};

		Slot() = default;
		Slot(const Slot& other)
			: data(other.data), typeId(other.typeId)
			, refCount(other.refCount.load(std::memory_order_relaxed))
			, generation(other.generation), alive(other.alive) {}
		Slot& operator=(const Slot& other)
		{
			data = other.data; typeId = other.typeId;
			refCount.store(other.refCount.load(std::memory_order_relaxed), std::memory_order_relaxed);
			generation = other.generation; alive = other.alive;
			return *this;
		}
		Slot(Slot&& other) noexcept
			: data(other.data), typeId(other.typeId)
			, refCount(other.refCount.load(std::memory_order_relaxed))
			, generation(other.generation), alive(other.alive) {}
		Slot& operator=(Slot&& other) noexcept
		{
			data = other.data; typeId = other.typeId;
			refCount.store(other.refCount.load(std::memory_order_relaxed), std::memory_order_relaxed);
			generation = other.generation; alive = other.alive;
			return *this;
		}
	};

	/* --- Handle 編解碼 --- */
	static std::uint32_t HandleToIndex(std::uint64_t h) { return static_cast<std::uint32_t>(h & 0xFFFFFFFFull) - 1; }
	static std::uint32_t HandleToGen(std::uint64_t h)   { return static_cast<std::uint32_t>(h >> 32); }
	static std::uint64_t MakeHandle(std::uint32_t idx, std::uint32_t gen)
	{
		return (static_cast<std::uint64_t>(gen) << 32) | static_cast<std::uint64_t>(idx + 1);
	}

	mutable std::mutex       mutex_;
	std::vector<Slot>        slots_;
	std::vector<std::uint32_t> freeList_;
	std::uint32_t            allocatedCount_{0};
};

/* ======================== 類型化 Handle ======================== */

/**
 * @brief 類型化資產 Handle（僅持 uint64，8 位元組）。
 *
 * 模板參數 T 僅用於型別安全；實際資料由 NNHandleTable 管理。
 * T = void 表示未型別化 Handle。
 */
template <typename T = void>
class NNAssetHandleT
{
public:
	/** @brief 無效 Handle。 */
	NNAssetHandleT() noexcept : value_(0) {}

	/** @brief 從原始值建構（通常由 NNAssetManager 內部使用）。 */
	explicit NNAssetHandleT(std::uint64_t raw) noexcept : value_(raw) {}

	/** @brief 是否有效（非零）。 */
	explicit operator bool() const noexcept { return value_ != 0; }

	/** @brief 取得原始值（用於跨 ABI 傳遞）。 */
	std::uint64_t Value() const noexcept { return value_; }

	/** @brief 取得對應的 C ABI Handle。 */
	NNAssetHandle ToAbi() const noexcept { return static_cast<NNAssetHandle>(value_); }

	/** @brief 從 C ABI Handle 建構。 */
	static NNAssetHandleT FromAbi(NNAssetHandle h) noexcept { return NNAssetHandleT(static_cast<std::uint64_t>(h)); }

	bool operator==(NNAssetHandleT other) const noexcept { return value_ == other.value_; }
	bool operator!=(NNAssetHandleT other) const noexcept { return value_ != other.value_; }

private:
	std::uint64_t value_;
};

/* ======================== 常用別名 ======================== */

using NNAssetHandleGeneric = NNAssetHandleT<void>;

} // namespace NN::Runtime::Asset
