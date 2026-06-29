using Neverness.Runtime.Assets;
using Neverness.Runtime.VFS;

namespace Neverness.Editor.Assets;

/// <summary>
/// EditorAssetDatabase — GUID ↔ Path 映射 + TypeIndex + Move/Delete。
/// </summary>
public static partial class EditorAssetDatabase
{
    /* === 索引 === */
    private static readonly Dictionary<NVirtualPath, GUID> s_pathToGuid = new();
    private static readonly Dictionary<string, NVirtualPath> s_guidToPath = new();
    private static readonly Dictionary<NVirtualPath, NPath> s_pathToSourcePath = new();
    private static readonly Dictionary<string, ulong> s_guidToTypeId = new();

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
}
