using Neverness.Runtime.Settings;
using Neverness.Runtime.Settings.Attributes;

namespace Neverness.Runtime.Settings.Tests;

/// <summary>
/// 测试用设置表——覆盖常见字段类型。
/// </summary>
[SettingTable("test", "测试设置", Category = "测试")]
public sealed class TestSettings : SettingsTable
{
    public override string TableId => "test";
    public override string DisplayName => "测试设置";
    public override string? Category => "测试";

    [SettingField(DisplayName = "布尔值")]
    public bool BoolProp { get; set; } = true;

    [SettingField(DisplayName = "整数")]
    [SettingRange(0, 100)]
    public int IntProp { get; set; } = 42;

    [SettingField(DisplayName = "浮点数")]
    [SettingRange(0f, 1f)]
    public float FloatProp { get; set; } = 0.5f;

    [SettingField(DisplayName = "字符串")]
    public string StringProp { get; set; } = "hello";

    [SettingField(DisplayName = "枚举")]
    public TestEnum EnumProp { get; set; } = TestEnum.Second;

    [SettingHidden]
    public int HiddenProp { get; set; } = 999;

    [SettingReadOnly]
    public int ReadOnlyProp { get; set; } = 100;
}

/// <summary>测试用枚举。</summary>
public enum TestEnum { First, Second, Third }

/// <summary>
/// 最小设置表——只含一个字段，用于边界测试。
/// </summary>
[SettingTable("minimal", "最小")]
public sealed class MinimalSettings : SettingsTable
{
    public override string TableId => "minimal";
    public override string DisplayName => "最小";

    [SettingField(DisplayName = "值")]
    public int Value { get; set; } = 0;
}
