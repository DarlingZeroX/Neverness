using Neverness.Runtime.Assets;
using Neverness.Runtime.VFS;

namespace Neverness.Editor.Assets;

/// <summary>
/// EditorAssetDatabase — 依賴查詢 + 元數據 + Native 同步。
/// </summary>
public static partial class EditorAssetDatabase
{
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
}
