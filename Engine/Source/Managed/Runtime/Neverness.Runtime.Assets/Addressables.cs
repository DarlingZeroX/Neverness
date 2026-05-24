namespace Neverness.Runtime.Assets;

/// <summary>
/// Addressable Runtime API。
///
/// 面向游戏逻辑的资产加载接口，支持：
///   - 按地址（字符串）加载
///   - 按标签批量加载
///   - 按 GUID 加载
///   - 包管理
///
/// 解析流程：
///   Address "character/hero"
///     → 查 Catalog → GUID
///     → 查 GUID → 所在 Package
///     → Package 已挂载？直接返回
///     → Package 未挂载？异步加载 .nnpack
///     → 从 Package 中读取 .nnasset → 返回 Handle
/// </summary>
public static class Addressables
{
    /* 地址 → GUID 的目录映射 */
    private static readonly Dictionary<string, GUID> s_catalog = new(StringComparer.OrdinalIgnoreCase);

    /* GUID → 地址的反向映射 */
    private static readonly Dictionary<string, string> s_guidToAddress = new();

    /* 标签 → GUID 列表 */
    private static readonly Dictionary<string, List<GUID>> s_labelIndex = new(StringComparer.OrdinalIgnoreCase);

    private static readonly object s_lock = new();
    private static bool s_catalogLoaded;

    /* ========== 目录管理 ========== */

    /// <summary>加载 Addressable 目录（从文件）。</summary>
    public static async Task<bool> LoadCatalogAsync(NPath catalogPath)
    {
        try
        {
            if (!File.Exists(catalogPath.FullPath))
                return false;

            var lines = await File.ReadAllLinesAsync(catalogPath.FullPath);
            lock (s_lock)
            {
                s_catalog.Clear();
                s_guidToAddress.Clear();
                s_labelIndex.Clear();

                foreach (var line in lines)
                {
                    if (string.IsNullOrWhiteSpace(line) || line.StartsWith('#'))
                        continue;

                    var parts = line.Split('|');
                    if (parts.Length < 2)
                        continue;

                    var address = parts[0].Trim();
                    var guid = GUID.Parse(parts[1].Trim());

                    if (!guid.IsZero)
                    {
                        s_catalog[address] = guid;
                        s_guidToAddress[guid.ToHexString()] = address;

                        /* 可选第三字段：labels（逗号分隔） */
                        if (parts.Length >= 3 && !string.IsNullOrWhiteSpace(parts[2]))
                        {
                            var labels = parts[2].Split(',', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries);
                            foreach (var label in labels)
                            {
                                if (!s_labelIndex.TryGetValue(label, out var list))
                                {
                                    list = new List<GUID>();
                                    s_labelIndex[label] = list;
                                }

                                list.Add(guid);
                            }
                        }
                    }
                }

                s_catalogLoaded = true;
            }

            Console.WriteLine($"[Addressables] 目录已加载: {s_catalog.Count} 个地址");
            return true;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[Addressables] 目录加载失败: {ex.Message}");
            return false;
        }
    }

    /// <summary>设置目录映射（编程方式）。</summary>
    public static void RegisterAddress(string address, GUID guid)
    {
        lock (s_lock)
        {
            s_catalog[address] = guid;
            s_guidToAddress[guid.ToHexString()] = address;
        }
    }

    /// <summary>移除地址映射。</summary>
    public static void UnregisterAddress(string address)
    {
        lock (s_lock)
        {
            if (s_catalog.TryGetValue(address, out var guid))
            {
                s_guidToAddress.Remove(guid.ToHexString());
                s_catalog.Remove(address);
            }
        }
    }

    /* ========== 标签注册 ========== */

    /// <summary>注册标签映射。</summary>
    public static void RegisterLabel(string label, GUID guid)
    {
        lock (s_lock)
        {
            if (!s_labelIndex.TryGetValue(label, out var list))
            {
                list = new List<GUID>();
                s_labelIndex[label] = list;
            }

            if (!list.Contains(guid))
                list.Add(guid);
        }
    }

    /// <summary>批量注册标签（为一个 GUID 注册多个标签）。</summary>
    public static void RegisterLabels(GUID guid, IEnumerable<string> labels)
    {
        lock (s_lock)
        {
            foreach (var label in labels)
            {
                if (!s_labelIndex.TryGetValue(label, out var list))
                {
                    list = new List<GUID>();
                    s_labelIndex[label] = list;
                }

                if (!list.Contains(guid))
                    list.Add(guid);
            }
        }
    }

