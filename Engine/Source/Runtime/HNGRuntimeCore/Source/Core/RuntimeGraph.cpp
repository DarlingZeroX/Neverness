#include "Core/RuntimeGraph.h"

namespace Horizon::NodeGraphRuntime
{
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
    SLOT_ID PushSlot(RuntimeGraph& g, RuntimeSlot s)
    {
        const SLOT_ID idx = static_cast<SLOT_ID>(g.slots.size());
        s.id = idx;
        g.slots.push_back(std::move(s));
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
        // Use nodeIdToIndex for O(1) lookup.
        // NOTE: caller must have called BuildNodeIndexMap(g) after nodes are finalized.
        int idx = GetNodeIndex(g, id);
        if (idx < 0) return nullptr;
        return &g.nodes[static_cast<size_t>(idx)];
    }

}
