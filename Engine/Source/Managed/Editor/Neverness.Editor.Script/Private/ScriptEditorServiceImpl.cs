// ============================================================================
// ScriptEditorServiceImpl.cs - 脚本编辑器服务实现
// ============================================================================
// 管理文件监听和编译事件转发。
// 编译逻辑由 ScriptCompileService 管理，本类只负责文件监听。
// ============================================================================

using Neverness.Editor.Script.Public;

namespace Neverness.Editor.Script.Private;

/// <summary>
/// 脚本编辑器服务实现——文件监听 + 编译事件转发。
/// </summary>
public sealed class ScriptEditorServiceImpl : IScriptEditorService, IDisposable
{
    // ========================================================================
    // 内部状态
    // ========================================================================

    private FileSystemWatcher? _watcher;
    private string? _watchDirectory;
    private ScriptAssemblyDefinition? _assemblyDefinition;

    // ── 编译状态（由外部 ScriptCompileService 更新）──
    private ScriptCompileStatus _status = ScriptCompileStatus.None;
    private ScriptCompileResult? _lastResult;

    // ========================================================================
    // IScriptEditorService
    // ========================================================================

    public ScriptCompileStatus Status => _status;
    public ScriptCompileResult? LastResult => _lastResult;

    public event EventHandler<ScriptFileChangedEventArgs>? FileChanged;
    public event EventHandler<ScriptCompileResult>? CompileFinished;

    // ========================================================================
    // 构造函数
    // ========================================================================

    public ScriptEditorServiceImpl()
    {
    }

    // ========================================================================
    // 配置
    // ========================================================================

    /// <summary>设置文件监听目录。</summary>
    public void SetWatchDirectory(string directory)
    {
        _watchDirectory = directory;
    }

    /// <summary>设置当前程序集定义。</summary>
    public void SetAssemblyDefinition(ScriptAssemblyDefinition assemblyDefinition)
    {
        _assemblyDefinition = assemblyDefinition;
    }

    // ========================================================================
    // 编译（由 ScriptEditorModule 调用）
    // ========================================================================

    /// <summary>异步编译所有脚本（由 ScriptEditorModule 委托给 ScriptCompileService）。</summary>
    public async Task<ScriptCompileResult> CompileAllAsync(CancellationToken ct = default)
    {
        // 编译逻辑由 ScriptEditorModule 管理
        // 这里只提供接口兼容性
        await Task.CompletedTask;
        return _lastResult ?? new ScriptCompileResult
        {
            Status = ScriptCompileStatus.Error,
            Diagnostics = new[] { new ScriptDiagnostic { Message = "Compilation not available.", IsError = true } }
        };
    }

    /// <summary>异步编译单个文件。</summary>
    public async Task<ScriptCompileResult> CompileFileAsync(string filePath, CancellationToken ct = default)
    {
        return await CompileAllAsync(ct);
    }

    /// <summary>请求热重载。</summary>
    public async Task<bool> RequestHotReloadAsync(CancellationToken ct = default)
    {
        await Task.CompletedTask;
        return _lastResult?.Success ?? false;
    }

    // ========================================================================
    // 编译状态更新（由 ScriptCompileService 回调）
    // ========================================================================

    /// <summary>更新编译状态（由 ScriptCompileService.CompileFinished 回调）。</summary>
    public void UpdateCompileStatus(ScriptCompileStatus status, ScriptCompileResult? result)
    {
        _status = status;
        _lastResult = result;
        if (result != null)
        {
            CompileFinished?.Invoke(this, result);
        }
    }

    // ========================================================================
    // 文件监听
    // ========================================================================

    public IReadOnlyList<string> GetScriptFiles(string projectDirectory)
    {
        if (!Directory.Exists(projectDirectory))
            return Array.Empty<string>();

        return Directory.GetFiles(projectDirectory, "*.cs", SearchOption.AllDirectories)
            .Where(f => !f.Contains(Path.Combine("obj", "")) &&
                        !f.Contains(Path.Combine("bin", "")) &&
                        !f.Contains(Path.Combine("Build", "")))
            .ToArray();
    }

    public void StartWatching(string projectDirectory)
    {
        StopWatching();
        _watchDirectory = projectDirectory;

        if (!Directory.Exists(projectDirectory))
            return;

        _watcher = new FileSystemWatcher(projectDirectory, "*.cs")
        {
            NotifyFilter = NotifyFilters.LastWrite | NotifyFilters.FileName | NotifyFilters.CreationTime,
            IncludeSubdirectories = true,
            EnableRaisingEvents = true
        };

        _watcher.Created += OnFileChanged;
        _watcher.Changed += OnFileChanged;
        _watcher.Deleted += OnFileChanged;
        _watcher.Renamed += OnFileRenamed;
    }

    public void StopWatching()
    {
        if (_watcher == null) return;

        _watcher.EnableRaisingEvents = false;
        _watcher.Created -= OnFileChanged;
        _watcher.Changed -= OnFileChanged;
        _watcher.Deleted -= OnFileChanged;
        _watcher.Renamed -= OnFileRenamed;
        _watcher.Dispose();
        _watcher = null;
    }

    // ========================================================================
    // 内部方法
    // ========================================================================

    private void OnFileChanged(object sender, FileSystemEventArgs e)
    {
        var kind = e.ChangeType switch
        {
            WatcherChangeTypes.Created => ScriptFileChangeKind.Created,
            WatcherChangeTypes.Deleted => ScriptFileChangeKind.Deleted,
            _ => ScriptFileChangeKind.Modified
        };

        FileChanged?.Invoke(this, new ScriptFileChangedEventArgs
        {
            FilePath = e.FullPath,
            ChangeKind = kind
        });
    }

    private void OnFileRenamed(object sender, RenamedEventArgs e)
    {
        FileChanged?.Invoke(this, new ScriptFileChangedEventArgs
        {
            FilePath = e.FullPath,
            ChangeKind = ScriptFileChangeKind.Renamed
        });
    }

    // ========================================================================
    // IDisposable
    // ========================================================================

    public void Dispose()
    {
        StopWatching();
    }
}
