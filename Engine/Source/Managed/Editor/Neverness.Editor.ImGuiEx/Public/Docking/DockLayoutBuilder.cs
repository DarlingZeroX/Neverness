using Hexa.NET.ImGui;
using System.Numerics;

namespace Neverness.Editor.ImGuiEx;

/// <summary>
/// DockBuilder API 封装。提供便捷的 DockSpace 分割和布局构建。
///
/// 封装 ImGuiP 内部 DockBuilder 函数：
/// - AddNode / RemoveNode
/// - SplitNode
/// - SetNodeSize / SetNodePos
/// - DockWindow
/// - Finish
/// </summary>
public static class DockLayoutBuilder
{
    /// <summary>添加或获取 DockSpace 节点。</summary>
    public static uint AddDockSpace(uint dockspaceId, ImGuiDockNodeFlags flags = ImGuiDockNodeFlags.None)
    {
        return ImGuiP.DockBuilderAddNode(dockspaceId, flags);
    }

    /// <summary>设置节点大小。</summary>
    public static void SetNodeSize(uint nodeId, Vector2 size)
    {
        ImGuiP.DockBuilderSetNodeSize(nodeId, size);
    }

    /// <summary>分割节点，返回两个子节点 ID。</summary>
    /// <param name="nodeId">要分割的节点。</param>
    /// <param name="direction">分割方向。</param>
    /// <param name="ratio">新节点占父节点的比例 (0..1)。</param>
    /// <param name="outIdAtDir">输出：分割方向侧的节点 ID。</param>
    /// <param name="outIdAtOpposite">输出：相反方向侧的节点 ID。</param>
    public static void SplitNode(
        uint nodeId,
        ImGuiDir direction,
        float ratio,
        out uint outIdAtDir,
        out uint outIdAtOpposite)
    {
        unsafe
        {
            uint dirId = 0;
            uint oppId = 0;
            ImGuiP.DockBuilderSplitNode(nodeId, direction, ratio, &dirId, &oppId);
            outIdAtDir = dirId;
            outIdAtOpposite = oppId;
        }
    }

    /// <summary>将 ImGui 窗口名停靠到指定节点。</summary>
    public static void DockWindow(string windowImGuiName, uint nodeId)
    {
        ImGuiP.DockBuilderDockWindow(windowImGuiName, nodeId);
    }

    /// <summary>完成 DockBuilder 布局构建。</summary>
    public static void Finish(uint nodeId)
    {
        ImGuiP.DockBuilderFinish(nodeId);
    }

    /// <summary>移除节点及其所有子节点。</summary>
    public static void RemoveNode(uint nodeId)
    {
        ImGuiP.DockBuilderRemoveNode(nodeId);
    }

    /// <summary>
    /// 构建线性分割布局。
    /// 按 zones 列表的顺序依次分割，每次分割方向和比例由 DockZone 定义。
    /// </summary>
    public static void BuildLinearLayout(
        uint rootNodeId,
        IReadOnlyList<DockZone> zones,
        Dictionary<string, uint> outputZoneNodes)
    {
        if (zones.Count == 0) return;

        uint currentNode = rootNodeId;

        for (int i = 0; i < zones.Count; i++)
        {
            var zone = zones[i];
            SplitNode(currentNode, zone.Direction, zone.Ratio,
                out uint newZoneNode, out uint remainingNode);

            outputZoneNodes[zone.Name] = newZoneNode;
            currentNode = remainingNode;
        }

        // 最后剩余的节点也记录（作为中央区域或最后一区）
        if (zones.Count > 0)
        {
            outputZoneNodes.TryAdd("_remainder", currentNode);
        }
    }
}
