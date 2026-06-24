namespace Neverness.Runtime.Settings.Attributes;

/// <summary>
/// 为数值字段指定范围约束。
/// 设置 UI 中会渲染为 Slider + NumericUpDown。
/// </summary>
[AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
public sealed class SettingRangeAttribute : Attribute
{
    /// <summary>范围下限。</summary>
    public float Min { get; }

    /// <summary>范围上限。</summary>
    public float Max { get; }

    /// <summary>步进值（0 表示连续，默认 0）。</summary>
    public float Step { get; init; }

    /// <summary>建立范围约束。</summary>
    /// <param name="min">下限。</param>
    /// <param name="max">上限。</param>
    public SettingRangeAttribute(float min, float max)
    {
        Min = min;
        Max = max;
    }
}
