using Neverness.Editor.Framework.Public.Services;

namespace Neverness.Editor.AvaloniaFrontend.Services;

/// <summary>
/// Avalonia 通知服务实现——管理通知消息。
/// </summary>
public class AvaloniaNotificationService : INotificationService
{
    private readonly Dictionary<string, NotificationItem> _notifications = new();
    private int _nextId;

    public void ShowInfo(string message, float durationSeconds = 3f)
    {
        ShowNotification(message, NotificationLevel.Info, durationSeconds);
    }

    public void ShowWarning(string message, float durationSeconds = 5f)
    {
        ShowNotification(message, NotificationLevel.Warning, durationSeconds);
    }

    public void ShowSuccess(string message, float durationSeconds = 3f)
    {
        ShowNotification(message, NotificationLevel.Success, durationSeconds);
    }

    public void ShowError(string message, float durationSeconds = 0f)
    {
        ShowNotification(message, NotificationLevel.Error, durationSeconds);
    }

    public void ClearAll()
    {
        _notifications.Clear();
        // TODO: 更新 UI
    }

    public void Dismiss(string notificationId)
    {
        _notifications.Remove(notificationId);
        // TODO: 更新 UI
    }

    private void ShowNotification(string message, NotificationLevel level, float durationSeconds)
    {
        var id = $"notification_{++_nextId}";
        var item = new NotificationItem
        {
            Id = id,
            Message = message,
            Level = level,
            DurationSeconds = durationSeconds,
            CreatedAt = DateTime.UtcNow
        };

        _notifications[id] = item;

        // TODO: 通过 Avalonia Dispatcher 更新 UI
        Console.WriteLine($"[NotificationService] [{level}] {message}");

        // 自动消失
        if (durationSeconds > 0)
        {
            _ = Task.Delay(TimeSpan.FromSeconds(durationSeconds)).ContinueWith(_ =>
            {
                Dismiss(id);
            });
        }
    }
}

/// <summary>
/// 通知条目。
/// </summary>
internal class NotificationItem
{
    public string Id { get; set; } = "";
    public string Message { get; set; } = "";
    public NotificationLevel Level { get; set; }
    public float DurationSeconds { get; set; }
    public DateTime CreatedAt { get; set; }
}
