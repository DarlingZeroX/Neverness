namespace Neverness.Runtime.Settings;

/// <summary>
/// 设置表描述符——描述一个 SettingsTable 的所有字段元数据。
/// Phase 1 手写实现，Phase 2 由 Source Generator 自动生成。
/// </summary>
public interface ISettingsDescriptor
{
    /// <summary>关联的 SettingsTable 类型。</summary>
    Type TableType { get; }

    /// <summary>设置表 ID。</summary>
    string TableId { get; }

    /// <summary>显示名称。</summary>
    string DisplayName { get; }

    /// <summary>设置范围（Project 或 User）。</summary>
    SettingsScope Scope { get; }

    /// <summary>分类（用于 TreeView 分组）。</summary>
    string? Category { get; }

    /// <summary>图标（可选）。</summary>
    string? Icon { get; }

    /// <summary>所有字段描述（已排序）。</summary>
    IReadOnlyList<FieldDescriptor> Fields { get; }

    /// <summary>创建默认实例。</summary>
    SettingsTable CreateDefault();
}
