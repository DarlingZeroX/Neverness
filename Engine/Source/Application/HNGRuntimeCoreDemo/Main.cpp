#include <HNGRuntimeCore/Include/Core/RuntimeGraph.h>
#include <HNGRuntimeCore/Include/Core/Nodes.h>
#include <HNGRuntimeCore/Include/Core/RuntimeContext.h>
#include <cstdio>
#include <string>
	
using namespace Horizon::NodeGraphRuntime;

int main(int argc, char** argv) {
    // 1. 构建节点
    RuntimeGraph g;

    // Entry
    RuntimeNode entry; entry.id = 1; entry.type = NodeType::Entry; entry.execute = EntryNodeExecute;
    // Dialogue
    RuntimeNode dlg; dlg.id = 2; dlg.type = NodeType::Dialogue; dlg.execute = DialogueNodeExecute;
    // Branch
    RuntimeNode branch; branch.id = 3; branch.type = NodeType::Branch; branch.execute = BranchNodeExecute;
    // EndA
    RuntimeNode endA; endA.id = 4; endA.type = NodeType::Invalid; endA.execute = nullptr;
    // EndB
    RuntimeNode endB; endB.id = 5; endB.type = NodeType::Invalid; endB.execute = nullptr;

    g.nodes = { entry, dlg, branch, endA, endB };
    BuildNodeIndexMap(g);

    // 2. 构建 slots
    // Entry 输出：指向 Dialogue
    RuntimeSlot entryOut; entryOut.type = SlotType::Exec; entryOut.name = "Next";
    SLOT_ID entryOutIdx = PushSlot(g, entryOut);
    g.slotToNode[entryOutIdx] = entry.id;
    int entryIdx = GetNodeIndex(g, entry.id);
    g.nodes[entryIdx].outputsBegin = entryOutIdx;
    g.nodes[entryIdx].outputsCount = 1;

			// Dialogue 输入槽（必须有！）
    RuntimeSlot dlgIn; dlgIn.type = SlotType::Exec; dlgIn.name = "In";
    SLOT_ID dlgInIdx = PushSlot(g, dlgIn);
    g.slotToNode[dlgInIdx] = dlg.id;
    int dlgIdx = GetNodeIndex(g, dlg.id);
    g.nodes[dlgIdx].inputsBegin = dlgInIdx;
    g.nodes[dlgIdx].inputsCount = 1;

    // Dialogue 输出：3 行对白（保持不变）
    RuntimeSlot dlgOut1; dlgOut1.type = SlotType::String; dlgOut1.value = Value(std::string("你好，欢迎来到 Demo!"));
    RuntimeSlot dlgOut2; dlgOut2.type = SlotType::String; dlgOut2.value = Value(std::string("请选择分支："));
    RuntimeSlot dlgOut3; dlgOut3.type = SlotType::String; dlgOut3.value = Value(std::string("分支即将跳转..."));
    SLOT_ID dlgOutIdx1 = PushSlot(g, dlgOut1);
    SLOT_ID dlgOutIdx2 = PushSlot(g, dlgOut2);
    SLOT_ID dlgOutIdx3 = PushSlot(g, dlgOut3);
    g.slotToNode[dlgOutIdx1] = dlg.id;
    g.slotToNode[dlgOutIdx2] = dlg.id;
    g.slotToNode[dlgOutIdx3] = dlg.id;
    g.nodes[dlgIdx].outputsBegin = dlgOutIdx1;
    g.nodes[dlgIdx].outputsCount = 3;

    // Branch 输入：choice 变量
    RuntimeSlot branchIn; branchIn.type = SlotType::Bool; branchIn.value = Value(true); // true=EndA, false=EndB
    SLOT_ID branchInIdx = PushSlot(g, branchIn);
    g.slotToNode[branchInIdx] = branch.id;
    int branchIdx = GetNodeIndex(g, branch.id);
    g.nodes[branchIdx].inputsBegin = branchInIdx;
    g.nodes[branchIdx].inputsCount = 1;

    // Branch 输出：True/False
    RuntimeSlot branchTrue; branchTrue.type = SlotType::Exec; branchTrue.name = "True";
    RuntimeSlot branchFalse; branchFalse.type = SlotType::Exec; branchFalse.name = "False";
    SLOT_ID branchTrueIdx = PushSlot(g, branchTrue);
    SLOT_ID branchFalseIdx = PushSlot(g, branchFalse);
    g.slotToNode[branchTrueIdx] = branch.id;
    g.slotToNode[branchFalseIdx] = branch.id;
    g.nodes[branchIdx].outputsBegin = branchTrueIdx;
    g.nodes[branchIdx].outputsCount = 2;

    // EndA/EndB 输入
    RuntimeSlot endAIn; endAIn.type = SlotType::Exec; endAIn.name = "In";
    SLOT_ID endAInIdx = PushSlot(g, endAIn);
    g.slotToNode[endAInIdx] = endA.id;
    int endAIdx = GetNodeIndex(g, endA.id);
    g.nodes[endAIdx].inputsBegin = endAInIdx;
    g.nodes[endAIdx].inputsCount = 1;

    RuntimeSlot endBIn; endBIn.type = SlotType::Exec; endBIn.name = "In";
    SLOT_ID endBInIdx = PushSlot(g, endBIn);
    g.slotToNode[endBInIdx] = endB.id;
    int endBIdx = GetNodeIndex(g, endB.id);
    g.nodes[endBIdx].inputsBegin = endBInIdx;
    g.nodes[endBIdx].inputsCount = 1;

    // 3. 构建边
    g.edges.push_back(RuntimeEdge(entryOutIdx, dlgInIdx)); // EntryOut → DialogueIn
    g.edges.push_back(RuntimeEdge(dlgOutIdx3, branchInIdx));
    g.edges.push_back(RuntimeEdge(branchTrueIdx, endAInIdx));
    g.edges.push_back(RuntimeEdge(branchFalseIdx, endBInIdx));
    BuildEdgeMaps(g);

    // 4. 设置入口节点
    g.entryNodeId = entry.id;

    // 5. 初始化执行上下文
    RuntimeContext ctx;
    ctx.graph = &g;
    ctx.execStack.push_back(g.entryNodeId);

    // 6. 执行
    printf("=== RuntimeGraph Demo Start ===\n");
    ExecuteGraph(ctx);
    printf("=== RuntimeGraph Demo End ===\n");

    return 0;
}