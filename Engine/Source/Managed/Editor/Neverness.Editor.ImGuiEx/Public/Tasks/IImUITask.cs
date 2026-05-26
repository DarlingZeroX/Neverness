namespace Neverness.Editor.ImGuiEx;

/// <summary>
/// UI 任务状态。
/// </summary>
public enum ImTaskState
{
    /// <summary>正在运行。</summary>
    Running,

    /// <summary>已完成。</summary>
    Completed,

    /// <summary>发生异常。</summary>
    Faulted,

    /// <summary>已取消。</summary>
    Cancelled,
}

/// <summary>
/// UI 任务接口，表示一个可在 UI 中显示进度的后台异步操作。
/// </summary>
public interface IImUITask
{
    /// <summary>任务唯一标识。</summary>
    Guid TaskId { get; }

    /// <summary>任务显示名称。</summary>
    string Label { get; }

    /// <summary>当前任务状态。</summary>
    ImTaskState State { get; }

    /// <summary>进度 (0..1)。-1 表示不确定进度。</summary>
    float Progress { get; }

    /// <summary>状态描述文本（可选）。</summary>
    string? StatusText { get; }

    /// <summary>取消令牌。</summary>
    CancellationToken CancellationToken { get; }

    /// <summary>启动任务执行。</summary>
    Task RunAsync(UIThreadDispatcher dispatcher);

    /// <summary>请求取消任务。</summary>
    void Cancel();
}
