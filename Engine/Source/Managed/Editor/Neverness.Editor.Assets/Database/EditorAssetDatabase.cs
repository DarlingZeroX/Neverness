using System.Text;
using Neverness.Runtime.Assets;
using Neverness.Runtime.VFS;

namespace Neverness.Editor.Assets;

/// <summary>
/// Editor 資產資料庫（工業級）。
///
/// 職責：
///   - GUID ↔ VirtualPath 雙向映射
///   - 型別索引（GUID → TypeId）
///   - 標籤索引（Label → GUID[]）
///   - 髒標記追蹤
///   - 序列化快取（Library/Cache/AssetDatabase.cache）
///   - 增量掃描（content hash 比對）
///   - .meta 檔案管理
///
/// 設計原則：
///   - Editor-only，不暴露給 Runtime
///   - 支援 10w+ 資產
///   - Thread-safe
///
/// @threadsafe 所有公共方法使用 s_lock 保護共享狀態，
///   粗粒度鎖對 Editor 場景已足夠（導入為偶發操作）。
/// </summary>
public static class EditorAssetDatabase
{
    private static readonly object s_lock = new();

    /* === 索引 === */
    private static readonly Dictionary<NVirtualPath, GUID> s_pathToGuid = new();
    private static readonly Dictionary<string, NVirtualPath> s_guidToPath = new();
    private static readonly Dictionary<NVirtualPath, NPath> s_pathToSourcePath = new();
    private static readonly Dictionary<string, ulong> s_guidToTypeId = new();
    private static readonly Dictionary<string, HashSet<string>> s_labelToGuids = new(StringComparer.OrdinalIgnoreCase);
    private static readonly Dictionary<string, HashSet<string>> s_guidToLabels = new();
    private static readonly HashSet<string> s_dirtyGuids = new();

    /* === 狀態 === */
    private static bool s_initialized;
    private static bool s_cacheDirty;
    private static NPath s_assetsRoot;
    private static NPath s_libraryRoot;

    /* ======================== 初始化 ======================== */

    /// <summary>初始化資料庫，設定 Assets 和 Library 根目錄。</summary>
    public static void Initialize(NPath assetsRoot, NPath libraryRoot)
    {
        lock (s_lock)
        {
            s_assetsRoot = assetsRoot;
            s_libraryRoot = libraryRoot;
            s_initialized = true;

            /* 嘗試載入快取 */
            TryLoadCache();

            /* 将快取中的所有资产同步到 Native NNAssetRegistry */
            SyncCacheToNative();
        }
    }

    /// <summary>是否已初始化。</summary>
    public static bool IsInitialized
    {
        get { lock (s_lock) return s_initialized; }
    }

    /* ======================== GUID ↔ Path ======================== */

    /// <summary>登記資產（GUID ↔ 虛擬路徑）。</summary>
    public static bool Register(NVirtualPath virtualPath, GUID guid, ulong typeId = 0)
    {
        lock (s_lock)
        {
            EnsureInitialized();

            if (virtualPath.IsEmpty || guid.IsZero)
                return false;

            /* 如果路徑已存在且 GUID 不同，先移除舊映射 */
            if (s_pathToGuid.TryGetValue(virtualPath, out var oldGuid) && oldGuid != guid)
            {
                RemoveFromIndices(oldGuid, virtualPath);
            }

            /* 如果 GUID 已存在且路徑不同，先移除舊映射 */
            var guidKey = guid.ToHexString();
            if (s_guidToPath.TryGetValue(guidKey, out var oldPath) && oldPath != virtualPath)
            {
                s_pathToGuid.Remove(oldPath);
            }

            s_pathToGuid[virtualPath] = guid;
            s_guidToPath[guidKey] = virtualPath;

            if (typeId != 0)
                s_guidToTypeId[guidKey] = typeId;

            s_cacheDirty = true;

            /* 同步到 Runtime AssetDatabase */
            Console.WriteLine($"[EditorAssetDatabase] 同步到 Runtime: path={virtualPath.FullPath}, guid={guid}");
            Neverness.Runtime.Assets.AssetDatabase.Register(virtualPath, guid);

            return true;
        }
    }

    /// <summary>依虛擬路徑查詢 GUID。</summary>
    public static bool TryGetGuid(NVirtualPath virtualPath, out GUID guid)
    {
        lock (s_lock)
        {
            guid = GUID.Zero;
            if (!s_initialized) return false;
            return s_pathToGuid.TryGetValue(virtualPath, out guid);
        }
    }

