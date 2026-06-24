namespace Neverness.Runtime.Settings.Attributes;

/// <summary>
/// 标记字段为只读。
/// 设置 UI 中会显示值但不允许编辑。
/// </summary>
[AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
public sealed class SettingReadOnlyAttribute : Attribute
{
}
