namespace Neverness.Runtime.Assets.Registry;

/// <summary>
/// 資產註冊表，組合 GuidTable + DependencyTable。
///
/// 提供統一的 GUID↔Path 映射和依賴管理 API。
/// 與 C++ NNAssetRegistry 對應。
/// </summary>
public sealed class AssetRegistry
{
    private readonly GuidTable _guidTable = new();
    private readonly DependencyTable _depTable = new();
    private readonly object _lock = new();

    /// <summary>全域單例。</summary>
    public static AssetRegistry Instance { get; } = new();

    private AssetRegistry() { }

    /* ======================== GUID ↔ Path ======================== */

    /// <summary>
    /// 註冊資產路徑與 GUID 的映射。
    /// </summary>
    /// <returns>是否成功。</returns>
    public bool RegisterAsset(string virtualPath, GUID guid)
    {
        if (string.IsNullOrEmpty(virtualPath) || guid.IsZero)
            return false;

        lock (_lock)
        {
            _guidTable.Register(virtualPath, guid);
        }
        return true;
    }

    /// <summary>
    /// 依 GUID 註銷資產（同時清除依賴）。
    /// </summary>
    public bool UnregisterByGuid(GUID guid)
    {
        if (guid.IsZero) return false;

        lock (_lock)
        {
            var path = _guidTable.UnregisterByGuid(guid);
            if (path == null) return false;
            _depTable.ClearDependencies(guid);
        }
        return true;
    }

    /// <summary>
    /// 依路徑註銷資產（同時清除依賴）。
    /// </summary>
    public bool UnregisterByPath(string virtualPath)
    {
        if (string.IsNullOrEmpty(virtualPath)) return false;

        lock (_lock)
        {
            var guid = _guidTable.UnregisterByPath(virtualPath);
            if (guid == null) return false;
            _depTable.ClearDependencies(guid.Value);
        }
        return true;
    }

    /// <summary>
    /// 依 GUID 解析路徑。
    /// </summary>
    public bool TryResolvePath(GUID guid, out string? path)
    {
        return _guidTable.TryResolvePath(guid, out path);
    }

    /// <summary>
    /// 依路徑解析 GUID。
    /// </summary>
    public bool TryResolveGuid(string virtualPath, out GUID guid)
    {
        return _guidTable.TryResolveGuid(virtualPath, out guid);
    }

    /* ======================== 依賴管理 ======================== */

    /// <summary>
    /// 設定資產的完整依賴列表。
    /// </summary>
    public void SetDependencies(GUID asset, ReadOnlySpan<GUID> deps)
    {
        _depTable.SetDependencies(asset, deps);
    }

    /// <summary>
    /// 添加單條依賴。
    /// </summary>
    public void AddDependency(GUID asset, GUID dependency)
    {
        _depTable.AddDependency(asset, dependency);
    }

    /// <summary>
    /// 移除單條依賴。
    /// </summary>
    public void RemoveDependency(GUID asset, GUID dependency)
    {
        _depTable.RemoveDependency(asset, dependency);
    }

    /// <summary>
    /// 取得依賴數量。
    /// </summary>
    public uint GetDependencyCount(GUID asset)
    {
        return _depTable.GetDependencyCount(asset);
    }

    /// <summary>
    /// 取得指定索引的依賴。
    /// </summary>
    public bool TryGetDependencyAt(GUID asset, uint index, out GUID dependency)
    {
        return _depTable.TryGetDependencyAt(asset, index, out dependency);
    }

    /// <summary>
    /// 取得所有依賴。
    /// </summary>
    public IReadOnlyList<GUID> GetDependencies(GUID asset)
    {
        return _depTable.GetDependencies(asset);
    }

    /// <summary>
    /// 取得反向依賴數量。
    /// </summary>
    public uint GetReverseDependencyCount(GUID asset)
    {
        return _depTable.GetReverseDependencyCount(asset);
    }

    /// <summary>
    /// 取得指定索引的反向依賴。
    /// </summary>
    public bool TryGetReverseDependencyAt(GUID asset, uint index, out GUID dependent)
    {
        return _depTable.TryGetReverseDependencyAt(asset, index, out dependent);
    }

    /// <summary>
    /// 取得所有反向依賴。
    /// </summary>
    public IReadOnlyList<GUID> GetReverseDependencies(GUID asset)
    {
        return _depTable.GetReverseDependencies(asset);
    }

    /* ======================== 圖查詢 ======================== */

    /// <summary>
    /// 檢測依賴圖中是否存在環。
    /// </summary>
    public bool HasCycle()
    {
        return _depTable.HasCycle();
    }

    /// <summary>
    /// 已登記的資產數量。
    /// </summary>
    public int AssetCount => _guidTable.Count;

    /// <summary>
    /// 依賴邊數量。
    /// </summary>
    public int EdgeCount => _depTable.GetEdgeCount();

    /* ======================== 匯入 ======================== */

    /// <summary>
    /// 匯入資產：以虛擬路徑產生穩定的合成 GUID，並登記映射。
    /// 與 C++ NNAssetRegistry::ImportAsset 對應。
    /// </summary>
    public GUID ImportAsset(string virtualPath)
    {
        if (string.IsNullOrEmpty(virtualPath))
            return GUID.Zero;

        // 使用 FNV-1a 雜湊產生合成 GUID（'NNAS' 前綴）
        var hash = Formats.Fnv1a64.Hash(virtualPath);
        var guid = new GUID(0x4E4E4153ul, hash == 0 ? 1 : hash);

        lock (_lock)
        {
            _guidTable.Register(virtualPath, guid);
        }
        return guid;
    }

    /* ======================== 子元件存取 ======================== */

    /// <summary>取得內部 GuidTable（供 AssetDatabase 等使用）。</summary>
    public GuidTable GuidTable => _guidTable;

    /// <summary>取得內部 DependencyTable（供 DependencyTracking 等使用）。</summary>
    public DependencyTable DependencyTable => _depTable;
}