    /// <summary>依 GUID 查詢虛擬路徑。</summary>
    public static bool TryGetPath(GUID guid, out NVirtualPath path)
    {
        lock (s_lock)
        {
            path = default;
            if (!s_initialized || guid.IsZero) return false;
            return s_guidToPath.TryGetValue(guid.ToHexString(), out path);
        }
    }

    /// <summary>依路徑查詢對應的 source asset 路徑。</summary>
    public static bool TryGetSourcePath(NVirtualPath virtualPath, out NPath sourcePath)
    {
        lock (s_lock)
        {
            sourcePath = default;
            if (!s_initialized) return false;
            return s_pathToSourcePath.TryGetValue(virtualPath, out sourcePath);
        }
    }

    /// <summary>設定 source asset 路徑映射。</summary>
    public static void SetSourcePath(NVirtualPath virtualPath, NPath sourcePath)
    {
        lock (s_lock)
        {
            s_pathToSourcePath[virtualPath] = sourcePath;
        }
    }

    /// <summary>資產是否存在。</summary>
    public static bool Exists(NVirtualPath virtualPath)
    {
        lock (s_lock) return s_initialized && s_pathToGuid.ContainsKey(virtualPath);
    }

    /// <summary>資產是否存在。</summary>
    public static bool Exists(GUID guid)
    {
        lock (s_lock) return s_initialized && !guid.IsZero && s_guidToPath.ContainsKey(guid.ToHexString());
    }

    /* ======================== 枚舉 ======================== */

    /// <summary>所有已登記的 GUID。</summary>
    public static IReadOnlyList<GUID> AllAssets
    {
        get
        {
            lock (s_lock)
            {
                return s_pathToGuid.Values.ToList();
            }
        }
    }

    /// <summary>已登記資產數量。</summary>
    public static int AssetCount
    {
        get { lock (s_lock) return s_pathToGuid.Count; }
    }

    /// <summary>依型別查詢資產。</summary>
    public static IReadOnlyList<GUID> FindAssetsByType(string typeName)
    {
        lock (s_lock)
        {
            var result = new List<GUID>();
            /* TODO: TypeIndex 需要 typeName → typeId 的映射 */
            return result;
        }
    }

    /* ======================== 標籤 ======================== */

    /// <summary>為資產添加標籤。</summary>
    public static void AddLabel(GUID guid, string label)
    {
        lock (s_lock)
        {
            if (guid.IsZero || string.IsNullOrWhiteSpace(label)) return;

            var guidKey = guid.ToHexString();

            if (!s_labelToGuids.TryGetValue(label, out var guids))
            {
                guids = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
                s_labelToGuids[label] = guids;
            }
            guids.Add(guidKey);

            if (!s_guidToLabels.TryGetValue(guidKey, out var labels))
            {
                labels = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
                s_guidToLabels[guidKey] = labels;
            }
            labels.Add(label);
            s_cacheDirty = true;
        }
    }

    /// <summary>移除資產標籤。</summary>
    public static void RemoveLabel(GUID guid, string label)
    {
        lock (s_lock)
        {
            if (guid.IsZero || string.IsNullOrWhiteSpace(label)) return;

            var guidKey = guid.ToHexString();

            if (s_labelToGuids.TryGetValue(label, out var guids))
                guids.Remove(guidKey);

            if (s_guidToLabels.TryGetValue(guidKey, out var labels))
                labels.Remove(label);
            s_cacheDirty = true;
        }
    }

    /// <summary>取得資產的所有標籤。</summary>
    public static IReadOnlyList<string> GetLabels(GUID guid)
    {
        lock (s_lock)
        {
            if (guid.IsZero) return Array.Empty<string>();
            var guidKey = guid.ToHexString();
            return s_guidToLabels.TryGetValue(guidKey, out var labels)
                ? labels.ToList()
                : Array.Empty<string>();
        }
    }

    /// <summary>依標籤查詢資產。</summary>
    public static IReadOnlyList<GUID> FindAssetsByLabel(string label)
    {
        lock (s_lock)
        {
            if (s_labelToGuids.TryGetValue(label, out var guids))
                return guids.Select(g => GUID.Parse(g)).ToList();
            return Array.Empty<GUID>();
        }
    }

    /* ======================== 髒標記 ======================== */

    /// <summary>標記資產為髒。</summary>
    public static void MarkDirty(GUID guid)
    {
        lock (s_lock)
        {
            if (!guid.IsZero)
                s_dirtyGuids.Add(guid.ToHexString());
        }
    }

    /// <summary>資產是否為髒。</summary>
    public static bool IsDirty(GUID guid)
    {
        lock (s_lock) return !guid.IsZero && s_dirtyGuids.Contains(guid.ToHexString());
    }

