using Neverness.Runtime.Settings;
using Neverness.Runtime.Settings.Attributes;

namespace Neverness.Editor.Settings.Private.Descriptors;

/// <summary>
/// 编辑器偏好设置——吸收旧 IPreferencesService。
/// </summary>
[SettingTable("editorPreferences", "偏好设置", Scope = SettingsScope.User, Category = "编辑器")]
public sealed class EditorPreferencesSettings : SettingsTable
{
    public override string TableId => "editorPreferences";
    public override string DisplayName => "偏好设置";
    public override SettingsScope Scope => SettingsScope.User;
    public override string? Category => "编辑器";

    [SettingField(DisplayName = "首选 IDE")]
    public IDEPreference PreferredIDE { get; set; } = IDEPreference.VisualStudio;

    [SettingField(DisplayName = "字体大小")]
    [SettingRange(10, 24)]
    public int FontSize { get; set; } = 14;

    [SettingField(DisplayName = "显示行号")]
    public bool ShowLineNumbers { get; set; } = true;

    [SettingField(DisplayName = "自动保存")]
    public bool AutoSave { get; set; } = true;

    [SettingField(DisplayName = "自动保存间隔（秒）")]
    [SettingRange(30, 600)]
    public int AutoSaveInterval { get; set; } = 120;

    [SettingField(DisplayName = ".cs 文件编辑方式")]
    public CsEditorMode CsEditorMode { get; set; } = CsEditorMode.ExternalIDE;
}

/// <summary>IDE 偏好枚举。</summary>
public enum IDEPreference
{
    VisualStudio,
    VSCode
}

/// <summary>.cs 文件编辑方式枚举。</summary>
public enum CsEditorMode
{
    /// <summary>外部 IDE（VS / VSCode）。</summary>
    ExternalIDE,

    /// <summary>编辑器内联代码编辑器。</summary>
    InlineEditor
}
