using System.Security.Cryptography;
using Neverness.Runtime.Assets;
using Neverness.Runtime.VFS;

namespace Neverness.Editor.Assets;

/// <summary>
/// 資產檔案監視器。
///
/// 封裝 FileSystemWatcher，提供：
///   - 防抖處理（200ms 延遲合併連續事件）
///   - 事件過濾（忽略 Library/、.meta 檔案）
///   - 增量掃描（content hash 比對）
///   - 與 EditorAssetDatabase 整合
///
/// @threadsafe 內部使用 lock(_lock) 保護事件佇列，
///   FlushPendingEvents 從 Timer 回調執行（執行緒集區），事件在鎖外觸發。
///   ImportStateCache 各自獨立加鎖。
/// </summary>
public sealed class AssetWatcher : IDisposable
{
    private FileSystemWatcher? _watcher;
    private readonly object _lock = new();
    private readonly NPath _assetsRoot;
    private readonly int _debounceMilliseconds;

    /* 防抖計時器 */
    private Timer? _debounceTimer;
    private readonly HashSet<NPath> _pendingChanges = new();
    private readonly HashSet<NPath> _pendingCreations = new();
    private readonly HashSet<NPath> _pendingDeletions = new();
    private readonly List<(NPath from, NPath to)> _pendingRenames = new();

    /* 事件 */
    public event Action<NPath>? OnAssetChanged;
    public event Action<NPath>? OnAssetCreated;
    public event Action<NPath>? OnAssetDeleted;
    public event Action<NPath, NPath>? OnAssetRenamed;

    /// <summary>Content hash 快取。</summary>
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

    /* ======================== 公開 API ======================== */

