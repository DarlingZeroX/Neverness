namespace Neverness.Editor.Framework.Public;

/// <summary>
/// 菜单项描述符——路径式注册的入口数据。
/// 使用 readonly record struct 避免 GC，NativeAOT 友好。
/// </summary>
/// <param name="Path">路径，如 "File/New Scene"、"Window/Rendering/Shader Debugger"。</param>
/// <param name="Command">绑定的命令（null = 分隔符或纯子菜单）。</param>
/// <param name="CommandId">命令 ID 字符串（当 Command 为 null 时，通过 EditorMenuRegistry 按此 ID 查找命令）。</param>
/// <param name="Icon">图标（Emoji 字符）。</param>
/// <param name="Shortcut">快捷键字符串，如 "Ctrl+N"。</param>
/// <param name="SortOrder">同级排序权重（升序）。</param>
/// <param name="Group">分组名（用于 Separator 分隔）。</param>
/// <param name="IsSeparator">是否为分隔符。</param>
/// <param name="Tooltip">悬停提示。</param>
public readonly record struct EditorMenuItem(
    string Path,
    EditorCommand? Command = null,
    string CommandId = "",
    string Icon = "",
    string Shortcut = "",
    int SortOrder = 0,
    string Group = "",
    bool IsSeparator = false,
    string Tooltip = ""
);
