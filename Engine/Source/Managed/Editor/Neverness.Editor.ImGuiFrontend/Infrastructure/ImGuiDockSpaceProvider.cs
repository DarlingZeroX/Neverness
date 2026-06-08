using Hexa.NET.ImGui;

namespace Neverness.Editor.ImGuiFrontend.Infrastructure;

/// <summary>
/// ImGui 停靠空间管理器——封装 ImGuiWindowClass 和 DockId 的创建与管理。
/// 从 PanelManager 的构造函数中拆分出来。
/// </summary>
public sealed class ImGuiDockSpaceProvider
{
    /// <summary>全局单例。</summary>
    public static ImGuiDockSpaceProvider Instance { get; } = new();

    private ImGuiWindowClass _topLevelClass;
    private readonly uint _topLevelDockId;

    private ImGuiDockSpaceProvider()
    {
        _topLevelDockId = ImGui.GetID("TopLevelDockSpace");

        _topLevelClass = new ImGuiWindowClass
        {
            ClassId = _topLevelDockId,
            DockingAllowUnclassed = 1
        };
    }

    /// <summary>获取顶级停靠窗口类。</summary>
    public ref ImGuiWindowClass GetTopLevelWindowClass() => ref _topLevelClass;

    /// <summary>获取顶级停靠 ID。</summary>
    public uint GetTopLevelDockId() => _topLevelDockId;

    /// <summary>创建停靠空间。</summary>
    public void DockSpace(uint dockId)
    {
        ImGui.DockSpace(dockId);
    }
}
