namespace Neverness.Editor.ImGuiEx;

/// <summary>
/// UI 任务抽象基类。
///
/// 子类实现 ExecuteAsync 定义实际异步逻辑。
/// 基类自动处理 CancellationTokenSource、状态转换和异常捕获。
///
/// 用法:
/// <code>
/// class MyImportTask : ImUITask {
///     public MyImportTask() : base("Importing assets...") { }
///
///     protected override async Task ExecuteAsync(UIThreadDispatcher d, CancellationToken ct) {
///         for (int i = 0; i &lt; files.Count; i++) {
///             ct.ThrowIfCancellationRequested();
///             await ImportFileAsync(files[i]);
///             Progress = (float)(i + 1) / files.Count;
///             StatusText = $"Importing {files[i].Name}...";
///         }
///     }
/// }
///
/// var task = new MyImportTask();
/// await task.RunAsync(dispatcher);
/// </code>
/// </summary>
public abstract class ImUITask : IImUITask, IDisposable
{
    private CancellationTokenSource? m_Cts;
    private bool m_Disposed;

    /// <inheritdoc />
    public Guid TaskId { get; } = Guid.NewGuid();

    /// <inheritdoc />
    public string Label { get; }

    /// <inheritdoc />
    public ImTaskState State { get; private set; } = ImTaskState.Running;

    /// <inheritdoc />
    public float Progress { get; protected set; } = -1f;

    /// <inheritdoc />
    public string? StatusText { get; protected set; }

    /// <inheritdoc />
    public CancellationToken CancellationToken => (m_Cts ??= new CancellationTokenSource()).Token;

    protected ImUITask(string label)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(label);
        Label = label;
    }

    /// <inheritdoc />
    public async Task RunAsync(UIThreadDispatcher dispatcher)
    {
        ArgumentNullException.ThrowIfNull(dispatcher);
        m_Cts ??= new CancellationTokenSource();

        try
        {
            await ExecuteAsync(dispatcher, CancellationToken);
            State = ImTaskState.Completed;
        }
        catch (OperationCanceledException)
        {
            State = ImTaskState.Cancelled;
        }
        catch
        {
            State = ImTaskState.Faulted;
            throw;
        }
    }

    /// <summary>子类实现：定义异步工作逻辑。</summary>
    /// <param name="dispatcher">UI 线程调度器，用于安全提交 UI 操作。</param>
    /// <param name="ct">取消令牌，子类应定期检查。</param>
    protected abstract Task ExecuteAsync(UIThreadDispatcher dispatcher, CancellationToken ct);

    /// <inheritdoc />
    public void Cancel()
    {
        m_Cts?.Cancel();
    }

    /// <inheritdoc />
    public void Dispose()
    {
        if (!m_Disposed)
        {
            m_Disposed = true;
            m_Cts?.Dispose();
        }
    }
}