    /// <summary>清除所有髒標記。</summary>
    public static void ClearDirtyFlags()
    {
        lock (s_lock) s_dirtyGuids.Clear();
    }

    /// <summary>取得所有髒資產。</summary>
    public static IReadOnlyList<GUID> GetDirtyAssets()
    {
        lock (s_lock) return s_dirtyGuids.Select(g => GUID.Parse(g)).ToList();
    }

    /* ======================== 移動/刪除 ======================== */

    /// <summary>移動資產（GUID 不變）。</summary>
    public static void MoveAsset(NVirtualPath fromPath, NVirtualPath toPath)
    {
        lock (s_lock)
        {
            if (!s_pathToGuid.TryGetValue(fromPath, out var guid))
                return;

            s_pathToGuid.Remove(fromPath);
            s_pathToGuid[toPath] = guid;

            var guidKey = guid.ToHexString();
            s_guidToPath[guidKey] = toPath;

            /* 移動 source path 映射 */
            if (s_pathToSourcePath.TryGetValue(fromPath, out var srcPath))
            {
                s_pathToSourcePath.Remove(fromPath);
                s_pathToSourcePath[toPath] = srcPath;
            }

            s_cacheDirty = true;
        }
    }

    /// <summary>刪除資產。</summary>
    public static void DeleteAsset(NVirtualPath virtualPath)
    {
        lock (s_lock)
        {
            RemoveFromIndices(virtualPath);
        }
    }

    /* ======================== 型別索引 ======================== */

    /// <summary>取得資產型別 ID。</summary>
    public static ulong GetTypeId(GUID guid)
    {
        lock (s_lock)
        {
            if (guid.IsZero) return 0;
            return s_guidToTypeId.TryGetValue(guid.ToHexString(), out var typeId) ? typeId : 0;
        }
    }

    /// <summary>設定資產型別 ID。</summary>
    public static void SetTypeId(GUID guid, ulong typeId)
    {
        lock (s_lock)
        {
            if (!guid.IsZero && typeId != 0)
                s_guidToTypeId[guid.ToHexString()] = typeId;
            s_cacheDirty = true;
        }
    }

    /* ======================== 元数据查询 ======================== */

    /// <summary>依虚拟路径查询资产元数据（GUID + TypeId + .meta 信息）。</summary>
    public static AssetMeta? TryGetMeta(NVirtualPath virtualPath)
    {
        lock (s_lock)
        {
            if (!s_initialized) return null;
            if (!s_pathToGuid.TryGetValue(virtualPath, out var guid)) return null;

            var typeId = s_guidToTypeId.TryGetValue(guid.ToHexString(), out var t) ? t : 0;

            // 尝试从 .meta 读取额外信息
            AssetMeta? fileMeta = null;
            if (s_pathToSourcePath.TryGetValue(virtualPath, out var sourcePath))
            {
                fileMeta = MetaFileManager.ReadMeta(sourcePath);
            }

            return new AssetMeta
            {
                Guid = guid,
                AssetTypeId = typeId,
                Importer = fileMeta?.Importer ?? "DefaultImporter",
                ImportSettings = fileMeta?.ImportSettings ?? new Dictionary<string, string>(),
                Labels = fileMeta?.Labels ?? new List<string>(),
                Dependencies = fileMeta?.Dependencies ?? new List<string>(),
            };
        }
    }

    /// <summary>依 GUID 查询资产元数据。</summary>
    public static AssetMeta? TryGetMeta(GUID guid)
    {
        lock (s_lock)
        {
            if (!s_initialized || guid.IsZero) return null;
            var guidKey = guid.ToHexString();
            if (!s_guidToPath.TryGetValue(guidKey, out var virtualPath)) return null;

            var typeId = s_guidToTypeId.TryGetValue(guidKey, out var t) ? t : 0;

            AssetMeta? fileMeta = null;
            if (s_pathToSourcePath.TryGetValue(virtualPath, out var sourcePath))
            {
                fileMeta = MetaFileManager.ReadMeta(sourcePath);
            }

            return new AssetMeta
            {
                Guid = guid,
                AssetTypeId = typeId,
                Importer = fileMeta?.Importer ?? "DefaultImporter",
                ImportSettings = fileMeta?.ImportSettings ?? new Dictionary<string, string>(),
                Labels = fileMeta?.Labels ?? new List<string>(),
                Dependencies = fileMeta?.Dependencies ?? new List<string>(),
            };
        }
    }

    /* ======================== 快取序列化 ======================== */

