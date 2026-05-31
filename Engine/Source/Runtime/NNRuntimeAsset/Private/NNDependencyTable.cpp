#include "NNDependencyTable.h"

#include <algorithm>

namespace NN::Runtime::Asset
{

/* ======================== 前向依賴 ======================== */

void NNDependencyTable::SetDependencies(NNGuid asset, const NNGuid* deps, std::uint32_t count)
{
	const GuidHash key = GuidToKey(asset);

	/* 先移除舊的反向依賴 */
	auto itFwd = forward_.find(key);
	if (itFwd != forward_.end())
	{
		for (const auto& dep : itFwd->second)
		{
			auto& revList = reverse_[GuidToKey(dep)];
			revList.erase(std::remove(revList.begin(), revList.end(), asset), revList.end());
			if (revList.empty())
				reverse_.erase(GuidToKey(dep));
		}
	}

	/* 設定新的前向依賴 */
	auto& fwdList = forward_[key];
	fwdList.assign(deps, deps + count);

	/* 更新反向依賴 */
	for (std::uint32_t i = 0; i < count; ++i)
	{
		reverse_[GuidToKey(deps[i])].push_back(asset);
	}
}

void NNDependencyTable::AddDependency(NNGuid asset, NNGuid dependency)
{
	const GuidHash key = GuidToKey(asset);
	auto& fwdList = forward_[key];

	/* 檢查是否已存在 */
	for (const auto& dep : fwdList)
	{
		if (dep.low == dependency.low && dep.high == dependency.high)
			return;
	}

	fwdList.push_back(dependency);
	reverse_[GuidToKey(dependency)].push_back(asset);
}

void NNDependencyTable::RemoveDependency(NNGuid asset, NNGuid dependency)
{
	const GuidHash key = GuidToKey(asset);
	auto it = forward_.find(key);
	if (it == forward_.end())
		return;

	auto& fwdList = it->second;
	fwdList.erase(
		std::remove_if(fwdList.begin(), fwdList.end(),
			[dependency](const NNGuid& g) { return g.low == dependency.low && g.high == dependency.high; }),
		fwdList.end());

	if (fwdList.empty())
		forward_.erase(it);

	/* 更新反向 */
	auto& revList = reverse_[GuidToKey(dependency)];
	revList.erase(
		std::remove_if(revList.begin(), revList.end(),
			[asset](const NNGuid& g) { return g.low == asset.low && g.high == asset.high; }),
		revList.end());
	if (revList.empty())
		reverse_.erase(GuidToKey(dependency));
}

std::uint32_t NNDependencyTable::GetDependencyCount(NNGuid asset) const
{
	auto it = forward_.find(GuidToKey(asset));
	if (it == forward_.end())
		return 0;
	return static_cast<std::uint32_t>(it->second.size());
}

bool NNDependencyTable::GetDependencyAt(NNGuid asset, std::uint32_t index, NNGuid& outDep) const
{
	auto it = forward_.find(GuidToKey(asset));
	if (it == forward_.end() || index >= it->second.size())
		return false;
	outDep = it->second[index];
	return true;
}

/* ======================== 反向依賴 ======================== */

std::uint32_t NNDependencyTable::GetReverseDependencyCount(NNGuid asset) const
{
	auto it = reverse_.find(GuidToKey(asset));
	if (it == reverse_.end())
		return 0;
	return static_cast<std::uint32_t>(it->second.size());
}

bool NNDependencyTable::GetReverseDependencyAt(NNGuid asset, std::uint32_t index, NNGuid& outDep) const
{
	auto it = reverse_.find(GuidToKey(asset));
	if (it == reverse_.end() || index >= it->second.size())
		return false;
	outDep = it->second[index];
	return true;
}

/* ======================== 圖查詢 ======================== */

void NNDependencyTable::ClearDependencies(NNGuid asset)
{
	const GuidHash key = GuidToKey(asset);

	/* 移除反向引用 */
	auto it = forward_.find(key);
	if (it != forward_.end())
	{
		for (const auto& dep : it->second)
		{
			auto& revList = reverse_[GuidToKey(dep)];
			revList.erase(
				std::remove_if(revList.begin(), revList.end(),
					[asset](const NNGuid& g) { return g.low == asset.low && g.high == asset.high; }),
				revList.end());
			if (revList.empty())
				reverse_.erase(GuidToKey(dep));
		}
		forward_.erase(it);
	}
}

bool NNDependencyTable::HasCycle() const
{
	std::unordered_set<GuidHash> visited;
	std::unordered_set<GuidHash> inStack;

	for (const auto& [key, deps] : forward_)
	{
		if (visited.find(key) == visited.end())
		{
			/* 從這個節點開始 DFS */
			NNGuid startGuid{};
			startGuid.low = key;
			startGuid.high = 0;
			if (!deps.empty())
				startGuid = deps[0]; /* 不精確但足夠用於 DFS */

			/* 簡化：構造一個 NNGuid 作為起點 */
			NNGuid nodeGuid{};
			nodeGuid.low = key;
			if (DfsVisit(nodeGuid, visited, inStack))
				return true;
		}
	}
	return false;
}

bool NNDependencyTable::DfsVisit(NNGuid node,
                                  std::unordered_set<GuidHash>& visited,
                                  std::unordered_set<GuidHash>& inStack) const
{
	const GuidHash key = GuidToKey(node);

	/* 已完成訪問 */
	if (visited.find(key) != visited.end())
		return false;

	visited.insert(key);
	inStack.insert(key);

	auto it = forward_.find(key);
	if (it != forward_.end())
	{
		for (const auto& dep : it->second)
		{
			const GuidHash depKey = GuidToKey(dep);

			/* 灰色節點 → 發現環 */
			if (inStack.find(depKey) != inStack.end())
				return true;

			if (DfsVisit(dep, visited, inStack))
				return true;
		}
	}

	inStack.erase(key);
	return false;
}

std::uint32_t NNDependencyTable::GetNodeCount() const
{
	/* 合併前向和反向的所有節點 */
	std::unordered_set<GuidHash> nodes;
	for (const auto& [key, _] : forward_)
		nodes.insert(key);
	for (const auto& [key, _] : reverse_)
		nodes.insert(key);
	return static_cast<std::uint32_t>(nodes.size());
}

std::uint32_t NNDependencyTable::GetEdgeCount() const
{
	std::uint32_t total = 0;
	for (const auto& [key, deps] : forward_)
		total += static_cast<std::uint32_t>(deps.size());
	return total;
}

void NNDependencyTable::Clear()
{
	forward_.clear();
	reverse_.clear();
}

} // namespace NN::Runtime::Asset
