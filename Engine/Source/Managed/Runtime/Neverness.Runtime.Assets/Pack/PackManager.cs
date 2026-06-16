namespace Neverness.Runtime.Assets.Pack;

/// <summary>
/// .nnpack 包管理器。
///
/// 管理已掛載的 .nnpack 包，提供包內資產的查詢和讀取。
/// 與 C++ NNPackManager 對應。
/// </summary>
public sealed class PackManager
{
    /* ======================== 內部結構 ======================== */

    /// <summary>資產在包內的位置。</summary>
    private struct AssetLocation
    {
        public int MountIndex;
        public int EntryIndex;
    }

    /* ======================== 內部狀態 ======================== */

    private readonly List<PackMount> _mounts = new();
    private readonly Dictionary<ulong, AssetLocation> _globalIndex = new();
    private readonly object _lock = new();

    /// <summary>全域單例。</summary>
    public static PackManager Instance { get; } = new();

    private PackManager() { }

    /* ======================== 掛載/卸載 ======================== */

    /// <summary>
    /// 掛載 .nnpack 包。
    /// </summary>
    /// <param name="packPath">包檔案路徑。</param>
    /// <returns>是否成功。</returns>
    public bool MountPackage(string packPath)
    {
        lock (_lock)
        {
            // 檢查是否已掛載
            foreach (var m in _mounts)
            {
                if (m.Path == packPath)
                    return true;
            }

            PackMount mount;
            try
            {
                mount = new PackMount(packPath);
            }
            catch
            {
                return false;
            }

            var mountIdx = _mounts.Count;
            _mounts.Add(mount);

            // 建立全域索引
            for (int i = 0; i < mount.AssetTable.Length; i++)
            {
                var entry = mount.AssetTable[i];
                _globalIndex[entry.GuidLow] = new AssetLocation
                {
                    MountIndex = mountIdx,
                    EntryIndex = i,
                };
            }

            return true;
        }
    }

    /// <summary>
    /// 卸載 .nnpack 包。
    /// </summary>
    public void UnmountPackage(string packPath)
    {
        lock (_lock)
        {
            for (int i = 0; i < _mounts.Count; i++)
            {
                if (_mounts[i].Path == packPath)
                {
                    // 從全域索引中移除
                    var mount = _mounts[i];
                    foreach (var guidLow in mount.GuidToIndex.Keys)
                        _globalIndex.Remove(guidLow);

                    mount.Dispose();
                    _mounts.RemoveAt(i);
                    return;
                }
            }
        }
    }

    /// <summary>
    /// 卸載所有包。
    /// </summary>
    public void UnmountAll()
    {
        lock (_lock)
        {
            foreach (var mount in _mounts)
                mount.Dispose();
            _mounts.Clear();
            _globalIndex.Clear();
        }
    }

    /* ======================== 查詢 ======================== */

    /// <summary>
    /// 資產是否在任何已掛載的包中。
    /// </summary>
    public bool IsAssetInPackage(GUID guid)
    {
        lock (_lock)
        {
            return _globalIndex.ContainsKey(guid.Low);
        }
    }

    /// <summary>
    /// 取得資產所在的包路徑（未找到回傳 null）。
    /// </summary>
    public string? GetPackageForAsset(GUID guid)
    {
        lock (_lock)
        {
            if (!_globalIndex.TryGetValue(guid.Low, out var loc))
                return null;
            return _mounts[loc.MountIndex].Path;
        }
    }

    /// <summary>
    /// 取得包內資產的型別 ID（未找到回傳 0）。
    /// </summary>
    public ulong GetAssetTypeIdInPackage(GUID guid)
    {
        lock (_lock)
        {
            if (!_globalIndex.TryGetValue(guid.Low, out var loc))
                return 0;
            return _mounts[loc.MountIndex].AssetTable[loc.EntryIndex].TypeId;
        }
    }

    /// <summary>
    /// 從包內讀取資產資料（零拷貝）。
    ///
    /// 注意：返回的 Span 指向 MemoryMappedFile 的記憶體，
    /// 必須在 PackMount 被 Dispose 前使用。若需跨異步邊界傳遞，應使用 <see cref="ReadAssetFromPackageCopy"/>。
    /// </summary>
    /// <param name="guid">資產 GUID。</param>
    /// <returns>資產資料的唯讀 Span；未找到或範圍無效則返回空。</returns>
    public ReadOnlySpan<byte> ReadAssetFromPackage(GUID guid)
    {
        lock (_lock)
        {
            if (!_globalIndex.TryGetValue(guid.Low, out var loc))
                return ReadOnlySpan<byte>.Empty;
            return _mounts[loc.MountIndex].ReadAssetData(loc.EntryIndex);
        }
    }

    /// <summary>
    /// 從包內讀取資產資料（安全版本，複製為 byte[]）。
    /// 適用於需要跨異步邊界傳遞資料的場景。
    /// </summary>
    public byte[]? ReadAssetFromPackageCopy(GUID guid)
    {
        lock (_lock)
        {
            if (!_globalIndex.TryGetValue(guid.Low, out var loc))
                return null;
            return _mounts[loc.MountIndex].ReadAssetDataCopy(loc.EntryIndex);
        }
    }

    /// <summary>
    /// 取得已掛載的包路徑列表。
    /// </summary>
    public IReadOnlyList<string> GetMountedPackages()
    {
        lock (_lock)
        {
            var result = new string[_mounts.Count];
            for (int i = 0; i < _mounts.Count; i++)
                result[i] = _mounts[i].Path;
            return result;
        }
    }

    /// <summary>
    /// 已掛載的包數量。
    /// </summary>
    public int MountedPackageCount
    {
        get { lock (_lock) return _mounts.Count; }
    }

    /* ======================== IDisposable ======================== */

    /// <summary>
    /// 釋放所有掛載的包（由 AssetManager 在 Shutdown 時調用）。
    /// </summary>
    public void Dispose()
    {
        UnmountAll();
    }
}