    /// <summary>開始監視。</summary>
    public void Start()
    {
        lock (_lock)
        {
            if (_watcher != null)
                return;

            if (!Directory.Exists(_assetsRoot.FullPath))
            {
                Console.WriteLine($"[AssetWatcher] Assets 目錄不存在: {_assetsRoot}");
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

            Console.WriteLine($"[AssetWatcher] 開始監視: {_assetsRoot}");
        }
    }

    /// <summary>停止監視。</summary>
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

            _pendingChanges.Clear();
            _pendingCreations.Clear();
            _pendingDeletions.Clear();
            _pendingRenames.Clear();

            Console.WriteLine("[AssetWatcher] 停止監視");
        }
    }

    /// <summary>是否正在監視。</summary>
    public bool IsWatching
    {
        get { lock (_lock) return _watcher is { EnableRaisingEvents: true }; }
    }

    /// <summary>強制全量掃描（不依賴 FileSystemWatcher）。</summary>
    public void ForceFullScan()
    {
        lock (_lock)
        {
            ScanDirectory(_assetsRoot);
        }
    }

    /// <summary>Content hash 快取。</summary>
    public ImportStateCache StateCache => _stateCache;

    /* ======================== 事件處理 ======================== */

    private void OnFileChanged(object sender, FileSystemEventArgs e)
    {
        if (ShouldIgnore(e.FullPath)) return;

        lock (_lock)
        {
            _pendingChanges.Add(new NPath(e.FullPath));
            ResetDebounceTimer();
        }
    }

    private void OnFileCreated(object sender, FileSystemEventArgs e)
    {
        if (ShouldIgnore(e.FullPath)) return;

        lock (_lock)
        {
            _pendingCreations.Add(new NPath(e.FullPath));
            ResetDebounceTimer();
        }
    }

    private void OnFileDeleted(object sender, FileSystemEventArgs e)
    {
        if (ShouldIgnore(e.FullPath)) return;

        lock (_lock)
        {
            _pendingDeletions.Add(new NPath(e.FullPath));
            ResetDebounceTimer();
        }
    }

    private void OnFileRenamed(object sender, RenamedEventArgs e)
    {
        if (ShouldIgnore(e.FullPath)) return;

        lock (_lock)
        {
            _pendingRenames.Add((new NPath(e.OldFullPath), new NPath(e.FullPath)));
            ResetDebounceTimer();
        }
    }

    private void OnWatcherError(object sender, ErrorEventArgs e)
    {
        Console.WriteLine($"[AssetWatcher] FileSystemWatcher 錯誤: {e.GetException().Message}");
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

        /* 處理事件 */
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
                Console.WriteLine($"[AssetWatcher] 處理建立事件失敗: {path} → {ex.Message}");
            }
        }

        foreach (var path in changes)
        {
            try
            {
                /* Content hash 比對，只在真正變化時觸發 */
                if (_stateCache.HasChanged(path))
                {
                    OnAssetChanged?.Invoke(path);
                    _stateCache.MarkImported(path, ComputeContentHash(path));
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[AssetWatcher] 處理變更事件失敗: {path} → {ex.Message}");
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
                Console.WriteLine($"[AssetWatcher] 處理刪除事件失敗: {path} → {ex.Message}");
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
                Console.WriteLine($"[AssetWatcher] 處理重命名事件失敗: {from} → {to} → {ex.Message}");
            }
        }
    }

    /* ======================== 增量掃描 ======================== */

    private void ScanDirectory(NPath dir)
    {
        if (!Directory.Exists(dir.FullPath))
            return;

        try
        {
            foreach (var file in Directory.EnumerateFiles(dir.FullPath))
            {
                if (ShouldIgnore(file))
                    continue;

                var filePath = new NPath(file);
                if (_stateCache.HasChanged(filePath))
                {
                    OnAssetCreated?.Invoke(filePath);
                    _stateCache.MarkImported(filePath, ComputeContentHash(filePath));
                }
            }

            foreach (var subDir in Directory.EnumerateDirectories(dir.FullPath))
            {
                if (ShouldIgnoreDirectory(subDir))
                    continue;
                ScanDirectory(new NPath(subDir));
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[AssetWatcher] 掃描目錄失敗: {dir} → {ex.Message}");
        }
    }

    /* ======================== 過濾 ======================== */

    private static bool ShouldIgnore(string path)
    {
        /* 忽略 .meta 檔案 */
        if (path.EndsWith(".meta", StringComparison.OrdinalIgnoreCase))
            return true;

        /* 忽略暫存檔案 */
        var fileName = Path.GetFileName(path);
        if (fileName.StartsWith('.') || fileName.StartsWith('~'))
            return true;

        return false;
    }

    private static bool ShouldIgnoreDirectory(string dirPath)
    {
        var dirName = Path.GetFileName(dirPath);
        return dirName.Equals("Library", StringComparison.OrdinalIgnoreCase)
            || dirName.Equals("Temp", StringComparison.OrdinalIgnoreCase)
            || dirName.StartsWith('.');
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

/// <summary>
/// 匯入狀態快取（content hash 比對）。
///
/// 用於增量匯入：檔案未變化時跳過 reimport。
/// </summary>
public sealed class ImportStateCache
{
    private readonly Dictionary<NPath, (DateTime lastWrite, byte[] hash)> _cache = new();

    private readonly object _lock = new();

    /// <summary>檔案是否有變化（hash 不匹配或不存在於快取）。</summary>
    public bool HasChanged(NPath path)
    {
        lock (_lock)
        {
            if (!File.Exists(path.FullPath))
                return true;

            var lastWrite = File.GetLastWriteTimeUtc(path.FullPath);

            if (_cache.TryGetValue(path, out var entry))
            {
                /* 時間戳相同，認定未變化 */
                if (entry.lastWrite == lastWrite)
                    return false;
            }

            return true;
        }
    }

    /// <summary>標記檔案為已匯入。</summary>
    public void MarkImported(NPath path, byte[] hash)
    {
        lock (_lock)
        {
            var lastWrite = File.Exists(path.FullPath) ? File.GetLastWriteTimeUtc(path.FullPath) : DateTime.UtcNow;
            _cache[path] = (lastWrite, hash);
        }
    }

    /// <summary>取得已快取的 hash。</summary>
    public byte[]? GetCachedHash(NPath path)
    {
        lock (_lock)
        {
            return _cache.TryGetValue(path, out var entry) ? entry.hash : null;
        }
    }

    /// <summary>移除快取條目。</summary>
    public void Remove(NPath path)
    {
        lock (_lock) _cache.Remove(path);
    }

    /// <summary>快取條目數量。</summary>
    public int Count
    {
        get { lock (_lock) return _cache.Count; }
    }

    /// <summary>儲存快取至磁碟。</summary>
    public void Save(NPath cachePath)
    {
        lock (_lock)
        {
            try
            {
                var dir = Path.GetDirectoryName(cachePath.FullPath);
                if (!string.IsNullOrEmpty(dir) && !Directory.Exists(dir))
                    Directory.CreateDirectory(dir);

                using var stream = File.Create(cachePath.FullPath);
                using var writer = new BinaryWriter(stream);

                writer.Write(0x4E4E4943u); /* 'NNIC' = Neverness Import Cache */
                writer.Write(1u);          /* version */
                writer.Write(_cache.Count);

                foreach (var (path, entry) in _cache)
                {
                    writer.Write(path.FullPath);
                    writer.Write(entry.lastWrite.ToBinary());
                    writer.Write(entry.hash.Length);
                    writer.Write(entry.hash);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[ImportStateCache] 儲存失敗: {ex.Message}");
            }
        }
    }

    /// <summary>從磁碟載入快取。</summary>
    public void Load(NPath cachePath)
    {
        lock (_lock)
        {
            if (!File.Exists(cachePath.FullPath))
                return;

            try
            {
                using var stream = File.OpenRead(cachePath.FullPath);
                using var reader = new BinaryReader(stream);

                var magic = reader.ReadUInt32();
                var version = reader.ReadUInt32();
                if (magic != 0x4E4E4943u || version != 1u)
                    return;

                var count = reader.ReadInt32();
                for (var i = 0; i < count; i++)
                {
                    var path = reader.ReadString();
                    var lastWriteBinary = reader.ReadInt64();
                    var hashLen = reader.ReadInt32();
                    var hash = reader.ReadBytes(hashLen);

                    _cache[new NPath(path)] = (DateTime.FromBinary(lastWriteBinary), hash);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[ImportStateCache] 載入失敗: {ex.Message}");
                _cache.Clear();
            }
        }
    }
}
