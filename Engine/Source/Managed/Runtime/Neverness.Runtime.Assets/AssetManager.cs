using System.IO;
using System.Runtime.InteropServices;
using Neverness.Runtime.Assets.Formats;
using Neverness.Runtime.Assets.Pack;
using Neverness.Runtime.Assets.Registry;
using Neverness.Runtime.Assets.Streaming;

namespace Neverness.Runtime.Assets;

/// <summary>
/// 資產管理條目（內部結構）。
/// </summary>
internal sealed class AssetEntry
{
    public GUID Guid;
    public ulong TypeId;
    public ulong Handle;
    public int RefCount;
    public AssetState State;
    public List<GUID> Dependencies = new();
    public byte[] Data = Array.Empty<byte>();
    public BlobDescriptor[] Blobs = Array.Empty<BlobDescriptor>();
    public int PayloadOffset;
    public string? SourcePath;
    public bool NeedsReload;
}

/// <summary>
/// 資產狀態。
/// </summary>
internal enum AssetState
{
    Unloaded = 0,
    Loading = 1,
    Loaded = 2,
    Failed = 3,
    Streaming = 4,
}

/// <summary>
/// 核心資產管理器（單例）。
///
/// 串聯 HandleTable、AssetCache、StreamingManager、PackManager、AssetRegistry。
/// 提供同步/異步加載、Tick、Hot Reload、資料存取等完整 API。
///
/// 與 C++ NNAssetManager 對應。
/// </summary>
public sealed class AssetManager
{
    /* ======================== 內部狀態 ======================== */

    private readonly HandleTable _handleTable = new();
    private readonly AssetCache _cache = new();
    private StreamingManager? _streaming;
    private readonly AssetTypeRegistry _typeRegistry = AssetTypeRegistry.Instance;
    private readonly AssetRegistry _assetRegistry = AssetRegistry.Instance;
    private readonly PackManager _packManager = PackManager.Instance;

    // GUID.Low → AssetEntry
    private readonly Dictionary<ulong, AssetEntry> _guidToEntry = new();
    // Handle → AssetEntry
    private readonly Dictionary<ulong, AssetEntry> _handleToEntry = new();

    private string _assetRoot = string.Empty;
    private bool _initialized;
    private readonly object _lock = new();

    /* ======================== 單例 ======================== */

    public static AssetManager Instance { get; } = new();

    private AssetManager() { }

    /* ======================== 初始化/關閉 ======================== */

    /// <summary>
    /// 初始化資產管理器。
    /// </summary>
    /// <param name="assetRoot">資產根目錄（如 "Library/Imported"）。</param>
    public bool Initialize(string assetRoot)
    {
        lock (_lock)
        {
            if (_initialized) return true;
            _assetRoot = assetRoot;

            // 快取預設 512MB
            _cache.MemoryBudget = 512L * 1024 * 1024;

            // 啟動 StreamingManager
            _streaming = new StreamingManager(assetRoot);
            _streaming.Start();

            _initialized = true;
            return true;
        }
    }

    /// <summary>
    /// 關閉資產管理器，釋放所有資源。
    /// </summary>
    public void Shutdown()
    {
        lock (_lock)
        {
            if (!_initialized) return;

            _streaming?.Dispose();
            _streaming = null;

            _guidToEntry.Clear();
            _handleToEntry.Clear();
            _packManager.UnmountAll();

            _initialized = false;
        }
    }

    /* ======================== Tick ======================== */

    /// <summary>
    /// 主線程 Tick：消費非同步完成佇列，處理 Hot Reload。
    /// 每幀調用一次。
    /// </summary>
    public void Tick()
    {
        lock (_lock)
        {
            if (!_initialized) return;

            // 消費 StreamingManager 完成佇列
            _streaming?.DrainCompleted(OnLoadCompleted);

            // Hot Reload：檢查標記的資產
            foreach (var entry in _guidToEntry.Values)
            {
                if (!entry.NeedsReload) continue;
                entry.NeedsReload = false;

                if (ReadNnAsset(entry.SourcePath, entry))
                    entry.State = AssetState.Loaded;
                else
                    entry.State = AssetState.Failed;
            }
        }
    }

    /* ======================== 同步載入 ======================== */

