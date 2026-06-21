/**
 * @file NNHierarchySnapshotBuilder.cpp
 * @brief 层级快照构建器实现——两遍扫描 + DFS 输出。
 */

#include "Snapshot/NNHierarchySnapshotBuilder.h"

#include <algorithm>
#include <cstring>
#include <functional>
#include <vector>

#include "Components/NNRelationshipComponent.h"
#include "Components/NNTagComponent.h"
#include "NNNativeEngineAPI/Include/EditorSceneAPI.h"

namespace NN::Runtime::Scene
{

std::uint32_t NNHierarchySnapshotBuilder::EstimateSize(const NNRuntimeScene& scene)
{
	const auto& registry = scene.GetRegistry();

	std::uint32_t nodeCount     = 0u;
	std::uint32_t namePoolBytes = 0u;

	registry.view<NNRelationshipComponent>().each(
		[&](const entt::entity enttE, const NNRelationshipComponent&)
		{
			nodeCount++;

			const auto* tag = registry.try_get<NNTagComponent>(enttE);
			if (tag != nullptr)
			{
				namePoolBytes += static_cast<std::uint32_t>(strnlen(tag->Name, sizeof(tag->Name) - 1)) + 1u;
			}
			else
			{
				namePoolBytes += 1u; // 空字符串 = 单个 NUL
			}
		});

	return static_cast<std::uint32_t>(sizeof(NNSceneSnapshotHeader))
	     + nodeCount * static_cast<std::uint32_t>(sizeof(NNSceneNodeSnapshot))
	     + namePoolBytes;
}

std::uint32_t NNHierarchySnapshotBuilder::Build(
	const NNRuntimeScene& scene,
	void*                 outBuffer,
	std::uint32_t         capacity)
{
	const auto& registry    = scene.GetRegistry();
	const auto& handleTable = scene.GetHandleTable();
	const auto& hierarchy   = const_cast<NNRuntimeScene&>(scene).GetHierarchySystem();

	// ── 第一遍：统计节点数 + 名字池大小，收集根节点 ──

	std::uint32_t nodeCount     = 0u;
	std::uint32_t namePoolBytes = 0u;
	std::vector<NNEntity> roots;

	registry.view<NNRelationshipComponent>().each(
		[&](const entt::entity enttE, const NNRelationshipComponent& rel)
		{
			nodeCount++;

			if (rel.Parent == NNEntityInvalid)
			{
				roots.push_back(handleTable.HandleFromEntt(enttE));
			}

			const auto* tag = registry.try_get<NNTagComponent>(enttE);
			if (tag != nullptr)
			{
				namePoolBytes += static_cast<std::uint32_t>(strnlen(tag->Name, sizeof(tag->Name) - 1)) + 1u;
			}
			else
			{
				namePoolBytes += 1u;
			}
		});

	// 根节点排序（稳定 DFS 序，按 handle 值升序）
	std::sort(roots.begin(), roots.end());

	// ── 验证容量 ──

	const std::uint32_t needed = static_cast<std::uint32_t>(sizeof(NNSceneSnapshotHeader))
	                           + nodeCount * static_cast<std::uint32_t>(sizeof(NNSceneNodeSnapshot))
	                           + namePoolBytes;

	if (capacity < needed || outBuffer == nullptr)
	{
		return needed; // 返回所需大小，调用方可用于 getSnapshotSize
	}

	// ── 写 Header ──

	auto* header = static_cast<NNSceneSnapshotHeader*>(outBuffer);
	header->magic            = 0x56475343u; // 'VGSC'
	header->layoutVersion    = 1u;
	header->hierarchyVersion = scene.HierarchyVersion();
	header->nodeCount        = nodeCount;
	header->namePoolBytes    = namePoolBytes;
	header->rootCount        = static_cast<std::uint32_t>(roots.size());
	header->_pad             = 0u;

	// ── DFS 遍历：写 Nodes[] + NamePool ──

	auto* nodes    = reinterpret_cast<NNSceneNodeSnapshot*>(header + 1);
	char* namePool = reinterpret_cast<char*>(nodes + nodeCount);

	// 构建 Parent → Children 映射
	const auto& childrenMap = hierarchy.GetAllChildrenMap();

	std::uint32_t nodeIdx    = 0u;
	std::uint32_t nameOffset = 0u;

	// 递归 DFS lambda
	std::function<void(NNEntity, std::uint32_t)> dfs;
	dfs = [&](const NNEntity entity, const std::uint32_t depth)
	{
		auto& n = nodes[nodeIdx++];

		n.entity     = entity;
		n.parent     = NNEntityInvalid;
		n.depth      = depth;
		n.childCount = 0u;
		n.nameOffset = nameOffset;
		n.flags      = 0u;
		n._pad       = 0u;

		// 读 Relationship
		const entt::entity enttE = handleTable.Resolve(entity);
		if (const auto* rel = registry.try_get<NNRelationshipComponent>(enttE))
		{
			n.parent     = rel->Parent;
			n.childCount = rel->ChildCount;
		}

		// 读 Tag → 写入 namePool
		if (const auto* tag = registry.try_get<NNTagComponent>(enttE))
		{
			n.flags = tag->Flags;
			const std::uint32_t len = static_cast<std::uint32_t>(
				strnlen(tag->Name, sizeof(tag->Name) - 1));
			std::memcpy(namePool + nameOffset, tag->Name, len);
			namePool[nameOffset + len] = '\0';
			n.nameLen = len;
			nameOffset += len + 1u;
		}
		else
		{
			namePool[nameOffset] = '\0';
			n.nameLen = 0u;
			nameOffset += 1u;
		}

		// 子节点逆序入栈（保持 DFS 有序：handle 小的先访问）
		const auto it = childrenMap.find(entity);
		if (it != childrenMap.end())
		{
			const auto& children = it->second;
			for (int i = static_cast<int>(children.size()) - 1; i >= 0; --i)
			{
				dfs(children[i], depth + 1u);
			}
		}
	};

	// 从根节点开始 DFS
	for (int i = static_cast<int>(roots.size()) - 1; i >= 0; --i)
	{
		dfs(roots[i], 0u);
	}

	return needed;
}

} // namespace NN::Runtime::Scene
