#include "Core/RuntimeGraph.h"

namespace Horizon::NodeGraphRuntime
{
    // 示例帮助函数：创建节点槽
    SLOT_ID MakeSlotId(NODE_ID nodeId, uint32_t localIndex)
    {
        // high 32 bits node id, low 32 bits local index
        return (static_cast<SLOT_ID>(nodeId) << 32) | static_cast<SLOT_ID>(localIndex);
    }

    // Build nodeIdToIndex map
    void BuildNodeIndexMap(RuntimeGraph& g)
    {
        g.nodeIdToIndex.clear();
        for (uint32_t i = 0; i < g.nodes.size(); ++i)
        {
            g.nodeIdToIndex[g.nodes[i].id] = i;
        }
    }

    int GetNodeIndex(RuntimeGraph& g, NODE_ID id)
    {
        auto it = g.nodeIdToIndex.find(id);
        if (it == g.nodeIdToIndex.end()) return -1;
        return static_cast<int>(it->second);
    }

    // helper to push a slot and return its index in graph.slots
    uint32_t PushSlot(RuntimeGraph& g, const RuntimeSlot& s)
    {
        g.slots.push_back(s);
        uint32_t idx = static_cast<uint32_t>(g.slots.size() - 1);
        g.slotIdToIndex[s.id] = idx;
        return idx;
    }

    // helper to ensure slotToNode mapping is updated from edges vector (call after building edges)
    void BuildEdgeMaps(RuntimeGraph& g)
    {
        // clear existing
        g.edgeFromTo.clear();
        for (const RuntimeEdge& e : g.edges)
        {
            g.edgeFromTo[e.from].push_back(e.to);
        }
        // also ensure slotIdToIndex for existing slots (already filled by PushSlot)
        for (uint32_t i = 0; i < g.slots.size(); ++i)
        {
            g.slotIdToIndex[g.slots[i].id] = i;
        }
    }

    // find output slot by name
    SLOT_ID FindOutputSlot(RuntimeGraph& g, const RuntimeNode& node, const std::string& name)
    {
        uint32_t begin = node.outputsBegin;
        uint32_t count = node.outputsCount;
        for (uint32_t i = 0; i < count; ++i)
        {
            const RuntimeSlot& s = g.slots[begin + i];
            if (s.name == name) return s.id;
        }
        return 0;
    }

    RuntimeNode* GetNodeById(RuntimeGraph& g, NODE_ID id)
    {
        int idx = g.FindNodeIndex(id);
        if (idx < 0) return nullptr;
        return &g.nodes[static_cast<size_t>(idx)];
    }

}