    /// <summary>若髒則儲存快取至磁碟（Tick 批量保存用）。</summary>
    public static void SaveIfDirty()
    {
        lock (s_lock)
        {
            if (!s_cacheDirty) return;
            s_cacheDirty = false;
            SaveCacheInternal();
        }
    }

    /// <summary>儲存快取至磁碟。</summary>
    public static void SaveCache()
    {
        lock (s_lock)
        {
            if (!s_initialized) return;
            SaveCacheInternal();
        }
    }

    /// <summary>載入快取。</summary>
    public static void LoadCache()
    {
        lock (s_lock)
        {
            if (!s_initialized) return;
            TryLoadCache();
        }
    }

    /// <summary>使快取失效。</summary>
    public static void InvalidateCache()
    {
        lock (s_lock)
        {
            if (!s_initialized) return;
            var cachePath = GetCachePath();
            if (File.Exists(cachePath.FullPath))
                File.Delete(cachePath.FullPath);
        }
    }

    /* ======================== 依赖查询 ======================== */

    /// <summary>取得资产的直接依赖 GUID 列表。</summary>
    public static IReadOnlyCollection<GUID> GetDependencies(GUID guid)
    {
        return ImportPipeline.DependencyGraph.GetDirectDependencies(guid);
    }

    /// <summary>取得资产的虚拝路径列表形式的依赖。</summary>
    public static IReadOnlyList<NVirtualPath> GetDependencies(NVirtualPath virtualPath)
    {
        lock (s_lock)
        {
            if (!s_pathToGuid.TryGetValue(virtualPath, out var guid))
                return Array.Empty<NVirtualPath>();

            var depGuids = ImportPipeline.DependencyGraph.GetDirectDependencies(guid);
            var result = new List<NVirtualPath>();
            foreach (var dg in depGuids)
            {
                var dk = dg.ToHexString();
                if (s_guidToPath.TryGetValue(dk, out var depPath))
                    result.Add(depPath);
            }
            return result;
        }
    }

    /// <summary>取得资产的所有递归依赖。</summary>
    public static IReadOnlyCollection<GUID> GetAllDependencies(GUID guid)
    {
        return ImportPipeline.DependencyGraph.GetAllDependencies(guid);
    }

    /// <summary>取得资产的直接反向依赖（谁引用了此资产）。</summary>
    public static IReadOnlyCollection<GUID> GetReverseDependencies(GUID guid)
    {
        return ImportPipeline.DependencyGraph.GetDirectReverseDependencies(guid);
    }

    /// <summary>取得资产的所有递归反向依赖。</summary>
    public static IReadOnlyCollection<GUID> GetAllReverseDependencies(GUID guid)
    {
        return ImportPipeline.DependencyGraph.GetAllReverseDependencies(guid);
    }

    /// <summary>检测依赖图中是否存在环。</summary>
    public static bool HasDependencyCycle()
    {
        return ImportPipeline.DependencyGraph.HasCycle();
    }

    /* ======================== 清理 ======================== */

    /// <summary>清除所有索引（測試用）。</summary>
    public static void ClearForTesting()
    {
        lock (s_lock)
        {
            s_pathToGuid.Clear();
            s_guidToPath.Clear();
            s_pathToSourcePath.Clear();
            s_guidToTypeId.Clear();
            s_labelToGuids.Clear();
            s_guidToLabels.Clear();
            s_dirtyGuids.Clear();
        }
    }

    /* ======================== Native 同步 ======================== */

    /// <summary>将快取中的所有资产注册到 Native NNAssetRegistry（启动时调用一次）。</summary>
    private static void SyncCacheToNative()
    {
        var count = 0;
        foreach (var kvp in s_pathToGuid)
        {
            if (Neverness.Runtime.Assets.AssetDatabase.Register(kvp.Key, kvp.Value))
                count++;
        }
        Console.WriteLine($"[EditorAssetDatabase] SyncCacheToNative: 同步 {count}/{s_pathToGuid.Count} 个资产到 NNAssetRegistry");
    }

    /* ======================== 內部實作 ======================== */

    private static void EnsureInitialized()
    {
        if (!s_initialized)
            throw new InvalidOperationException("EditorAssetDatabase 未初始化。請先呼叫 Initialize(assetsRoot, libraryRoot)。");
    }

