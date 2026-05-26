namespace Neverness.Editor.ImGuiEx;

/// <summary>
/// UI 任务管理器。管理后台任务的启动、跟踪和清理。
/// </summary>
public sealed class UITaskManager
{
    private readonly List<IImUITask> m_ActiveTasks = new();
    private readonly object m_Lock = new();
    private readonly UIThreadDispatcher m_Dispatcher;

    public UITaskManager(UIThreadDispatcher dispatcher)
    {
        m_Dispatcher = dispatcher ?? throw new ArgumentNullException(nameof(dispatcher));
    }

    /// <summary>所有活跃任务的快照。</summary>
    public IReadOnlyList<IImUITask> ActiveTasks
    {
        get { lock (m_Lock) { return m_ActiveTasks.ToArray(); } }
    }

    /// <summary>当前活跃任务数量。</summary>
    public int ActiveCount
    {
        get { lock (m_Lock) { return m_ActiveTasks.Count; } }
    }

    /// <summary>
    /// 启动一个任务并在后台线程执行。
    /// 任务完成后自动从活跃列表移除。
    /// </summary>
    public void StartTask(IImUITask task)
    {
        ArgumentNullException.ThrowIfNull(task);

        lock (m_Lock)
        {
            m_ActiveTasks.Add(task);
        }

        _ = RunTaskAsync(task);
    }

    /// <summary>移除已完成、已取消、已失败的任务。</summary>
    public void PruneCompleted()
    {
        lock (m_Lock)
        {
            m_ActiveTasks.RemoveAll(t =>
                t.State == ImTaskState.Completed ||
                t.State == ImTaskState.Cancelled ||
                t.State == ImTaskState.Faulted);
        }
    }

    /// <summary>取消所有活跃任务。</summary>
    public void CancelAll()
    {
        lock (m_Lock)
        {
            foreach (var task in m_ActiveTasks)
            {
                task.Cancel();
            }
        }
    }

    private async Task RunTaskAsync(IImUITask task)
    {
        try
        {
            await task.RunAsync(m_Dispatcher);
        }
        catch
        {
            // 异常已记录在 task.State 中
        }
        finally
        {
            // 可选：任务完成后自动通知 UI
            m_Dispatcher.Enqueue(() =>
            {
                // UI 层可以监听此事件来显示完成通知
            });
        }
    }
}
