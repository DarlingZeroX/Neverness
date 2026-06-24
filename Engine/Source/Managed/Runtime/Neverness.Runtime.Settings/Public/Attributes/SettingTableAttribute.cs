namespace Neverness.Runtime.Settings.Attributes;

/// <summary>
/// 标记类为设置表。
/// 系统通过此 Attribute 发现和注册设置表。
/// </summary>
[AttributeUsage(AttributeTargets.Class, Inherited = false)]
public sealed class SettingTableAttribute : Attribute
{
    /// <summary>设置表唯一 ID（如 "graphics"）。</summary>
    public string TableId { get; }

    /// <summary>设置表显示名称（如 "图形"）。</summary>
    public string DisplayName { get; }

    /// <summary>设置范围（Project 或 User）。</summary>
    public SettingsScope Scope { get; init; } = SettingsScope.Project;

    /// <summary>分类（用于 TreeView 分组，如 "渲染"、"音频"）。</summary>
    public string? Category { get; init; }

    /// <summary>设置表图标（可选，用于 TreeView）。</summary>
    public string? Icon { get; init; }

    /// <summary>创建 SettingTableAttribute。</summary>
    /// <param name="tableId">设置表唯一 ID。</param>
    /// <param name="displayName">显示名称。</param>
    public SettingTableAttribute(string tableId, string displayName)
    {
        TableId = tableId;
        DisplayName = displayName;
    }
}
