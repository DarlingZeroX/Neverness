#pragma once
#include <gtest/gtest.h>
#include "HNodeGraph/Include/Core/RuntimeGraph.h"

using namespace NodeGraphRuntime;

// Helper to build a small graph used by multiple tests
static RuntimeGraph BuildBranchGraph(bool branchConditionTrue)
{
    RuntimeGraph g;

    // nodes
    RuntimeNode entry; entry.id = 1; entry.type = NodeType::Entry; entry.execute = nullptr;
    RuntimeNode branch; branch.id = 2; branch.type = NodeType::Branch; branch.execute = nullptr;
    RuntimeNode A; A.id = 3; A.type = NodeType::Dialogue; A.execute = nullptr;
    RuntimeNode B; B.id = 4; B.type = NodeType::Dialogue; B.execute = nullptr;

    g.nodes.push_back(entry);
    g.nodes.push_back(branch);
    g.nodes.push_back(A);
    g.nodes.push_back(B);

	// build node id -> index map for O(1) node lookups
	BuildNodeIndexMap(g);

    // entry output -> branch
    RuntimeSlot entryOut; entryOut.id = MakeSlotId(entry.id, 0); entryOut.type = SlotType::Int; entryOut.value = Value(static_cast<int64_t>(branch.id));
    uint32_t eidx = PushSlot(g, entryOut);
	int entryIndex = GetNodeIndex(g, entry.id);
	if (entryIndex >= 0) { g.nodes[entryIndex].outputsBegin = eidx; g.nodes[entryIndex].outputsCount = 1; }
    g.slotToNode[entryOut.id] = entry.id;

    // branch condition input
    RuntimeSlot cond; cond.id = MakeSlotId(branch.id, 0); cond.type = SlotType::Bool; cond.value = Value(branchConditionTrue);
    uint32_t cidx = PushSlot(g, cond);
	int branchIndex = GetNodeIndex(g, branch.id);
	if (branchIndex >= 0) { g.nodes[branchIndex].inputsBegin = cidx; g.nodes[branchIndex].inputsCount = 1; }
    g.slotToNode[cond.id] = branch.id;

    // branch outputs
    RuntimeSlot tslot; tslot.id = MakeSlotId(branch.id, 1); tslot.type = SlotType::Exec; tslot.name = "True"; tslot.value = Value(static_cast<int64_t>(A.id));
	RuntimeSlot fslot; fslot.id = MakeSlotId(branch.id, 2); fslot.type = SlotType::Exec; fslot.name = "False"; fslot.value = Value(static_cast<int64_t>(B.id));
    uint32_t tidx = PushSlot(g, tslot);
    uint32_t fidx = PushSlot(g, fslot);
    g.slotToNode[tslot.id] = branch.id;
    g.slotToNode[fslot.id] = branch.id;
	// set branch outputs range
    branchIndex = GetNodeIndex(g, branch.id);
	if (branchIndex >= 0)
	{
		g.nodes[branchIndex].outputsBegin = tidx;
		g.nodes[branchIndex].outputsCount = 2;
	}

    // A and B input exec slots
    RuntimeSlot Ain; Ain.id = MakeSlotId(A.id, 0); Ain.type = SlotType::Exec; Ain.name = "In";
    uint32_t aInIdx = PushSlot(g, Ain);
	g.slotToNode[Ain.id] = A.id;
	int aIndex = GetNodeIndex(g, A.id);
	if (aIndex >= 0) { g.nodes[aIndex].inputsBegin = aInIdx; g.nodes[aIndex].inputsCount = 1; }

    RuntimeSlot Bin; Bin.id = MakeSlotId(B.id, 0); Bin.type = SlotType::Exec; Bin.name = "In";
    uint32_t bInIdx = PushSlot(g, Bin);
	g.slotToNode[Bin.id] = B.id;
	int bIndex = GetNodeIndex(g, B.id);
	if (bIndex >= 0) { g.nodes[bIndex].inputsBegin = bInIdx; g.nodes[bIndex].inputsCount = 1; }

    // connect edges: branch true -> Ain; branch false -> Bin
	g.edges.push_back(RuntimeEdge(tslot.id, Ain.id));
	g.edges.push_back(RuntimeEdge(fslot.id, Bin.id));
	g.edgeFromTo[tslot.id].push_back(Ain.id);
	g.edgeFromTo[fslot.id].push_back(Bin.id);

	// map destination slots to node indices (ensure slotIdToIndex filled)
	g.slotToNode[Ain.id] = A.id;
	g.slotToNode[Bin.id] = B.id;

    // set entry
    g.entryNodeId = entry.id;

	// build edge maps
	BuildEdgeMaps(g);

    return g;
}


//#include <gtest/gtest.h>
//#include "TestRuntimeGraph.hpp" // helper that builds graph
//#include "HNodeGraph/Include/Core/RuntimeGraph.h"

using namespace NodeGraphRuntime;

