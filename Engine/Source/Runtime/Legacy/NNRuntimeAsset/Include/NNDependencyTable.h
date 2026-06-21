#pragma once

/**
 * @file NNDependencyTable.h
 * @brief 依赖关系图（前向 + 反向 + 环检测）。
 *
 * 数据结构：
 *   前向依赖：asset A → [B, C]（A 依赖 B 和 C）
 *   反向依赖：asset B → [A]（B 被 A 引用）
 *
 * 环检测：DFS + 三色标记（白/灰/黑）。
 */

#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../NNRuntimeAssetExport.h"
#include "NNNativeEngineAPI/Include/EngineTypes.h"

namespace NN::Runtime::Asset
{

/**
 * @brief 依赖关系图。
 */
class NN_ASSET_API NNDependencyTable
{
public:
	/* === 前向依賴 === */

	/** @brief 設定資產完整依賴列表（替換舊列表）。 */
	void SetDependencies(NNGuid asset, const NNGuid* deps, std::uint32_t count);

	/** @brief 新增單個依賴。 */
	void AddDependency(NNGuid asset, NNGuid dependency);

	/** @brief 移除單個依賴。 */
	void RemoveDependency(NNGuid asset, NNGuid dependency);

	/** @brief 取得前向依賴數量。 */
	std::uint32_t GetDependencyCount(NNGuid asset) const;

	/** @brief 取得前向依賴。 */
	bool GetDependencyAt(NNGuid asset, std::uint32_t index, NNGuid& outDep) const;

	/* === 反向依賴 === */

	/** @brief 取得反向依賴數量（誰引用了 asset）。 */
	std::uint32_t GetReverseDependencyCount(NNGuid asset) const;

	/** @brief 取得反向依賴。 */
	bool GetReverseDependencyAt(NNGuid asset, std::uint32_t index, NNGuid& outDep) const;

	/* === 圖查詢 === */

	/** @brief 清除某資產的所有依賴。 */
	void ClearDependencies(NNGuid asset);

	/** @brief 檢測是否存在環。 */
	bool HasCycle() const;

	/** @brief 取得圖中節點（資產）數量。 */
	std::uint32_t GetNodeCount() const;

	/** @brief 取得圖中邊（依賴關係）總數。 */
	std::uint32_t GetEdgeCount() const;

	/** @brief 清除所有資料。 */
	void Clear();

private:
	using GuidHash = std::uint64_t;
	static GuidHash GuidToKey(NNGuid g) { return g.low; }

	bool DfsVisit(NNGuid node,
	              std::unordered_set<GuidHash>& visited,
	              std::unordered_set<GuidHash>& inStack) const;

	/* 前向依賴：asset → [dep1, dep2, ...] */
	std::unordered_map<GuidHash, std::vector<NNGuid>> forward_;
	/* 反向依賴：dep → [asset1, asset2, ...] */
	std::unordered_map<GuidHash, std::vector<NNGuid>> reverse_;
};

} // namespace NN::Runtime::Asset
