namespace Neverness.Editor.ImGuiEx;

/// <summary>
/// 通知数据结构，不可变值类型。
/// </summary>
public readonly record struct NotificationItem(
    Guid Id,
    string Title,
    string Message,
    NotificationLevel Level,
    DateTime CreatedAt,
    TimeSpan Duration);
