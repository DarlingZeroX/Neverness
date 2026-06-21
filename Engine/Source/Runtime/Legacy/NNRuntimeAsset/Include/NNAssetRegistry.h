#pragma once

/**
 * @file NNAssetRegistry.h
 * @brief Runtime 资产注册表（GUID ↔ 路径映射 + 依赖关系）。
 *
 * 职责：
 *   - GUID ↔ VirtualPath 双向映射
 *   - 前向依赖查询
 *   - 反向依赖查询
 *   - 环检测
 *   - 依赖图序列化/反序列化
 *
 * 设计原则：
 *   - Thread-safe（内部互斥）
 *   - 10w+ 资产可扩展（开放寻址哈希）
 *   - Runtime 可用（不依赖 Editor 逻辑）
 *   - SHARED 库导出（跨 DLL 边界单例唯一）
 */

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "../NNRuntimeAssetExport.h"
#include "NNGuidTable.h"
#include "NNDependencyTable.h"
#include "NNNativeEngineAPI/Include/EngineTypes.h"

namespace NN::Runtime::Asset
{

/**
 * @brief Runtime 资产注册表（SHARED 库导出）。
 */
class NN_ASSET_API NNAssetRegistry
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
