namespace Neverness.Runtime.Settings;

/// <summary>
/// 字段元数据——描述一个可设置字段的所有信息。
/// 用于 SettingsFieldRenderer 自动生成 UI 控件。
/// </summary>
public sealed class FieldDescriptor
{
    /// <summary>字段名（属性名，如 "VSync"）。</summary>
    public required string Name { get; init; }

    /// <summary>显示名称（如 "垂直同步"）。</summary>
    public required string DisplayName { get; init; }

    /// <summary>字段描述/提示文本。</summary>
    public string? Description { get; init; }

    /// <summary>字段类型（决定 UI 控件）。</summary>
    public required FieldType FieldType { get; init; }

    /// <summary>实际值类型（如 typeof(bool)）。</summary>
    public required Type ValueType { get; init; }

    /// <summary>范围下限（FieldType 为 Int/Float 时有效）。</summary>
    public float Min { get; init; }

    /// <summary>范围上限（FieldType 为 Int/Float 时有效）。</summary>
    public float Max { get; init; }

    /// <summary>步进值（0 表示连续）。</summary>
    public float Step { get; init; }

    /// <summary>是否有范围约束。</summary>
    public bool HasRange => Min != 0f || Max != 0f;

    /// <summary>是否只读。</summary>
    public bool IsReadOnly { get; init; }

    /// <summary>是否在 UI 中隐藏。</summary>
    public bool IsHidden { get; init; }

    /// <summary>字段分组名（如 "高级"）。</summary>
    public string? Group { get; init; }

    /// <summary>排序权重（越小越靠前）。</summary>
    public int Order { get; init; }

    /// <summary>枚举类型（FieldType 为 Enum 时有效）。</summary>
    public Type? EnumType { get; init; }

    /// <summary>枚举值列表（FieldType 为 Enum 时有效，预计算缓存）。</summary>
    public Array? EnumValues { get; init; }

    /// <summary>枚举显示名称列表（与 EnumValues 一一对应）。</summary>
    public string[]? EnumDisplayNames { get; init; }

    /// <summary>值读取器：从 SettingsTable 实例读取字段值。</summary>
    public Func<SettingsTable, object?>? Getter { get; init; }

    /// <summary>值设置器：向 SettingsTable 实例写入字段值。</summary>
    public Action<SettingsTable, object?>? Setter { get; init; }
}
