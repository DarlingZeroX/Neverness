namespace Neverness.Editor.Framework.Public.Services;

/// <summary>
/// Dock 布局服务接口——由 Frontend 模块实现。
///
/// 职责：
/// - 保存/恢复编辑器布局
/// - 添加/移除面板到 Dock
/// - 重置为默认布局
///
/// 设计原则：
/// - Core 只看接口，不依赖具体 UI 框架
/// - AvaloniaFrontend 和 ImGuiFrontend 各自实现
/// </summary>
public interface IDockService
{
    /// <summary>保存当前布局到文件。</summary>
    bool SaveLayout(string filePath);

    /// <summary>从文件恢复布局。</summary>
    bool LoadLayout(string filePath);

    /// <summary>重置为默认布局。</summary>
    void ResetToDefault();

    /// <summary>添加面板到指定区域。</summary>
    void AddPanel(string panelId, DockArea area);

    /// <summary>移除面板。</summary>
    void RemovePanel(string panelId);

    /// <summary>面板是否已停靠。</summary>
    bool IsPanelDocked(string panelId);

    /// <summary>显示/隐藏面板。</summary>
    void SetPanelVisible(string panelId, bool visible);
}

/// <summary>
/// Dock 区域枚举。
/// </summary>
public enum DockArea
{
    /// <summary>左侧。</summary>
    Left,

    /// <summary>右侧。</summary>
    Right,

    /// <summary>底部。</summary>
    Bottom,

    /// <summary>中央（文档区域）。</summary>
    Center,

    /// <summary>浮动窗口。</summary>
    Floating
}
