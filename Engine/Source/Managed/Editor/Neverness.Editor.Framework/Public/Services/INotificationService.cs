namespace Neverness.Editor.Framework.Public.Services;

/// <summary>
/// 通知服务接口——由 Frontend 模块实现。
///
/// 职责：
/// - 显示通知消息（Info/Warning/Success/Error）
/// - 显示模态对话框
/// - 通知生命周期管理（自动消失、手动关闭）
///
/// 设计原则：
/// - Core 只看接口，不依赖具体 UI 框架
/// - AvaloniaFrontend 和 ImGuiFrontend 各自实现
/// </summary>
public interface INotificationService
{
    /// <summary>显示信息通知。</summary>
    void ShowInfo(string message, float durationSeconds = 3f);

    /// <summary>显示警告通知。</summary>
    void ShowWarning(string message, float durationSeconds = 5f);

    /// <summary>显示成功通知。</summary>
    void ShowSuccess(string message, float durationSeconds = 3f);

    /// <summary>显示错误通知。</summary>
    void ShowError(string message, float durationSeconds = 0f);

    /// <summary>清除所有通知。</summary>
    void ClearAll();

    /// <summary>清除指定通知。</summary>
    void Dismiss(string notificationId);
}

/// <summary>
/// 通知级别。
/// </summary>
public enum NotificationLevel
{
    Info,
    Warning,
    Success,
    Error
}
