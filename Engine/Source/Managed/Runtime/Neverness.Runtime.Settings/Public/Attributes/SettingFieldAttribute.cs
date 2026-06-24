namespace Neverness.Runtime.Settings.Attributes;

/// <summary>
/// 标记字段或属性为可设置字段。
/// 未标记的字段不会出现在设置 UI 中，也不会被序列化。
/// </summary>
[AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
public sealed class SettingFieldAttribute : Attribute
{
    /// <summary>字段显示名称（覆盖字段名）。</summary>
    public string? DisplayName { get; init; }

    /// <summary>字段描述/提示文本。</summary>
    public string? Description { get; init; }

    /// <summary>字段分组名（如 "高级"、"基础"）。</summary>
    public string? Group { get; init; }

    /// <summary>字段排序权重（越小越靠前，默认 0）。</summary>
    public int Order { get; init; }
}
