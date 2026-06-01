// ============================================================================
// ScriptCompileQueue.cs - 脚本编译队列
// ============================================================================
// 管理脚本编译队列，支持延迟编译和防抖机制。
// ============================================================================

namespace Neverness.Editor.Script.Public;

/// <summary>
/// 脚本编译队列：管理待编译的脚本文件。
/// </summary>
/// <remarks>
/// 使用防抖机制避免频繁触发编译（例如快速保存多个文件时）。
/// </remarks>
public static class ScriptCompileQueue
{
    // ========================================================================
    // 内部状态
    // ========================================================================

    /// <summary>待编译的文件队列。</summary>
    private static readonly HashSet<string> _pendingFiles = new();

    /// <summary>防抖计时器。</summary>
    private static Timer? _debounceTimer;

    /// <summary>防抖延迟（毫秒）。</summary>
    private const int DebounceDelayMs = 500;

    /// <summary>锁。</summary>
    private static readonly object _lock = new();

    /// <summary>是否正在编译。</summary>
    private static bool _isCompiling;

    // ========================================================================
    // 事件
    // ========================================================================

    /// <summary>编译队列已触发时发生。</summary>
    public static event EventHandler<IReadOnlyList<string>>? CompileTriggered;

    // ========================================================================
    // 公共方法
    // ========================================================================

    /// <summary>
    /// 将文件加入编译队列。
    /// </summary>
    /// <param name="filePath">文件路径。</param>
    public static void Enqueue(string filePath)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(filePath);

        lock (_lock)
        {
            _pendingFiles.Add(filePath);

            // 重置防抖计时器
            _debounceTimer?.Dispose();
            _debounceTimer = new Timer(OnDebounceElapsed, null, DebounceDelayMs, Timeout.Infinite);
        }
    }

    /// <summary>
    /// 将多个文件加入编译队列。
    /// </summary>
    /// <param name="filePaths">文件路径列表。</param>
    public static void EnqueueRange(IEnumerable<string> filePaths)
    {
        ArgumentNullException.ThrowIfNull(filePaths);

        lock (_lock)
        {
            foreach (var path in filePaths)
            {
                _pendingFiles.Add(path);
            }

            // 重置防抖计时器
            _debounceTimer?.Dispose();
            _debounceTimer = new Timer(OnDebounceElapsed, null, DebounceDelayMs, Timeout.Infinite);
        }
    }

    /// <summary>
    /// 清空编译队列。
    /// </summary>
    public static void Clear()
    {
        lock (_lock)
        {
            _pendingFiles.Clear();
            _debounceTimer?.Dispose();
            _debounceTimer = null;
        }
    }

    /// <summary>
    /// 获取待编译的文件列表。
    /// </summary>
    /// <returns>待编译文件列表。</returns>
    public static IReadOnlyList<string> GetPendingFiles()
    {
        lock (_lock)
        {
            return _pendingFiles.ToList();
        }
    }

    /// <summary>
    /// 队列中是否有待编译的文件。
    /// </summary>
    public static bool HasPendingFiles
    {
        get
        {
            lock (_lock)
            {
                return _pendingFiles.Count > 0;
            }
        }
    }

    // ========================================================================
    // 内部方法
    // ========================================================================

    /// <summary>防抖计时器到期时触发编译。</summary>
    private static void OnDebounceElapsed(object? state)
    {
        List<string> filesToCompile;

        lock (_lock)
        {
            if (_pendingFiles.Count == 0 || _isCompiling)
            {
                return;
            }

            filesToCompile = _pendingFiles.ToList();
            _pendingFiles.Clear();
            _isCompiling = true;
        }

        try
        {
            // 触发编译事件
            CompileTriggered?.Invoke(null, filesToCompile);
        }
        finally
        {
            lock (_lock)
            {
                _isCompiling = false;
            }
        }
    }
}
