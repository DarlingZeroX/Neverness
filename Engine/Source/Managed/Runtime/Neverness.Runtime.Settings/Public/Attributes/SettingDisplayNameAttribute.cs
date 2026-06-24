namespace Neverness.Runtime.Settings.Attributes;

/// <summary>
/// 为字段、属性或类指定显示名称。
/// 与 SettingField.DisplayName 不同，此 Attribute 可独立使用。
/// </summary>
[AttributeUsage(AttributeTargets.Field | AttributeTargets.Property | AttributeTargets.Class)]
public sealed class SettingDisplayNameAttribute : Attribute
{
    /// <summary>显示名称。</summary>
    public string Name { get; }

    public SettingDisplayNameAttribute(string name)
    {
        Name = name;
    }
}
