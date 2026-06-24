namespace Neverness.Runtime.Settings.Descriptors;

/// <summary>
/// 图形设置描述符——手写，Phase 2 由 Source Generator 替代。
/// </summary>
public sealed class GraphicsSettingsDescriptor : ISettingsDescriptor
{
    public static readonly GraphicsSettingsDescriptor Instance = new();

    public Type TableType => typeof(GraphicsSettings);
    public string TableId => "graphics";
    public string DisplayName => "图形";
    public SettingsScope Scope => SettingsScope.Project;
    public string? Category => "渲染";
    public string? Icon => null;

    public IReadOnlyList<FieldDescriptor> Fields => new[]
    {
        new FieldDescriptor
        {
            Name = "VSync",
            DisplayName = "垂直同步",
            FieldType = FieldType.Bool,
            ValueType = typeof(bool),
            Order = 0,
            Getter = t => ((GraphicsSettings)t).VSync,
            Setter = (t, v) => ((GraphicsSettings)t).VSync = (bool)v!,
        },
        new FieldDescriptor
        {
            Name = "FPSLimit",
            DisplayName = "帧率限制",
            FieldType = FieldType.Int,
            ValueType = typeof(int),
            Min = 30,
            Max = 240,
            Step = 1,
            Order = 1,
            Getter = t => ((GraphicsSettings)t).FPSLimit,
            Setter = (t, v) => ((GraphicsSettings)t).FPSLimit = (int)v!,
        },
        new FieldDescriptor
        {
            Name = "RenderScale",
            DisplayName = "渲染缩放",
            FieldType = FieldType.Float,
            ValueType = typeof(float),
            Min = 0.5f,
            Max = 2.0f,
            Step = 0.05f,
            Order = 2,
            Getter = t => ((GraphicsSettings)t).RenderScale,
            Setter = (t, v) => ((GraphicsSettings)t).RenderScale = (float)v!,
        },
        new FieldDescriptor
        {
            Name = "Fullscreen",
            DisplayName = "全屏模式",
            FieldType = FieldType.Enum,
            ValueType = typeof(FullscreenMode),
            EnumType = typeof(FullscreenMode),
            EnumValues = Enum.GetValues<FullscreenMode>(),
            EnumDisplayNames = Enum.GetNames<FullscreenMode>(),
            Order = 3,
            Getter = t => ((GraphicsSettings)t).Fullscreen,
            Setter = (t, v) => ((GraphicsSettings)t).Fullscreen = (FullscreenMode)v!,
        },
    };

    public SettingsTable CreateDefault() => new GraphicsSettings();
}
