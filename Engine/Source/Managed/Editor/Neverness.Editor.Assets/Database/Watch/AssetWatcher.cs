using System.Security.Cryptography;
using System.Threading;
using Neverness.Runtime.VFS;

namespace Neverness.Editor.Assets;

/// <summary>
/// 资产文件监视器。
///
/// 封装 FileSystemWatcher，提供：
///   - 防抖处理（200ms 延迟合并连续事件）
///   - 增量扫描（content hash 比对）
///   - 错误恢复（条件定期扫描）
///
/// 过滤规则见 <see cref="AssetWatcherFilter"/>。
/// 内容哈希缓存见 <see cref="ImportStateCache"/>。
///
/// @threadsafe 内部使用 lock(_lock) 保护事件队列，
///   FlushPendingEvents 从 Timer 回调执行（线程池），事件在锁外触发。
/// </summary>
public sealed class AssetWatcher : IDisposable
{
    private FileSystemWatcher? _watcher;
    private readonly object _lock = new();
    private readonly NPath _assetsRoot;
    private readonly int _debounceMilliseconds;

    /* 防抖计时器 */
    private Timer? _debounceTimer;
    private readonly HashSet<NPath> _pendingChanges = new();
    private readonly HashSet<NPath> _pendingCreations = new();
    private readonly HashSet<NPath> _pendingDeletions = new();
    private readonly List<(NPath from, NPath to)> _pendingRenames = new();

    /* 错误恢复 */
    private int _errorCount;
    private Timer? _recoveryScanTimer;

    /* 事件 */
    public event Action<NPath>? OnAssetChanged;
    public event Action<NPath>? OnAssetCreated;
    public event Action<NPath>? OnAssetDeleted;
    public event Action<NPath, NPath>? OnAssetRenamed;

    /// <summary>Content hash 缓存。</summary>
    private readonly ImportStateCache _stateCache;

    /// <summary>创建资产监视器。</summary>
    /// <param name="assetsRoot">Assets 根目录。</param>
    /// <param name="debounceMilliseconds">防抖延迟毫秒数。</param>
    /// <param name="stateCache">外部 ImportStateCache（可选，为 null 时创建内部实例）。</param>
    public AssetWatcher(NPath assetsRoot, int debounceMilliseconds = 200, ImportStateCache? stateCache = null)
    {
        _assetsRoot = assetsRoot;
        _debounceMilliseconds = debounceMilliseconds;
        _stateCache = stateCache ?? new ImportStateCache();
    }

    /* ======================== 公开 API ======================== */

    /// <summary>开始监视。</summary>
    public void Start()
    {
        lock (_lock)
        {
            if (_watcher != null)
                return;

            if (!Directory.Exists(_assetsRoot.FullPath))
            {
                Console.WriteLine($"[AssetWatcher] Assets 目录不存在: {_assetsRoot}");
                return;
            }

            _watcher = new FileSystemWatcher(_assetsRoot.FullPath)
            {
                NotifyFilter = NotifyFilters.FileName
                             | NotifyFilters.DirectoryName
                             | NotifyFilters.LastWrite
                             | NotifyFilters.Size,
                IncludeSubdirectories = true,
                EnableRaisingEvents = true
            };

            _watcher.Changed += OnFileChanged;
            _watcher.Created += OnFileCreated;
            _watcher.Deleted += OnFileDeleted;
            _watcher.Renamed += OnFileRenamed;
            _watcher.Error += OnWatcherError;

            _debounceTimer = new Timer(FlushPendingEvents, null, Timeout.Infinite, Timeout.Infinite);

            Console.WriteLine($"[AssetWatcher] 开始监视: {_assetsRoot}");
        }
    }

    /// <summary>停止监视。</summary>
    public void Stop()
    {
        lock (_lock)
        {
            if (_watcher == null)
                return;

            _watcher.EnableRaisingEvents = false;
            _watcher.Dispose();
            _watcher = null;

            _debounceTimer?.Dispose();
            _debounceTimer = null;

            _recoveryScanTimer?.Dispose();
            _recoveryScanTimer = null;

            _pendingChanges.Clear();
            _pendingCreations.Clear();
            _pendingDeletions.Clear();
            _pendingRenames.Clear();

            Console.WriteLine("[AssetWatcher] 停止监视");
        }
    }

    /// <summary>是否正在监视。</summary>
    public bool IsWatching
    {
        get { lock (_lock) return _watcher is { EnableRaisingEvents: true }; }
    }

    /// <summary>强制全量扫描（不依赖 FileSystemWatcher）。</summary>
    public void ForceFullScan()
    {
        lock (_lock)
        {
            ScanDirectory(_assetsRoot);
        }
    }

    /// <summary>Content hash 缓存。</summary>
    public ImportStateCache StateCache => _stateCache;

    /* ======================== 事件处理 ======================== */

    private void OnFileChanged(object sender, FileSystemEventArgs e)
    {
        if (AssetWatcherFilter.ShouldIgnoreFile(e.FullPath)) return;

        lock (_lock)
        {
            _pendingChanges.Add(new NPath(e.FullPath));
            ResetDebounceTimer();
        }
    }

    private void OnFileCreated(object sender, FileSystemEventArgs e)
    {
        if (AssetWatcherFilter.ShouldIgnoreFile(e.FullPath)) return;

        lock (_lock)
        {
            _pendingCreations.Add(new NPath(e.FullPath));
            ResetDebounceTimer();
        }
    }

