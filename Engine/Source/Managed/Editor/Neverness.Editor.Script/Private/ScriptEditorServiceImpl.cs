using Neverness.Editor.Script.Public;

namespace Neverness.Editor.Script.Private;

/// <summary>
/// 脚本编辑器服务实现——管理 Gameplay 脚本的编译、热重载和文件监听。
/// </summary>
public sealed class ScriptEditorServiceImpl : IScriptEditorService, IDisposable
{
    private FileSystemWatcher? _watcher;
    private readonly object _lock = new();

    public ScriptCompileStatus Status { get; private set; } = ScriptCompileStatus.None;
    public ScriptCompileResult? LastResult { get; private set; }

    public event EventHandler<ScriptFileChangedEventArgs>? FileChanged;
    public event EventHandler<ScriptCompileResult>? CompileFinished;

    public async Task<ScriptCompileResult> CompileAllAsync(CancellationToken ct = default)
    {
        Status = ScriptCompileStatus.Compiling;

        var result = await Task.Run(() =>
        {
            // TODO: 调用 Runtime.Scripting 的 RoslynScriptCompiler 进行编译
            // 暂时返回占位结果
            return new ScriptCompileResult
            {
                Status = ScriptCompileStatus.Success,
                Diagnostics = Array.Empty<ScriptDiagnostic>(),
                Duration = TimeSpan.Zero
            };
        }, ct);

        LastResult = result;
        Status = result.Status;
        CompileFinished?.Invoke(this, result);
        return result;
    }

    public async Task<ScriptCompileResult> CompileFileAsync(string filePath, CancellationToken ct = default)
    {
        Status = ScriptCompileStatus.Compiling;

        var result = await Task.Run(() =>
        {
            // TODO: 单文件编译逻辑
            return new ScriptCompileResult
            {
                Status = ScriptCompileStatus.Success,
                Diagnostics = Array.Empty<ScriptDiagnostic>(),
                Duration = TimeSpan.Zero
            };
        }, ct);

        LastResult = result;
        Status = result.Status;
        CompileFinished?.Invoke(this, result);
        return result;
    }

    public async Task<bool> RequestHotReloadAsync(CancellationToken ct = default)
    {
        if (LastResult is not { Success: true })
            return false;

        // TODO: 与 Runtime.Scripting 的 HotReloadCoordinator 协作
        await Task.CompletedTask;
        return true;
    }

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

    public void Dispose()
    {
        StopWatching();
    }
}
