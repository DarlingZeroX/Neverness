#pragma once

/**
 * @file NNAssetRef.h
 * @brief 資產引用計數智慧指標。
 *
 * 包裝 NNAssetHandleT + NNAssetManager 指標，自動管理引用計數。
 * RAII 風格：建構時 +1，解構時 -1。
 */

#include <cstdint>
#include <utility>

#include "NNAssetHandle.h"

namespace NN::Runtime::Asset
{

class NNAssetManager; /* 前向宣告 */

/**
 * @brief 自動引用計數的資產指標。
 *
 * @tparam T 資產型別（void 表示未型別化）。
 */
template <typename T = void>
class NNAssetRef
{
public:
	/** @brief 空引用。 */
	NNAssetRef() noexcept : manager_(nullptr), handle_() {}

	/** @brief 建構引用並增加引用計數。 */
	NNAssetRef(NNAssetManager* mgr, NNAssetHandleT<T> handle) noexcept
		: manager_(mgr), handle_(handle)
	{
		AddRef();
	}

	/** @brief 解構時自動釋放引用。 */
	~NNAssetRef() { Release(); }

	/** @brief 複製建構。 */
	NNAssetRef(const NNAssetRef& other) noexcept
		: manager_(other.manager_), handle_(other.handle_)
	{
		AddRef();
	}

	/** @brief 複製賦值。 */
	NNAssetRef& operator=(const NNAssetRef& other) noexcept
	{
		if (this != &other)
		{
			Release();
			manager_ = other.manager_;
			handle_ = other.handle_;
			AddRef();
		}
		return *this;
	}

	/** @brief 移動建構。 */
	NNAssetRef(NNAssetRef&& other) noexcept
		: manager_(other.manager_), handle_(other.handle_)
	{
		other.manager_ = nullptr;
		other.handle_ = NNAssetHandleT<T>();
	}

	/** @brief 移動賦值。 */
	NNAssetRef& operator=(NNAssetRef&& other) noexcept
	{
		if (this != &other)
		{
			Release();
			manager_ = other.manager_;
			handle_ = other.handle_;
			other.manager_ = nullptr;
			other.handle_ = NNAssetHandleT<T>();
		}
		return *this;
	}

	/** @brief 取得 Handle。 */
	NNAssetHandleT<T> GetHandle() const noexcept { return handle_; }

	/** @brief 是否有效。 */
	explicit operator bool() const noexcept { return static_cast<bool>(handle_); }

	/** @brief 重設為空。 */
	void Reset() noexcept
	{
		Release();
		manager_ = nullptr;
		handle_ = NNAssetHandleT<T>();
	}

private:
	void AddRef() noexcept;
	void Release() noexcept;

	NNAssetManager*   manager_;
	NNAssetHandleT<T> handle_;
};

} // namespace NN::Runtime::Asset