    /* ========== 按地址加载 ========== */

    /// <summary>按地址同步加载资产。</summary>
    public static AssetHandle<T> LoadAsset<T>(string address)
    {
        var guid = ResolveAddress(address);
        if (guid.IsZero)
            return AssetHandle<T>.Zero;

        return AssetHandleExtensions.LoadSync(guid).As<T>();
    }

    /// <summary>按地址异步加载资产。</summary>
    public static TaskCompletionSource<ulong> LoadAssetAsync(
        string address,
        LoadPriority priority = LoadPriority.Normal)
    {
        var guid = ResolveAddress(address);
        if (guid.IsZero)
        {
            var failTcs = new TaskCompletionSource<ulong>();
            failTcs.SetException(new KeyNotFoundException($"地址未找到: {address}"));
            return failTcs;
        }

        return AssetHandleExtensions.LoadAsync(guid, priority: priority);
    }

    /* ========== 按标签加载 ========== */

    /// <summary>按标签同步加载所有资产。</summary>
    public static IReadOnlyList<AssetHandle<T>> LoadAssetsByLabel<T>(string label)
    {
        IReadOnlyList<GUID> guids;
        lock (s_lock)
        {
            if (s_labelIndex.TryGetValue(label, out var list))
                guids = list;
            else
                guids = Array.Empty<GUID>();
        }

        var handles = new List<AssetHandle<T>>(guids.Count);
        foreach (var guid in guids)
        {
            var handle = AssetHandleExtensions.LoadSync(guid);
            handles.Add(handle.As<T>());
        }
        return handles;
    }

    /// <summary>按标签异步加载所有资产。</summary>
    public static List<TaskCompletionSource<ulong>> LoadAssetsByLabelAsync(
        string label,
        LoadPriority priority = LoadPriority.Normal)
    {
        IReadOnlyList<GUID> guids;
        lock (s_lock)
        {
            if (s_labelIndex.TryGetValue(label, out var list))
                guids = list;
            else
                guids = Array.Empty<GUID>();
        }

        var tasks = new List<TaskCompletionSource<ulong>>(guids.Count);
        foreach (var guid in guids)
            tasks.Add(AssetHandleExtensions.LoadAsync(guid, priority: priority));
        return tasks;
    }

    /* ========== 按 GUID 加载 ========== */

    /// <summary>按 GUID 同步加载资产。</summary>
    public static AssetHandle<T> LoadAssetByGuid<T>(GUID guid)
    {
        return AssetHandleExtensions.LoadSync(guid).As<T>();
    }

    /// <summary>按 GUID 异步加载资产。</summary>
    public static TaskCompletionSource<ulong> LoadAssetByGuidAsync(
        GUID guid,
        LoadPriority priority = LoadPriority.Normal)
    {
        return AssetHandleExtensions.LoadAsync(guid, priority: priority);
    }

    /* ========== 释放 ========== */

    /// <summary>释放资产。</summary>
    public static void Release(AssetHandle handle)
    {
        AssetHandleExtensions.Unload(handle);
    }

    /// <summary>释放资产（按 GUID）。</summary>
    public static void Release(GUID guid)
    {
        AssetHandleExtensions.Unload(guid);
    }

    /* ========== 查询 ========== */

    /// <summary>通过地址查询 GUID。</summary>
    public static GUID ResolveAddress(string address)
    {
        lock (s_lock)
        {
            return s_catalog.TryGetValue(address, out var guid) ? guid : GUID.Zero;
        }
    }

    /// <summary>通过 GUID 查询地址。</summary>
    public static string? GetAddress(GUID guid)
    {
        lock (s_lock)
        {
            return s_guidToAddress.TryGetValue(guid.ToHexString(), out var addr) ? addr : null;
        }
    }

    /// <summary>目录是否已加载。</summary>
    public static bool IsCatalogLoaded
    {
        get { lock (s_lock) return s_catalogLoaded; }
    }

    /// <summary>目录中的地址数量。</summary>
    public static int AddressCount
    {
        get { lock (s_lock) return s_catalog.Count; }
    }
}

/* ========== AssetHandle 类型转换扩展 ========== */

/// <summary>AssetHandle 类型转换辅助。</summary>
public static class AssetHandleCastExtensions
{
    /// <summary>将非泛型 Handle 转为泛型 Handle（零拷贝，仅类型转换）。</summary>
    public static AssetHandle<T> As<T>(this AssetHandle handle)
    {
        return new AssetHandle<T>(handle.Value);
    }
}