// Entry: 推进到 entry.outputs 中指定的下一个节点
static ExecResult EntryFn(RuntimeContext& ctx, NODE_ID nodeIndex)
{
	if (!ctx.graph) return ExecResult::Finished;
	RuntimeNode* cur = GetNodeById(*ctx.graph, nodeIndex);
	if (!cur) return ExecResult::Finished;
	if (cur->outputsCount > 0)
	{
		RuntimeSlot& s = ctx.graph->slots[cur->outputsBegin];
		if (s.value.type == ValueType::Int)
		{
			NODE_ID next = static_cast<NODE_ID>(s.value.AsInt());
			ctx.execStack.push_back(next);
		}
	}
	return ExecResult::Finished;
}

// Branch: 根据第一个输入槽的布尔值路由到命名输出 "True"/"False"
static ExecResult BranchFn(RuntimeContext& ctx, NODE_ID nodeIndex)
{
	if (!ctx.graph) return ExecResult::Finished;
	RuntimeNode* node = GetNodeById(*ctx.graph, nodeIndex);
	if (!node) return ExecResult::Finished;
	if (node->inputsCount == 0) return ExecResult::Finished;

	RuntimeSlot& cond = ctx.graph->slots[node->inputsBegin];
	bool c = cond.value.AsBool();

	SLOT_ID trueSlot = FindOutputSlot(*ctx.graph, *node, "True");
	SLOT_ID falseSlot = FindOutputSlot(*ctx.graph, *node, "False");

	if (c && trueSlot != 0)
	{
		NODE_ID next = FindNextNode(ctx, trueSlot);
		if (next) ctx.execStack.push_back(next);
	}
	else if (!c && falseSlot != 0)
	{
		NODE_ID next = FindNextNode(ctx, falseSlot);
		if (next) ctx.execStack.push_back(next);
	}
	return ExecResult::Finished;
}

// Dialogue: 根据 nodeIndex 标记访问 A 或 B
static ExecResult DialogueFn(RuntimeContext& ctx, NODE_ID nodeIndex)
{
	const char* key = (nodeIndex == 3) ? "visitedA" : (nodeIndex == 4) ? "visitedB" : "visitedOther";
	int64_t prev = ctx.variables[ key ].AsInt();
	ctx.variables[ key ] = Value(static_cast<int64_t>(prev + 1));
	return ExecResult::Finished;
}

// 测试：当分支条件为 true 时，应执行 A 而非 B
TEST(HNodeGraphRuntime, BranchTrueRoutesToA)
{
	RuntimeGraph g = BuildBranchGraph(true);

	// 注册执行函数
	NodeRegistry reg;
	reg.Register(NodeType::Entry, EntryFn);
	reg.Register(NodeType::Branch, BranchFn);
	reg.Register(NodeType::Dialogue, DialogueFn);

	// 运行
	RuntimeContext ctx; ctx.graph = &g;
	ctx.execStack.push_back(g.entryNodeId);

	while (!ctx.execStack.empty())
	{
		NODE_ID nid = ctx.execStack.back(); ctx.execStack.pop_back();
		RuntimeNode* node = GetNodeById(g, nid);
		ASSERT_NE(node, nullptr);
		NodeExecuteFn fn = reg.Get(node->type);
		ASSERT_NE(fn, nullptr);
		ExecResult r = fn(ctx, node->id);
		if (r == ExecResult::Running)
		{
			ctx.execStack.push_back(nid);
		}
	}

	int visitedA = static_cast<int>(ctx.variables["visitedA"].AsInt());
	int visitedB = static_cast<int>(ctx.variables["visitedB"].AsInt());
	ASSERT_EQ(visitedA, 1);
	ASSERT_EQ(visitedB, 0);
}

// 测试：当分支条件为 false 时，应执行 B 而非 A
TEST(HNodeGraphRuntime, BranchFalseRoutesToB)
{
	RuntimeGraph g = BuildBranchGraph(false);

	// 注册执行函数
	NodeRegistry reg;
	reg.Register(NodeType::Entry, EntryFn);
	reg.Register(NodeType::Branch, BranchFn);
	reg.Register(NodeType::Dialogue, DialogueFn);

	// 运行
	RuntimeContext ctx; ctx.graph = &g;
	ctx.execStack.push_back(g.entryNodeId);

	while (!ctx.execStack.empty())
	{
		NODE_ID nid = ctx.execStack.back(); ctx.execStack.pop_back();
		RuntimeNode* node = GetNodeById(g, nid);
		ASSERT_NE(node, nullptr);
		NodeExecuteFn fn = reg.Get(node->type);
		ASSERT_NE(fn, nullptr);
		ExecResult r = fn(ctx, node->id);
		if (r == ExecResult::Running)
		{
			ctx.execStack.push_back(nid);
		}
	}

	int visitedA = static_cast<int>(ctx.variables["visitedA"].AsInt());
	int visitedB = static_cast<int>(ctx.variables["visitedB"].AsInt());
	ASSERT_EQ(visitedA, 0);
	ASSERT_EQ(visitedB, 1);
}
