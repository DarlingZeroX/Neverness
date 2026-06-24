namespace Neverness.Runtime.Settings.Attributes;

/// <summary>
/// 标记字段在设置 UI 中隐藏。
/// 字段仍会被序列化，但不显示在设置面板中。
/// </summary>
[AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
public sealed class SettingHiddenAttribute : Attribute
{
}