    /// <summary>
    /// 同步加載資產。
    /// </summary>
    /// <param name="guid">資產 GUID。</param>
    /// <param name="typeId">型別 ID（0 = 自動從 header 讀取）。</param>
    /// <returns>AssetHandle 原始值（0 = 失敗）。</returns>
    public ulong LoadAssetSync(GUID guid, ulong typeId = 0)
    {
        lock (_lock)
        {
            return LoadAssetInternal(guid, typeId);
        }
    }

    private ulong LoadAssetInternal(GUID guid, ulong typeId)
    {
        if (guid.IsZero)
            return 0;

        // 檢查是否已載入
        if (_guidToEntry.TryGetValue(guid.Low, out var existing))
        {
            if (existing.State == AssetState.Loaded)
            {
                _handleTable.AddRef(existing.Handle);
                return existing.Handle;
            }
        }

        // 先檢查包
        byte[]? data = null;
        if (_packManager.IsAssetInPackage(guid))
        {
            data = _packManager.ReadAssetFromPackageCopy(guid);
        }

        // 包中沒有，從檔案系統讀取
        if (data == null)
        {
            var path = ResolveAssetPath(guid);
            if (string.IsNullOrEmpty(path) || !File.Exists(path))
                return 0;

            try
            {
                data = File.ReadAllBytes(path);
            }
            catch
            {
                return 0;
            }
        }

        if (data.Length < (int)AssetFormat.HeaderSize)
            return 0;

        // 建立條目
        var entry = new AssetEntry
        {
            Guid = guid,
            TypeId = typeId,
            State = AssetState.Loading,
            Data = data,
            SourcePath = ResolveAssetPath(guid),
        };

        // 解析 .nnasset
        if (!ParseNnassetData(entry))
        {
            entry.State = AssetState.Failed;
            return 0;
        }

        // 分配 Handle
        var rawHandle = _handleTable.Allocate(entry, entry.TypeId);
        entry.Handle = rawHandle;
        entry.State = AssetState.Loaded;

        // 更新索引
        _guidToEntry[guid.Low] = entry;
        _handleToEntry[rawHandle] = entry;

        // 更新快取
        _cache.Touch(guid, entry.Data.Length, rawHandle);

        return rawHandle;
    }

    /* ======================== 異步載入 ======================== */

    /// <summary>
    /// 非同步加載資產。
    /// </summary>
    /// <param name="guid">資產 GUID。</param>
    /// <param name="typeId">型別 ID。</param>
    /// <param name="priority">加載優先級。</param>
    /// <param name="ct">取消令牌。</param>
    /// <returns>AssetHandle 原始值。</returns>
    public async ValueTask<ulong> LoadAssetAsync(GUID guid, ulong typeId = 0,
        LoadPriority priority = LoadPriority.Normal,
        CancellationToken ct = default)
    {
        // 檢查是否已載入
        lock (_lock)
        {
            if (_guidToEntry.TryGetValue(guid.Low, out var existing)
                && existing.State == AssetState.Loaded)
            {
                _handleTable.AddRef(existing.Handle);
                return existing.Handle;
            }
        }

        // 先檢查包
        if (_packManager.IsAssetInPackage(guid))
        {
            // 包內資產走同步路徑（MemoryMappedFile 已經很快）
            return LoadAssetSync(guid, typeId);
        }

        // 提交到 StreamingManager
        if (_streaming == null)
            return LoadAssetSync(guid, typeId);

        return await _streaming.SubmitRequestAsync(guid, typeId, priority, 0f, ct)
            .ConfigureAwait(false);
    }

    /* ======================== 完成回調 ======================== */

    /// <summary>
    /// StreamingManager 完成回調（在主線程 Tick 中調用）。
    /// </summary>
    private void OnLoadCompleted(LoadResult result)
    {
        // 注意：已在 _lock 內調用（由 Tick 持有）
        if (!result.Success || result.Data == null)
        {
            result.CompletionSource?.TrySetException(
                new InvalidOperationException(result.ErrorMessage ?? "加載失敗"));
            return;
        }

        // 查找或建立 AssetEntry
        if (!_guidToEntry.TryGetValue(result.Guid.Low, out var entry))
        {
            entry = new AssetEntry
            {
                Guid = result.Guid,
                TypeId = result.TypeId,
            };
        }

        entry.Data = result.Data;

        // 解析 .nnasset
        if (!ParseNnassetData(entry))
        {
            entry.State = AssetState.Failed;
            result.CompletionSource?.TrySetException(
                new InvalidOperationException("解析 .nnasset 失敗"));
            return;
        }

        // 分配 Handle（如果尚未分配）
        if (entry.Handle == 0)
        {
            var rawHandle = _handleTable.Allocate(entry, entry.TypeId);
            entry.Handle = rawHandle;
        }

        entry.State = AssetState.Loaded;

        // 更新索引
        _guidToEntry[result.Guid.Low] = entry;
        _handleToEntry[entry.Handle] = entry;

        // 更新快取
        _cache.Touch(result.Guid, entry.Data.Length, entry.Handle);

        // 通知調用方
        result.CompletionSource?.TrySetResult(entry.Handle);
    }

