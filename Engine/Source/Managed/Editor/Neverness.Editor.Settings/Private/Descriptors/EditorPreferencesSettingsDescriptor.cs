using Neverness.Runtime.Settings;

namespace Neverness.Editor.Settings.Private.Descriptors;

/// <summary>
/// 编辑器偏好设置描述符——手写，Phase 2 由 Source Generator 替代。
/// </summary>
public sealed class EditorPreferencesSettingsDescriptor : ISettingsDescriptor
{
    public static readonly EditorPreferencesSettingsDescriptor Instance = new();

    public Type TableType => typeof(EditorPreferencesSettings);
    public string TableId => "editorPreferences";
    public string DisplayName => "偏好设置";
    public SettingsScope Scope => SettingsScope.User;
    public string? Category => "编辑器";
    public string? Icon => null;

    public IReadOnlyList<FieldDescriptor> Fields => new[]
    {
        new FieldDescriptor
        {
            Name = "PreferredIDE",
            DisplayName = "首选 IDE",
            FieldType = FieldType.Enum,
            ValueType = typeof(IDEPreference),
            EnumType = typeof(IDEPreference),
            EnumValues = Enum.GetValues<IDEPreference>(),
            EnumDisplayNames = Enum.GetNames<IDEPreference>(),
            Order = 0,
            Getter = t => ((EditorPreferencesSettings)t).PreferredIDE,
            Setter = (t, v) => ((EditorPreferencesSettings)t).PreferredIDE = (IDEPreference)v!,
        },
        new FieldDescriptor
        {
            Name = "FontSize",
            DisplayName = "字体大小",
            FieldType = FieldType.Int,
            ValueType = typeof(int),
            Min = 10, Max = 24, Step = 1,
            Order = 1,
            Getter = t => ((EditorPreferencesSettings)t).FontSize,
            Setter = (t, v) => ((EditorPreferencesSettings)t).FontSize = (int)v!,
        },
        new FieldDescriptor
        {
            Name = "ShowLineNumbers",
            DisplayName = "显示行号",
            FieldType = FieldType.Bool,
            ValueType = typeof(bool),
            Order = 2,
            Getter = t => ((EditorPreferencesSettings)t).ShowLineNumbers,
            Setter = (t, v) => ((EditorPreferencesSettings)t).ShowLineNumbers = (bool)v!,
        },
        new FieldDescriptor
        {
            Name = "AutoSave",
            DisplayName = "自动保存",
            FieldType = FieldType.Bool,
            ValueType = typeof(bool),
            Order = 3,
            Getter = t => ((EditorPreferencesSettings)t).AutoSave,
            Setter = (t, v) => ((EditorPreferencesSettings)t).AutoSave = (bool)v!,
        },
        new FieldDescriptor
        {
            Name = "AutoSaveInterval",
            DisplayName = "自动保存间隔（秒）",
            FieldType = FieldType.Int,
            ValueType = typeof(int),
            Min = 30, Max = 600, Step = 10,
            Order = 4,
            Getter = t => ((EditorPreferencesSettings)t).AutoSaveInterval,
            Setter = (t, v) => ((EditorPreferencesSettings)t).AutoSaveInterval = (int)v!,
        },
        new FieldDescriptor
        {
            Name = "CsEditorMode",
            DisplayName = ".cs 文件编辑方式",
            FieldType = FieldType.Enum,
            ValueType = typeof(CsEditorMode),
            EnumType = typeof(CsEditorMode),
            EnumValues = Enum.GetValues<CsEditorMode>(),
            EnumDisplayNames = Enum.GetNames<CsEditorMode>(),
            Order = 5,
            Getter = t => ((EditorPreferencesSettings)t).CsEditorMode,
            Setter = (t, v) => ((EditorPreferencesSettings)t).CsEditorMode = (CsEditorMode)v!,
        },
    };

    public SettingsTable CreateDefault() => new EditorPreferencesSettings();
}
