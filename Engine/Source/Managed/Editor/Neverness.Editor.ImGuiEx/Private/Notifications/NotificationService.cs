namespace Neverness.Editor.ImGuiEx;

/// <summary>
/// Toast 通知服务实现。
///
/// 管理通知队列，处理过期自动消失，委托 ImNotificationRenderer 渲染。
/// 线程安全：Show / Info / Success / Warning / Error 可从任意线程调用。
/// </summary>
public sealed class NotificationService : INotificationService
{
    private readonly List<NotificationItem> m_Active = new();
    private readonly object m_Lock = new();

    // ── 公开 API ──

    /// <inheritdoc />
    public Guid Show(NotificationLevel level, string title, string message, TimeSpan? duration = null)
    {
        var item = new NotificationItem(
            Guid.NewGuid(),
            title,
            message,
            level,
            DateTime.UtcNow,
            duration ?? DefaultDuration(level));

        lock (m_Lock)
        {
            m_Active.Add(item);
        }

        return item.Id;
    }

    /// <inheritdoc />
    public Guid Info(string title, string message)
        => Show(NotificationLevel.Info, title, message);

    /// <inheritdoc />
    public Guid Success(string title, string message)
        => Show(NotificationLevel.Success, title, message);

    /// <inheritdoc />
    public Guid Warning(string title, string message)
        => Show(NotificationLevel.Warning, title, message);

    /// <inheritdoc />
    public Guid Error(string title, string message)
        => Show(NotificationLevel.Error, title, message, TimeSpan.FromSeconds(8));

    /// <inheritdoc />
    public void Dismiss(Guid id)
    {
        lock (m_Lock)
        {
            m_Active.RemoveAll(n => n.Id == id);
        }
    }

    /// <inheritdoc />
    public void DismissAll()
    {
        lock (m_Lock)
        {
            m_Active.Clear();
        }
    }

    // ── 帧循环 ──

    /// <inheritdoc />
    public void Update(float deltaTime)
    {
        lock (m_Lock)
        {
            var now = DateTime.UtcNow;
            m_Active.RemoveAll(n => now - n.CreatedAt >= n.Duration);
        }
    }

    /// <inheritdoc />
    public void Render()
    {
        IReadOnlyList<NotificationItem> snapshot;
        lock (m_Lock)
        {
            snapshot = m_Active.ToArray();
        }

        ImNotificationRenderer.Render(snapshot);
    }

    // ── 内部 ──

    private static TimeSpan DefaultDuration(NotificationLevel level) => level switch
    {
        NotificationLevel.Error => TimeSpan.FromSeconds(8),
        NotificationLevel.Warning => TimeSpan.FromSeconds(6),
        _ => TimeSpan.FromSeconds(4),
    };
}