    private void OnFileDeleted(object sender, FileSystemEventArgs e)
    {
        if (AssetWatcherFilter.ShouldIgnoreFile(e.FullPath)) return;

        lock (_lock)
        {
            _pendingDeletions.Add(new NPath(e.FullPath));
            ResetDebounceTimer();
        }
    }

    private void OnFileRenamed(object sender, RenamedEventArgs e)
    {
        if (AssetWatcherFilter.ShouldIgnoreFile(e.FullPath)) return;

        lock (_lock)
        {
            _pendingRenames.Add((new NPath(e.OldFullPath), new NPath(e.FullPath)));
            ResetDebounceTimer();
        }
    }

    private void OnWatcherError(object sender, ErrorEventArgs e)
    {
        Console.WriteLine($"[AssetWatcher] FileSystemWatcher 错误: {e.GetException().Message}");
        var count = Interlocked.Increment(ref _errorCount);

        /* 首次错误时启动定期恢复扫描（每 30 秒） */
        if (count == 1)
        {
            Console.WriteLine("[AssetWatcher] 启动定期恢复扫描（每 30 秒）");
            _recoveryScanTimer?.Dispose();
            _recoveryScanTimer = new Timer(RecoveryScanCallback, null,
                TimeSpan.FromSeconds(30), TimeSpan.FromSeconds(30));
        }
    }

    /// <summary>恢复扫描回调——扫描成功后停止定期扫描。</summary>
    private void RecoveryScanCallback(object? state)
    {
        try
        {
            Console.WriteLine("[AssetWatcher] 执行恢复扫描...");
            ForceFullScan();

            /* 扫描成功，重置错误计数，停止定期扫描 */
            Interlocked.Exchange(ref _errorCount, 0);
            _recoveryScanTimer?.Dispose();
            _recoveryScanTimer = null;
            Console.WriteLine("[AssetWatcher] 恢复扫描成功，停止定期扫描");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[AssetWatcher] 恢复扫描失败: {ex.Message}");
        }
    }

    /* ======================== 防抖 ======================== */

    private void ResetDebounceTimer()
    {
        _debounceTimer?.Change(_debounceMilliseconds, Timeout.Infinite);
    }

    private void FlushPendingEvents(object? state)
    {
        List<NPath> changes, creations, deletions;
        List<(NPath from, NPath to)> renames;

        lock (_lock)
        {
            if (_pendingChanges.Count == 0 && _pendingCreations.Count == 0
                && _pendingDeletions.Count == 0 && _pendingRenames.Count == 0)
                return;

            changes = new List<NPath>(_pendingChanges);
            creations = new List<NPath>(_pendingCreations);
            deletions = new List<NPath>(_pendingDeletions);
            renames = new List<(NPath, NPath)>(_pendingRenames);

            _pendingChanges.Clear();
            _pendingCreations.Clear();
            _pendingDeletions.Clear();
            _pendingRenames.Clear();
        }

        /* 处理事件 */
        foreach (var path in creations)
        {
            try
            {
                if (File.Exists(path.FullPath))
                {
                    /* 跳过已由 DropImportService 预导入的文件 */
                    if (!_stateCache.HasChanged(path))
                        continue;

                    OnAssetCreated?.Invoke(path);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[AssetWatcher] 处理创建事件失败: {path} → {ex.Message}");
            }
        }

        foreach (var path in changes)
        {
            try
            {
                /* Content hash 比对，只在真正变化时触发 */
                if (_stateCache.HasChanged(path))
                {
                    OnAssetChanged?.Invoke(path);
                    _stateCache.MarkImported(path, ComputeContentHash(path));
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[AssetWatcher] 处理变更事件失败: {path} → {ex.Message}");
            }
        }

        foreach (var path in deletions)
        {
            try
            {
                OnAssetDeleted?.Invoke(path);
                _stateCache.Remove(path);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[AssetWatcher] 处理删除事件失败: {path} → {ex.Message}");
            }
        }

        foreach (var (from, to) in renames)
        {
            try
            {
                OnAssetRenamed?.Invoke(from, to);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[AssetWatcher] 处理重命名事件失败: {from} → {to} → {ex.Message}");
            }
        }
    }

    /* ======================== 增量扫描 ======================== */

    private void ScanDirectory(NPath dir)
    {
        if (!Directory.Exists(dir.FullPath))
            return;

        try
        {
            foreach (var file in Directory.EnumerateFiles(dir.FullPath))
            {
                if (AssetWatcherFilter.ShouldIgnoreFile(file))
                    continue;

                var filePath = new NPath(file);
                if (_stateCache.HasChanged(filePath))
                {
                    /* 缓存中有记录 → 已有文件变化，无记录 → 新文件 */
                    if (_stateCache.GetCachedHash(filePath) != null)
                        OnAssetChanged?.Invoke(filePath);
                    else
                        OnAssetCreated?.Invoke(filePath);

                    _stateCache.MarkImported(filePath, ComputeContentHash(filePath));
                }
            }

            foreach (var subDir in Directory.EnumerateDirectories(dir.FullPath))
            {
                if (AssetWatcherFilter.ShouldIgnoreDirectory(subDir))
                    continue;
                ScanDirectory(new NPath(subDir));
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[AssetWatcher] 扫描目录失败: {dir} → {ex.Message}");
        }
    }

    /* ======================== Content Hash ======================== */

    private static byte[] ComputeContentHash(NPath filePath)
    {
        try
        {
            using var stream = File.OpenRead(filePath.FullPath);
            return SHA256.HashData(stream);
        }
        catch
        {
            return Array.Empty<byte>();
        }
    }

    /* ======================== IDisposable ======================== */

    public void Dispose()
    {
        Stop();
    }
}
