namespace Neverness.Editor.Framework.Public;

/// <summary>
/// 编辑器命令：菜单 / Toolbar / Context Menu 共用。
/// 基于委托模型，NativeAOT 友好，无反射。
/// </summary>
public sealed class EditorCommand
{
    /// <summary>命令唯一标识符（如 "file.save"）。</summary>
    public required string Id { get; init; }

    /// <summary>显示名称。</summary>
    public required string DisplayName { get; init; }

    /// <summary>执行回调。</summary>
    public required Action<EditorCommandContext> Execute { get; init; }

    /// <summary>是否可执行（null = 始终可执行）。</summary>
    public Func<bool>? CanExecute { get; init; }

    /// <summary>是否处于勾选状态（null = 非勾选菜单项）。</summary>
    public Func<bool>? IsChecked { get; init; }

    /// <summary>悬停提示。</summary>
    public string? Tooltip { get; init; }
}

/// <summary>
/// 命令执行上下文（预留扩展点）。
/// </summary>
public readonly struct EditorCommandContext
{
    /// <summary>用户数据（用于传递上下文信息）。</summary>
    public readonly object? UserData;

    /// <summary>构造命令上下文。</summary>
    public EditorCommandContext(object? userData = null) => UserData = userData;
}
