namespace Neverness.Rendering.Core;

/// <summary>
/// 主线程渲染调度器——管理渲染回调的注册和执行。
///
/// 设计原则：
/// - 纯静态类，无状态依赖
/// - 线程安全：回调列表使用 lock 保护
/// - 异常隔离：单个回调异常不影响其他回调
/// - 跨模块共享：Editor 和 Runtime 都可使用
///
/// 使用场景：
/// - Editor：AvaloniaFrontendModule.TickRendering() 替代
/// - Runtime：ApplicationHost 主循环中调用
/// - 多视口：每个 ViewportSurface 注册独立回调
/// </summary>
public static class RenderingLoop
{
    /// <summary>已注册的渲染回调列表。</summary>
    private static readonly List<Action> s_callbacks = new();

    /// <summary>线程安全锁。</summary>
    private static readonly object s_lock = new();

    /// <summary>已注册回调数量。</summary>
    public static int CallbackCount
    {
        get
        {
            lock (s_lock)
            {
                return s_callbacks.Count;
            }
        }
    }

    /// <summary>
    /// 注册主线程渲染回调。
    /// 重复注册同一回调会被忽略。
    /// </summary>
    /// <param name="callback">渲染回调，在每帧 TickRendering 时执行。</param>
    public static void RegisterRenderCallback(Action callback)
    {
        ArgumentNullException.ThrowIfNull(callback);

        lock (s_lock)
        {
            if (!s_callbacks.Contains(callback))
            {
                s_callbacks.Add(callback);
            }
        }
    }

    /// <summary>
    /// 注销主线程渲染回调。
    /// </summary>
    /// <param name="callback">要注销的回调。</param>
    public static void UnregisterRenderCallback(Action callback)
    {
        ArgumentNullException.ThrowIfNull(callback);

        lock (s_lock)
        {
            s_callbacks.Remove(callback);
        }
    }

    /// <summary>
    /// 执行所有已注册的渲染回调。
    ///
    /// 必须在主线程调用（Diligent immediate context 非线程安全）。
    /// 建议在主循环中事件处理后、SwapChain Present 前调用。
    ///
    /// 异常处理：单个回调异常不会影响其他回调执行。
    /// </summary>
    public static void TickRendering()
    {
        Action[] snapshot;
        lock (s_lock)
        {
            snapshot = s_callbacks.ToArray();
        }

        foreach (var callback in snapshot)
        {
            try
            {
                callback();
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine($"[RenderingLoop] 渲染回调异常: {ex.Message}");
            }
        }
    }

    /// <summary>
    /// 清除所有已注册的渲染回调。
    /// 通常在 Shutdown 时调用。
    /// </summary>
    public static void ClearAll()
    {
        lock (s_lock)
        {
            s_callbacks.Clear();
        }
    }
}
