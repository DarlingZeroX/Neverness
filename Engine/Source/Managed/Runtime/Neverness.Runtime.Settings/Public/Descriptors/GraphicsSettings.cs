using Neverness.Runtime.Settings.Attributes;

namespace Neverness.Runtime.Settings.Descriptors;

/// <summary>
/// 图形设置——运行时 Project Settings。
/// </summary>
[SettingTable("graphics", "图形", Scope = SettingsScope.Project, Category = "渲染")]
public sealed class GraphicsSettings : SettingsTable
{
    public override string TableId => "graphics";
    public override string DisplayName => "图形";
    public override SettingsScope Scope => SettingsScope.Project;
    public override string? Category => "渲染";

    [SettingField(DisplayName = "垂直同步")]
    public bool VSync { get; set; } = true;

    [SettingField(DisplayName = "帧率限制")]
    [SettingRange(30, 240)]
    public int FPSLimit { get; set; } = 60;

    [SettingField(DisplayName = "渲染缩放")]
    [SettingRange(0.5f, 2.0f)]
    public float RenderScale { get; set; } = 1.0f;

    [SettingField(DisplayName = "全屏模式")]
    public FullscreenMode Fullscreen { get; set; } = FullscreenMode.Windowed;

    [SettingHidden]
    public int InternalVersion { get; set; } = 1;
}

/// <summary>全屏模式枚举。</summary>
public enum FullscreenMode
{
    Windowed,
    Fullscreen,
    Borderless
}