    /* ======================== 卸載 ======================== */

    /// <summary>卸載資產（按 Handle）。</summary>
    public void UnloadAsset(ulong rawHandle)
    {
        lock (_lock)
        {
            if (!_handleToEntry.TryGetValue(rawHandle, out var entry))
                return;

            var prev = Interlocked.Decrement(ref entry.RefCount);
            if (prev > 0) return; // 引用計數未降至 0

            _cache.Remove(entry.Guid);
            _guidToEntry.Remove(entry.Guid.Low);
            _handleToEntry.Remove(rawHandle);
            _handleTable.Free(rawHandle);
        }
    }

    /// <summary>卸載資產（按 GUID）。</summary>
    public void UnloadAssetByGuid(GUID guid)
    {
        lock (_lock)
        {
            if (!_guidToEntry.TryGetValue(guid.Low, out var entry))
                return;

            var prev = Interlocked.Decrement(ref entry.RefCount);
            if (prev > 0) return;

            _cache.Remove(guid);
            _handleToEntry.Remove(entry.Handle);
            _guidToEntry.Remove(guid.Low);
            _handleTable.Free(entry.Handle);
        }
    }

    /* ======================== 查詢 ======================== */

    /// <summary>資產是否已載入。</summary>
    public bool IsLoaded(ulong rawHandle)
    {
        return _handleTable.IsAlive(rawHandle);
    }

    /// <summary>資產是否正在加載中。</summary>
    public bool IsLoading(ulong rawHandle)
    {
        lock (_lock)
        {
            return _handleToEntry.TryGetValue(rawHandle, out var entry)
                && entry.State == AssetState.Loading;
        }
    }

    /// <summary>依 GUID 取得已載入資產的 Handle。</summary>
    public ulong GetLoadedAsset(GUID guid)
    {
        lock (_lock)
        {
            if (_guidToEntry.TryGetValue(guid.Low, out var entry)
                && entry.State == AssetState.Loaded)
                return entry.Handle;
            return 0;
        }
    }

    /// <summary>依 Handle 取得 GUID。</summary>
    public GUID GetGuidByHandle(ulong rawHandle)
    {
        lock (_lock)
        {
            return _handleToEntry.TryGetValue(rawHandle, out var entry)
                ? entry.Guid
                : GUID.Zero;
        }
    }

    /* ======================== 引用計數 ======================== */

    /// <summary>增加引用計數。</summary>
    public void AddRef(ulong rawHandle)
    {
        lock (_lock)
        {
            if (_handleToEntry.TryGetValue(rawHandle, out var entry))
            {
                Interlocked.Increment(ref entry.RefCount);
                _handleTable.AddRef(rawHandle);
            }
        }
    }

    /// <summary>減少引用計數（計數歸零時自動卸載）。</summary>
    public void ReleaseRef(ulong rawHandle)
    {
        UnloadAsset(rawHandle);
    }

    /// <summary>取得引用計數。</summary>
    public uint GetRefCount(ulong rawHandle)
    {
        return _handleTable.GetRefCount(rawHandle);
    }

    /* ======================== 資料存取 ======================== */

    /// <summary>取得資產原始資料（Span 切片，零拷貝）。</summary>
    public ReadOnlySpan<byte> GetAssetData(ulong rawHandle)
    {
        lock (_lock)
        {
            if (!_handleToEntry.TryGetValue(rawHandle, out var entry) || entry.Data.Length == 0)
                return ReadOnlySpan<byte>.Empty;
            return entry.Data;
        }
    }

    /// <summary>取得資產資料大小（位元組）。</summary>
    public long GetAssetDataSize(ulong rawHandle)
    {
        lock (_lock)
        {
            return _handleToEntry.TryGetValue(rawHandle, out var entry) ? entry.Data.Length : 0;
        }
    }

