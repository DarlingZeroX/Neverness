namespace Neverness.Editor.ImGuiEx;

/// <summary>
/// 通知级别，影响 Toast 的颜色和默认持续时间。
/// </summary>
public enum NotificationLevel
{
    /// <summary>信息通知（蓝色）。</summary>
    Info,

    /// <summary>成功通知（绿色）。</summary>
    Success,

    /// <summary>警告通知（黄色）。</summary>
    Warning,

    /// <summary>错误通知（红色，持续时间更长）。</summary>
    Error,
}
