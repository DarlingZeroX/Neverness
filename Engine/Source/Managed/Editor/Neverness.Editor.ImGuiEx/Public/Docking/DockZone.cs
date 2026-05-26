using Hexa.NET.ImGui;

namespace Neverness.Editor.ImGuiEx;

/// <summary>
/// 停靠区域描述符。定义 DockBuilder 分割方向和比例。
/// </summary>
/// <param name="Name">区域名称（用于 DockWindow 绑定）。</param>
/// <param name="Direction">分割方向。</param>
/// <param name="Ratio">新节点占父节点的比例 (0..1)。</param>
public readonly record struct DockZone(
    string Name,
    ImGuiDir Direction,
    float Ratio);
