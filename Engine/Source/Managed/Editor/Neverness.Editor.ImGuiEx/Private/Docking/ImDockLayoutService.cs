using Hexa.NET.ImGui;
using System.Numerics;

namespace Neverness.Editor.ImGuiEx;

/// <summary>
/// 停靠布局服务实现。
///
/// 使用 ImGui DockBuilder API 构建和管理 DockSpace 布局。
/// 支持默认布局构建、窗口停靠绑定、INI 持久化。
/// </summary>
public sealed class ImDockLayoutService : IImDockLayoutService
{
    private uint m_DockspaceId;
    private bool m_LayoutBuilt;
    private readonly Dictionary<string, uint> m_ZoneNodes = new(StringComparer.Ordinal);

    /// <inheritdoc />
    public uint MainDockSpaceId => m_DockspaceId;

    /// <inheritdoc />
    public bool IsLayoutBuilt => m_LayoutBuilt;

    /// <inheritdoc />
    public void Initialize(uint dockspaceId)
    {
        m_DockspaceId = dockspaceId;
    }

    /// <inheritdoc />
    public void BuildDefaultLayout(IReadOnlyList<DockZone> zones)
    {
        if (m_LayoutBuilt) return;
        if (zones.Count == 0) return;
        if (m_DockspaceId == 0)
            throw new InvalidOperationException("请先调用 Initialize 设置 DockSpace ID。");

        // 添加根节点
        var rootNode = DockLayoutBuilder.AddDockSpace(m_DockspaceId, ImGuiDockNodeFlags.None);

        var viewport = ImGui.GetMainViewport();
        DockLayoutBuilder.SetNodeSize(rootNode, viewport.WorkSize);

        // 线性分割布局
        DockLayoutBuilder.BuildLinearLayout(rootNode, zones, m_ZoneNodes);

        // 完成构建
        DockLayoutBuilder.Finish(m_DockspaceId);
        m_LayoutBuilt = true;
    }

    /// <inheritdoc />
    public void DockWindow(string windowImGuiName, string zoneName)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(windowImGuiName);
        ArgumentException.ThrowIfNullOrWhiteSpace(zoneName);

        if (m_ZoneNodes.TryGetValue(zoneName, out var nodeId))
        {
            DockLayoutBuilder.DockWindow(windowImGuiName, nodeId);
        }
    }

    /// <inheritdoc />
    public void SaveLayout(string filePath = "imgui_layout.ini")
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(filePath);
        ImGui.SaveIniSettingsToDisk(filePath);
    }

    /// <inheritdoc />
    public void RestoreLayout(string filePath = "imgui_layout.ini")
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(filePath);
        ImGui.LoadIniSettingsFromDisk(filePath);
    }
}
