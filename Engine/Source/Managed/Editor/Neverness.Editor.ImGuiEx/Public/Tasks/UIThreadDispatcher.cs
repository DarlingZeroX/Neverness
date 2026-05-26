using System.Collections.Concurrent;

namespace Neverness.Editor.ImGuiEx;

/// <summary>
/// UI 线程调度器，解决后台线程安全提交 UI 操作的问题。
///
/// 所有 ImGui 调用必须在主线程执行。后台线程通过 Enqueue 提交 Action，
/// 主线程在每帧开始时调用 ProcessQueue 排干队列。
/// </summary>
public sealed class UIThreadDispatcher
{
    private readonly ConcurrentQueue<Action> m_Queue = new();

    /// <summary>当前队列中待处理的操作数量。</summary>
    public int PendingCount => m_Queue.Count;

    /// <summary>
    /// 后台线程调用：提交一个需要在主线程执行的 Action。
    /// </summary>
    public void Enqueue(Action action)
    {
        ArgumentNullException.ThrowIfNull(action);
        m_Queue.Enqueue(action);
    }

    /// <summary>
    /// 主线程每帧调用：排干队列并执行所有待处理操作。
    /// 单个 Action 的异常不会阻断后续 Action 的执行。
    /// </summary>
    public void ProcessQueue()
    {
        int budget = 256; // 防止无限循环
        while (budget-- > 0 && m_Queue.TryDequeue(out var action))
        {
            try
            {
                action();
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[UIThreadDispatcher] {ex}");
            }
        }
    }
}
