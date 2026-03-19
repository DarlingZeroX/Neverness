/*
 * 运行时 NodeGraph 实现（无虚函数、POD 风格、函数指针驱动）
 * 提供基础的数据结构：Value, RuntimeSlot, RuntimeEdge, RuntimeNode, RuntimeGraph, RuntimeContext
 */

#pragma once

#include <any>
#include <vector>
#include <string>
#include <unordered_map>
#include <cstdint>
#include <functional>
#include "../../Interface/Value.h"
#include "../../Interface/Types.h"
#include "../../Interface/RuntimeCore.h"
#include "../../HNodeGraphConfig.h"

namespace Horizon::NodeGraphRuntime
{
	struct RuntimeSlot;
	struct RuntimeNode;
	struct RuntimeGraph;
	struct RuntimeContext;
	struct NodeRegistry;
	//struct RuntimeContext; // forward

    // 将一个 RuntimeSlot 推入 graph.slots 并返回其 SLOT_ID（即 slots 索引）
    H_NODE_GRAPH_API SLOT_ID PushSlot(RuntimeGraph& g, RuntimeSlot s);

    // 根据节点 ID 获取 RuntimeNode 指针（返回 nullptr 则未找到）
    H_NODE_GRAPH_API RuntimeNode* GetNodeById(RuntimeGraph& g, NODE_ID id);

    // 将 graph.edges 中的所有边构建为 edgeFromTo 映射，便于 O(1) 查找某个槽的目标槽集合
    H_NODE_GRAPH_API void BuildEdgeMaps(RuntimeGraph& g);

    // 在节点的输出槽范围内按名称查找对应的槽并返回其 SLOT_ID（未找到返回 0）
    H_NODE_GRAPH_API SLOT_ID FindOutputSlot(RuntimeGraph& g, const RuntimeNode& node, const std::string& name);

		// Build nodeIdToIndex mapping from nodes vector
	H_NODE_GRAPH_API void BuildNodeIndexMap(RuntimeGraph& g);

	// O(1) node index lookup using nodeIdToIndex map. Returns -1 if not found.
	H_NODE_GRAPH_API int GetNodeIndex(RuntimeGraph& g, NODE_ID id);

    // RuntimeGraph: 包含节点、槽、边
    struct RuntimeGraph
    {
    	std::vector<RuntimeNode> nodes; // 节点列表，按构建顺序存储
        std::vector<RuntimeSlot> slots; // 槽列表，所有节点的输入输出槽按顺序存储
        std::vector<RuntimeEdge> edges; // 边列表，描述槽之间的连接关系

        NODE_ID entryNodeId; // 图的入口节点 ID

        // 槽 ID 到所属节点 ID 的映射，用于 O(1) 查找槽所属节点
        std::unordered_map<SLOT_ID, NODE_ID> slotToNode;
        // 边的映射：输出槽 ID 到目标槽 ID 列表，用于 O(1) 查找所有下游槽
        std::unordered_map<SLOT_ID, std::vector<SLOT_ID>> edgeFromTo;
		// 边的反向映射：输入槽 ID 到上游输出槽 ID 列表（Data Flow 读取用）
		// 说明：
		// - 用于 pull model：当节点读取某个输入槽时，可通过 edgeToFrom 找到其上游输出槽
		// - 当前版本只取第一个上游（后续可扩展 merge/多输入）
		std::unordered_map<SLOT_ID, std::vector<SLOT_ID>> edgeToFrom;
        // 节点 ID 到 nodes 向量索引的映射，用于 O(1) 查找节点对象
        std::unordered_map<NODE_ID, uint32_t> nodeIdToIndex;

        RuntimeGraph() noexcept : entryNodeId(0) {}

        // 辅助函数：通过节点 ID 查找节点索引（已弃用，建议用 GetNodeIndex）
        [[deprecated("Use GetNodeIndex for O(1) lookup")]]
        int FindNodeIndex(NODE_ID id) const noexcept
        {
            for (size_t i = 0; i < nodes.size(); ++i)
            {
                if (nodes[i].id == id) return static_cast<int>(i);
            }
            return -1;
        }
    };
}
