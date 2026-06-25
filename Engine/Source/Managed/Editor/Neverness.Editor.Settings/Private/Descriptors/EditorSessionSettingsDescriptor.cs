using Neverness.Runtime.Settings;

namespace Neverness.Editor.Settings.Private.Descriptors;

/// <summary>
/// 编辑器会话状态描述符——手写，Phase 2 由 Source Generator 替代。
/// </summary>
public sealed class EditorSessionSettingsDescriptor : ISettingsDescriptor
{
    public static readonly EditorSessionSettingsDescriptor Instance = new();

    public Type TableType => typeof(EditorSessionSettings);
    public string TableId => "editorSession";
    public string DisplayName => "会话状态";
    public SettingsScope Scope => SettingsScope.Project;
    public string? Category => "编辑器";
    public string? Icon => null;

    public IReadOnlyList<FieldDescriptor> Fields => new[]
    {
        new FieldDescriptor
        {
            Name = "LastOpenedScene",
            DisplayName = "上次打开的场景",
            FieldType = FieldType.String,
            ValueType = typeof(string),
            Order = 0,
            Getter = t => ((EditorSessionSettings)t).LastOpenedScene,
            Setter = (t, v) => ((EditorSessionSettings)t).LastOpenedScene = (string?)v,
        },
        new FieldDescriptor
        {
            Name = "LastSelectedEntityHandle",
            DisplayName = "上次选中的实体",
            FieldType = FieldType.Int,  // ulong 用 Int 展示（句柄值）
            ValueType = typeof(ulong),
            Order = 1,
            Getter = t => ((EditorSessionSettings)t).LastSelectedEntityHandle,
            Setter = (t, v) => ((EditorSessionSettings)t).LastSelectedEntityHandle = (ulong)v!,
        },
    };

    public SettingsTable CreateDefault() => new EditorSessionSettings();
}
