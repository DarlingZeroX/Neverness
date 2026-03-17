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
    RuntimeSlot entryOut; entryOut.id = MakeSlotId(entry.id, 0); entryOut.type = SlotType::Exec; entryOut.name = "Next";
    uint32_t entryOutIdx = PushSlot(g, entryOut);
    g.slotToNode[entryOut.id] = entry.id;
    int entryIdx = GetNodeIndex(g, entry.id);
    g.nodes[entryIdx].outputsBegin = entryOutIdx;
    g.nodes[entryIdx].outputsCount = 1;

			// Dialogue 输入槽（必须有！）
    RuntimeSlot dlgIn; dlgIn.id = MakeSlotId(dlg.id, 0); dlgIn.type = SlotType::Exec; dlgIn.name = "In";
    uint32_t dlgInIdx = PushSlot(g, dlgIn);
    g.slotToNode[dlgIn.id] = dlg.id;
    int dlgIdx = GetNodeIndex(g, dlg.id);
    g.nodes[dlgIdx].inputsBegin = dlgInIdx;
    g.nodes[dlgIdx].inputsCount = 1;

    // Dialogue 输出：3 行对白（保持不变）
    RuntimeSlot dlgOut1; dlgOut1.id = MakeSlotId(dlg.id, 1); dlgOut1.type = SlotType::String; dlgOut1.value = Value(std::string("你好，欢迎来到 Demo!"));
    RuntimeSlot dlgOut2; dlgOut2.id = MakeSlotId(dlg.id, 2); dlgOut2.type = SlotType::String; dlgOut2.value = Value(std::string("请选择分支："));
    RuntimeSlot dlgOut3; dlgOut3.id = MakeSlotId(dlg.id, 3); dlgOut3.type = SlotType::String; dlgOut3.value = Value(std::string("分支即将跳转..."));
    uint32_t dlgOutIdx1 = PushSlot(g, dlgOut1);
    uint32_t dlgOutIdx2 = PushSlot(g, dlgOut2);
    uint32_t dlgOutIdx3 = PushSlot(g, dlgOut3);
    g.slotToNode[dlgOut1.id] = dlg.id;
    g.slotToNode[dlgOut2.id] = dlg.id;
    g.slotToNode[dlgOut3.id] = dlg.id;
    g.nodes[dlgIdx].outputsBegin = dlgOutIdx1;
    g.nodes[dlgIdx].outputsCount = 3;

    // Branch 输入：choice 变量
    RuntimeSlot branchIn; branchIn.id = MakeSlotId(branch.id, 0); branchIn.type = SlotType::Bool; branchIn.value = Value(true); // true=EndA, false=EndB
    uint32_t branchInIdx = PushSlot(g, branchIn);
    g.slotToNode[branchIn.id] = branch.id;
    int branchIdx = GetNodeIndex(g, branch.id);
    g.nodes[branchIdx].inputsBegin = branchInIdx;
    g.nodes[branchIdx].inputsCount = 1;

    // Branch 输出：True/False
    RuntimeSlot branchTrue; branchTrue.id = MakeSlotId(branch.id, 1); branchTrue.type = SlotType::Exec; branchTrue.name = "True";
    RuntimeSlot branchFalse; branchFalse.id = MakeSlotId(branch.id, 2); branchFalse.type = SlotType::Exec; branchFalse.name = "False";
    uint32_t branchTrueIdx = PushSlot(g, branchTrue);
    uint32_t branchFalseIdx = PushSlot(g, branchFalse);
    g.slotToNode[branchTrue.id] = branch.id;
    g.slotToNode[branchFalse.id] = branch.id;
    g.nodes[branchIdx].outputsBegin = branchTrueIdx;
    g.nodes[branchIdx].outputsCount = 2;

    // EndA/EndB 输入
    RuntimeSlot endAIn; endAIn.id = MakeSlotId(endA.id, 0); endAIn.type = SlotType::Exec; endAIn.name = "In";
    uint32_t endAInIdx = PushSlot(g, endAIn);
    g.slotToNode[endAIn.id] = endA.id;
    int endAIdx = GetNodeIndex(g, endA.id);
    g.nodes[endAIdx].inputsBegin = endAInIdx;
    g.nodes[endAIdx].inputsCount = 1;

    RuntimeSlot endBIn; endBIn.id = MakeSlotId(endB.id, 0); endBIn.type = SlotType::Exec; endBIn.name = "In";
    uint32_t endBInIdx = PushSlot(g, endBIn);
    g.slotToNode[endBIn.id] = endB.id;
    int endBIdx = GetNodeIndex(g, endB.id);
    g.nodes[endBIdx].inputsBegin = endBInIdx;
    g.nodes[endBIdx].inputsCount = 1;

    // 3. 构建边
    g.edges.push_back(RuntimeEdge(entryOut.id, dlgIn.id)); // 修正为 EntryOut → DialogueIn
    g.edges.push_back(RuntimeEdge(dlgOut3.id, branchIn.id));
    g.edges.push_back(RuntimeEdge(branchTrue.id, endAIn.id));
    g.edges.push_back(RuntimeEdge(branchFalse.id, endBIn.id));
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