    /// <summary>取得 blob 數量。</summary>
    public int GetBlobCount(ulong rawHandle)
    {
        lock (_lock)
        {
            return _handleToEntry.TryGetValue(rawHandle, out var entry) ? entry.Blobs.Length : 0;
        }
    }

    /// <summary>取得指定 blob 的資料（Span 切片，零拷貝）。</summary>
    public ReadOnlySpan<byte> GetBlobData(ulong rawHandle, int index)
    {
        lock (_lock)
        {
            if (!_handleToEntry.TryGetValue(rawHandle, out var entry))
                return ReadOnlySpan<byte>.Empty;
            if (index < 0 || index >= entry.Blobs.Length)
                return ReadOnlySpan<byte>.Empty;

            ref var blob = ref entry.Blobs[index];
            var absOffset = entry.PayloadOffset + (int)blob.Offset;
            var size = (int)(blob.CompressedSize > 0 ? blob.CompressedSize : blob.Size);

            if (absOffset + size > entry.Data.Length)
                return ReadOnlySpan<byte>.Empty;
            return entry.Data.AsSpan(absOffset, size);
        }
    }

    /// <summary>取得指定 blob 的大小。</summary>
    public long GetBlobSize(ulong rawHandle, int index)
    {
        lock (_lock)
        {
            if (!_handleToEntry.TryGetValue(rawHandle, out var entry))
                return 0;
            if (index < 0 || index >= entry.Blobs.Length)
                return 0;
            return (long)entry.Blobs[index].Size;
        }
    }

    /// <summary>取得指定類型的第一個 blob 資料。</summary>
    public ReadOnlySpan<byte> GetBlobByType(ulong rawHandle, uint blobType)
    {
        lock (_lock)
        {
            if (!_handleToEntry.TryGetValue(rawHandle, out var entry))
                return ReadOnlySpan<byte>.Empty;

            for (int i = 0; i < entry.Blobs.Length; i++)
            {
                if (entry.Blobs[i].BlobType == blobType)
                {
                    ref var blob = ref entry.Blobs[i];
                    var absOffset = entry.PayloadOffset + (int)blob.Offset;
                    var size = (int)(blob.CompressedSize > 0 ? blob.CompressedSize : blob.Size);
                    if (absOffset + size > entry.Data.Length)
                        return ReadOnlySpan<byte>.Empty;
                    return entry.Data.AsSpan(absOffset, size);
                }
            }
            return ReadOnlySpan<byte>.Empty;
        }
    }

    /* ======================== Unsafe Blob 指针（Native Interop 专用） ======================== */

    /// <summary>
    /// 取得指定 blob 的原始数据指针（unsafe，用于 Native Interop）。
    ///
    /// 返回的指针指向托管 byte[] 内部，调用方必须在 GC 移动对象前使用完毕。
    /// 推荐在 fixed 块内使用，或配合 <see cref="GetBlobDataPinned"/> 使用。
    /// </summary>
    public unsafe void* GetBlobDataPtr(ulong rawHandle, int index)
    {
        lock (_lock)
        {
            if (!_handleToEntry.TryGetValue(rawHandle, out var entry))
                return null;
            if (index < 0 || index >= entry.Blobs.Length)
                return null;

            ref var blob = ref entry.Blobs[index];
            var absOffset = entry.PayloadOffset + (int)blob.Offset;
            var size = (int)(blob.CompressedSize > 0 ? blob.CompressedSize : blob.Size);

            if (absOffset + size > entry.Data.Length)
                return null;

            fixed (byte* p = &entry.Data[absOffset])
                return p;
        }
    }

    /// <summary>
    /// 取得指定 blob 的数据大小（用于 Native Interop 配合 <see cref="GetBlobDataPtr"/>）。
    /// </summary>
    public long GetBlobDataSize(ulong rawHandle, int index)
    {
        lock (_lock)
        {
            if (!_handleToEntry.TryGetValue(rawHandle, out var entry))
                return 0;
            if (index < 0 || index >= entry.Blobs.Length)
                return 0;

            ref var blob = ref entry.Blobs[index];
            return (long)(blob.CompressedSize > 0 ? blob.CompressedSize : blob.Size);
        }
    }

    /* ======================== 包管理 ======================== */

    /// <summary>掛載 .nnpack 包。</summary>
    public bool MountPackage(string packPath)
    {
        return _packManager.MountPackage(packPath);
    }

    /// <summary>卸載 .nnpack 包。</summary>
    public void UnmountPackage(string packPath)
    {
        _packManager.UnmountPackage(packPath);
    }

