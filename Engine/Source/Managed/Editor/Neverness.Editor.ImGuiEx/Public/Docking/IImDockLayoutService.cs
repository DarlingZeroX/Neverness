using Hexa.NET.ImGui;

namespace Neverness.Editor.ImGuiEx;

/// <summary>
/// 停靠布局服务接口。
///
/// 封装 ImGui DockBuilder API，提供默认布局构建、窗口停靠、布局持久化。
/// </summary>
public interface IImDockLayoutService
{
    /// <summary>主 DockSpace 的 ID。</summary>
    uint MainDockSpaceId { get; }

    /// <summary>默认布局是否已构建。</summary>
    bool IsLayoutBuilt { get; }

    /// <summary>初始化服务，设置 DockSpace ID。</summary>
    void Initialize(uint dockspaceId);

    /// <summary>构建默认布局（使用 DockBuilder API 分割节点）。</summary>
    void BuildDefaultLayout(IReadOnlyList<DockZone> zones);

    /// <summary>将指定 ImGui 窗口名停靠到目标区域。</summary>
    void DockWindow(string windowImGuiName, string zoneName);

    /// <summary>保存布局到 INI 文件。</summary>
    void SaveLayout(string filePath = "imgui_layout.ini");

    /// <summary>从 INI 文件恢复布局。</summary>
    void RestoreLayout(string filePath = "imgui_layout.ini");
}