    private static void RemoveFromIndices(GUID guid, NVirtualPath? knownPath = null)
    {
        var guidKey = guid.ToHexString();

        /* 解析路徑：優先 knownPath，其次從索引查找 */
        NVirtualPath? path = knownPath;
        if (path == null && s_guidToPath.TryGetValue(guidKey, out var resolved))
            path = resolved;

        if (path is { IsEmpty: false })
        {
            s_pathToGuid.Remove(path.Value);
            s_pathToSourcePath.Remove(path.Value);
        }

        s_guidToPath.Remove(guidKey);
        s_guidToTypeId.Remove(guidKey);

        /* 移除標籤 */
        if (s_guidToLabels.TryGetValue(guidKey, out var labels))
        {
            foreach (var label in labels)
            {
                if (s_labelToGuids.TryGetValue(label, out var guids))
                    guids.Remove(guidKey);
            }
            s_guidToLabels.Remove(guidKey);
        }

        s_dirtyGuids.Remove(guidKey);
        s_cacheDirty = true;
    }

    private static void RemoveFromIndices(NVirtualPath virtualPath)
    {
        if (s_pathToGuid.TryGetValue(virtualPath, out var guid))
            RemoveFromIndices(guid, virtualPath);
    }

    private static NPath GetCachePath()
    {
        return s_libraryRoot.Combine("Cache/AssetDatabase.cache");
    }

    private static void SaveCacheInternal()
    {
        try
        {
            var cachePath = GetCachePath();
            var dir = Path.GetDirectoryName(cachePath.FullPath);
            if (!string.IsNullOrEmpty(dir) && !Directory.Exists(dir))
                Directory.CreateDirectory(dir);

            using var stream = File.Create(cachePath.FullPath);
            using var writer = new BinaryWriter(stream, Encoding.UTF8, leaveOpen: true);

            /* Header: magic + version */
            writer.Write(0x4E4E4442u); /* 'NNDB' */
            writer.Write(1u);          /* version */

            /* Path → Guid 表 */
            writer.Write(s_pathToGuid.Count);
            foreach (var (path, g) in s_pathToGuid)
            {
                writer.Write(path.FullPath);
                writer.Write(g.High);
                writer.Write(g.Low);
            }

            /* Guid → TypeId 表 */
            writer.Write(s_guidToTypeId.Count);
            foreach (var (guidKey, typeId) in s_guidToTypeId)
            {
                writer.Write(guidKey);
                writer.Write(typeId);
            }

            /* Label 索引 */
            writer.Write(s_labelToGuids.Count);
            foreach (var (label, guids) in s_labelToGuids)
            {
                writer.Write(label);
                writer.Write(guids.Count);
                foreach (var g in guids)
                    writer.Write(g);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[EditorAssetDatabase] 儲存快取失敗: {ex.Message}");
        }
    }

    private static void TryLoadCache()
    {
        var cachePath = GetCachePath();
        if (!File.Exists(cachePath.FullPath))
            return;

        try
        {
            using var stream = File.OpenRead(cachePath.FullPath);
            using var reader = new BinaryReader(stream, Encoding.UTF8, leaveOpen: true);

            var magic = reader.ReadUInt32();
            var version = reader.ReadUInt32();
            if (magic != 0x4E4E4442u || version != 1u)
                return; /* 格式不匹配，忽略 */

            /* Path → Guid 表 */
            var count = reader.ReadInt32();
            for (var i = 0; i < count; i++)
            {
                var pathStr = reader.ReadString();
                var high = reader.ReadUInt64();
                var low = reader.ReadUInt64();
                var guid = new GUID(high, low);
                var vp = new NVirtualPath(pathStr);
                s_pathToGuid[vp] = guid;
                s_guidToPath[guid.ToHexString()] = vp;
            }

            /* Guid → TypeId 表 */
            count = reader.ReadInt32();
            for (var i = 0; i < count; i++)
            {
                var guidKey = reader.ReadString();
                var typeId = reader.ReadUInt64();
                s_guidToTypeId[guidKey] = typeId;
            }

            /* Label 索引 */
            count = reader.ReadInt32();
            for (var i = 0; i < count; i++)
            {
                var label = reader.ReadString();
                var guidCount = reader.ReadInt32();
                var guids = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
                for (var j = 0; j < guidCount; j++)
                {
                    var g = reader.ReadString();
                    guids.Add(g);

                    /* 同時更新反向索引 */
                    if (!s_guidToLabels.TryGetValue(g, out var labels))
                    {
                        labels = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
                        s_guidToLabels[g] = labels;
                    }
                    labels.Add(label);
                }
                s_labelToGuids[label] = guids;
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[EditorAssetDatabase] 載入快取失敗: {ex.Message}");
            /* 快取損壞，清除 */
            s_pathToGuid.Clear();
            s_guidToPath.Clear();
            s_guidToTypeId.Clear();
            s_labelToGuids.Clear();
            s_guidToLabels.Clear();
        }
    }
}
