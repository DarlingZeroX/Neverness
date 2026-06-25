using Neverness.Runtime.Settings;
using Neverness.Runtime.Settings.Attributes;

namespace Neverness.Editor.Settings.Private.Descriptors;

/// <summary>
/// 编辑器会话状态——记录上次打开的场景、窗口布局等。
/// Scope = Project，每个项目独立保存。
/// 使用 SetProperty() 触发 PropertyChanged，由 SettingsModule 订阅后自动持久化。
///
/// 存储路径：/projectSettings/Settings/editorSession.json
/// </summary>
[SettingTable("editorSession", "会话状态", Scope = SettingsScope.Project, Category = "编辑器")]
public sealed class EditorSessionSettings : SettingsTable
{
    public override string TableId => "editorSession";
    public override string DisplayName => "会话状态";
    public override SettingsScope Scope => SettingsScope.Project;
    public override string? Category => "编辑器";

    private string? _lastOpenedScene;
    private ulong _lastSelectedEntityHandle;

    /// <summary>上次打开的场景 VFS 路径（null = 首次启动，无历史）。</summary>
    [SettingField(DisplayName = "上次打开的场景")]
    public string? LastOpenedScene
    {
        get => _lastOpenedScene;
        set => SetProperty(ref _lastOpenedScene, value);
    }

    /// <summary>上次选中的实体句柄值（0 = 无选中）。</summary>
    [SettingField(DisplayName = "上次选中的实体")]
    public ulong LastSelectedEntityHandle
    {
        get => _lastSelectedEntityHandle;
        set => SetProperty(ref _lastSelectedEntityHandle, value);
    }
}