    /// <summary>資產是否在任何已掛載的包中。</summary>
    public bool IsAssetInPackage(GUID guid)
    {
        return _packManager.IsAssetInPackage(guid);
    }

    /* ======================== Hot Reload ======================== */

    /// <summary>標記資產需要重載。</summary>
    public void MarkForReload(GUID guid)
    {
        lock (_lock)
        {
            if (_guidToEntry.TryGetValue(guid.Low, out var entry))
                entry.NeedsReload = true;
        }
    }

    /// <summary>重新載入所有被標記的資產。</summary>
    public void ReloadMarkedAssets()
    {
        lock (_lock)
        {
            foreach (var entry in _guidToEntry.Values)
            {
                if (!entry.NeedsReload) continue;
                entry.NeedsReload = false;

                if (ReadNnAsset(entry.SourcePath, entry))
                    entry.State = AssetState.Loaded;
                else
                    entry.State = AssetState.Failed;
            }
        }
    }

    /* ======================== 統計 ======================== */

    /// <summary>已載入的資產數量。</summary>
    public ulong LoadedAssetCount => (ulong)_handleTable.AllocatedCount;

    /// <summary>快取記憶體使用量（位元組）。</summary>
    public long TotalMemoryUsage => _cache.CurrentUsage;

    /* ======================== 子系統存取 ======================== */

    /// <summary>Handle 分配表。</summary>
    public HandleTable HandleTable => _handleTable;

    /// <summary>LRU 快取。</summary>
    public AssetCache Cache => _cache;

    /// <summary>Pack 管理器。</summary>
    public PackManager PackMgr => _packManager;

    /// <summary>資產註冊表。</summary>
    public AssetRegistry Registry => _assetRegistry;

    /* ======================== 內部工具 ======================== */

    /// <summary>
    /// 從檔案讀取 .nnasset 並解析到 AssetEntry。
    /// </summary>
    private bool ReadNnAsset(string? path, AssetEntry entry)
    {
        if (string.IsNullOrEmpty(path) || !File.Exists(path))
            return false;

        try
        {
            entry.Data = File.ReadAllBytes(path);
        }
        catch
        {
            return false;
        }

        return ParseNnassetData(entry);
    }

    /// <summary>
    /// 解析 .nnasset 資料到 AssetEntry（Header + 依賴 + Blob 描述符）。
    /// </summary>
    private bool ParseNnassetData(AssetEntry entry)
    {
        var data = entry.Data.AsSpan();
        if (data.Length < (int)AssetFormat.HeaderSize)
            return false;

        var header = MemoryMarshal.Read<AssetHeader>(data);
        if (!AssetFormat.IsHeaderValid(in header))
            return false;

        entry.TypeId = header.TypeId;
        entry.PayloadOffset = (int)header.PayloadOffset;

        // 解析依賴
        entry.Dependencies.Clear();
        if (header.DependencyCount > 0)
        {
            var depOffset = (int)header.DependencyOffset;
            var depSize = (int)header.DependencyCount * 16; // sizeof(GUID) = 16
            if (depOffset + depSize <= data.Length)
            {
                var deps = MemoryMarshal.Cast<byte, GUID>(data.Slice(depOffset, depSize));
                foreach (var dep in deps)
                    entry.Dependencies.Add(dep);
            }
        }

        // 解析 Blob 描述符
        if (header.BlobCount > 0)
        {
            var blobOffset = (int)header.BlobTableOffset;
            var blobSize = (int)header.BlobCount * 32; // sizeof(BlobDescriptor) = 32
            if (blobOffset + blobSize <= data.Length)
            {
                entry.Blobs = MemoryMarshal.Cast<byte, BlobDescriptor>(
                    data.Slice(blobOffset, blobSize)).ToArray();
            }
            else
            {
                entry.Blobs = Array.Empty<BlobDescriptor>();
            }
        }
        else
        {
            entry.Blobs = Array.Empty<BlobDescriptor>();
        }

        return true;
    }

    /// <summary>
    /// 解析資產路徑（GUID → 檔案系統路徑）。
    /// 與 C++ NNAssetManager::ResolveAssetPath 對齊。
    /// </summary>
    private string ResolveAssetPath(GUID guid)
    {
        var hex = guid.ToHexString();
        var prefix = hex[..2];
        return Path.Combine(_assetRoot, "Imported", prefix, hex + ".nnasset");
    }
}
