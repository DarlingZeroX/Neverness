#pragma once

/**
 * @file NNAssetRegistry.h
 * @brief Runtime 資產註冊表（GUID ↔ 路徑映射 + 依賴關係）。
 *
 * 職責：
 *   - GUID ↔ VirtualPath 雙向映射
 *   - 前向依賴查詢
 *   - 反向依賴查詢
 *   - 環檢測
 *   - 依賴圖序列化/反序列化
 *
 * 設計原則：
 *   - Thread-safe（內部互斥）
 *   - 10w+ 資產可擴展（開放尋址哈希）
 *   - Runtime 可用（不依賴 Editor 邏輯）
 */

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "NNGuidTable.h"
#include "NNDependencyTable.h"
#include "NNNativeEngineAPI/Include/EngineTypes.h"

namespace NN::Runtime::Asset
{

/**
 * @brief Runtime 資產註冊表。
 */
class NNAssetRegistry
{
public:
	static NNAssetRegistry& Instance() noexcept;

	/* === GUID ↔ Path === */

	int RegisterAsset(const char* virtualPathUtf8, NNGuid guid) noexcept;
	int UnregisterByGuid(NNGuid guid) noexcept;
	int UnregisterByPath(const char* virtualPathUtf8) noexcept;
	int ResolvePathByGuid(NNGuid guid, char* outUtf8, std::size_t outCapacity) const noexcept;
	int ResolveGuidByPath(const char* virtualPathUtf8, NNGuid* outGuid) const noexcept;

	/* === 依賴管理 === */

	int SetDependencies(NNGuid guid, const NNGuid* deps, std::uint32_t count) noexcept;
	int AddDependency(NNGuid guid, NNGuid dependency) noexcept;
	int RemoveDependency(NNGuid guid, NNGuid dependency) noexcept;

	/* === 前向依賴 === */
	std::uint32_t GetDependencyCount(NNGuid guid) const noexcept;
	int GetDependencyAt(NNGuid guid, std::uint32_t index, NNGuid* outDependency) const noexcept;

	/* === 反向依賴 === */
	std::uint32_t GetReverseDependencyCount(NNGuid guid) const noexcept;
	int GetReverseDependencyAt(NNGuid guid, std::uint32_t index, NNGuid* outDep) const noexcept;

	/* === 圖查詢 === */
	int HasCycle() const noexcept;
	std::uint32_t GetAssetCount() const noexcept;
	std::uint32_t GetEdgeCount() const noexcept;

	/* === 匯入（合成 GUID） === */
	NNGuid ImportAsset(const char* virtualPathUtf8) noexcept;

private:
	NNAssetRegistry() = default;

	static bool GuidIsZero(NNGuid g) noexcept;
	static NNGuid MakeSyntheticGuid(const std::string& path) noexcept;
	static std::uint64_t HashPath(const std::string& s) noexcept;

	mutable std::mutex mutex_;
	NNGuidTable        guidTable_;
	NNDependencyTable  depTable_;
};

} // namespace NN::Runtime::Asset
