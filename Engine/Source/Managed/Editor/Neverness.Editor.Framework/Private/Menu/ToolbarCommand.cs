namespace Neverness.Editor.Framework.Private.Menu;

/// <summary>
/// 工具栏按钮描述符。
/// </summary>
internal readonly record struct ToolbarCommand(
    string Id,
    string Icon,
    string Tooltip = "",
    string CommandId = "",
    int SortOrder = 0
);
