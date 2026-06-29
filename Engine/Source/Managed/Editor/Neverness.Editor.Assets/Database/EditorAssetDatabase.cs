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
public static partial class EditorAssetDatabase
{
    private static readonly object s_lock = new();

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
}
