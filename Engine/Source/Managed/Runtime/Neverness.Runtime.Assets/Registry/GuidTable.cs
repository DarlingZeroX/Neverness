namespace Neverness.Runtime.Assets.Registry;

/// <summary>
/// GUID ↔ VirtualPath 雙向映射表。
///
/// 以 GUID.Low 為主鍵，維護 guid→path 和 path→guid 兩個字典。
/// 與 C++ NNGuidTable 對應。
/// </summary>
public sealed class GuidTable
{
    private readonly Dictionary<ulong, string> _guidToPath = new();
    private readonly Dictionary<string, GUID> _pathToGuid = new(StringComparer.Ordinal);
    private readonly object _lock = new();

    /// <summary>
    /// 註冊路徑與 GUID 的映射。
    /// 若路徑已有舊 GUID 或 GUID 已有舊路徑，自動移除舊映射。
    /// </summary>
    public void Register(string path, GUID guid)
    {
        lock (_lock)
        {
            // 如果路徑已有舊 GUID，移除舊映射
            if (_pathToGuid.TryGetValue(path, out var oldGuid))
            {
                _guidToPath.Remove(oldGuid.Low);
            }

            // 如果 GUID 已有舊路徑，移除舊映射
            if (_guidToPath.TryGetValue(guid.Low, out var oldPath))
            {
                _pathToGuid.Remove(oldPath);
            }

            _guidToPath[guid.Low] = path;
            _pathToGuid[path] = guid;
        }
    }

    /// <summary>
    /// 依 GUID 註銷映射，回傳被移除的路徑（未找到回傳 null）。
    /// </summary>
    public string? UnregisterByGuid(GUID guid)
    {
        lock (_lock)
        {
            if (!_guidToPath.TryGetValue(guid.Low, out var path))
                return null;

            _guidToPath.Remove(guid.Low);
            _pathToGuid.Remove(path);
            return path;
        }
    }

    /// <summary>
    /// 依路徑註銷映射，回傳被移除的 GUID（未找到回傳 null）。
    /// </summary>
    public GUID? UnregisterByPath(string path)
    {
        lock (_lock)
        {
            if (!_pathToGuid.TryGetValue(path, out var guid))
                return null;

            _pathToGuid.Remove(path);
            _guidToPath.Remove(guid.Low);
            return guid;
        }
    }

    /// <summary>
    /// 依 GUID 解析路徑。
    /// </summary>
    /// <returns>是否找到。</returns>
    public bool TryResolvePath(GUID guid, out string? path)
    {
        lock (_lock)
        {
            return _guidToPath.TryGetValue(guid.Low, out path);
        }
    }

    /// <summary>
    /// 依路徑解析 GUID。
    /// </summary>
    /// <returns>是否找到。</returns>
    public bool TryResolveGuid(string path, out GUID guid)
    {
        lock (_lock)
        {
            return _pathToGuid.TryGetValue(path, out guid);
        }
    }

    /// <summary>
    /// 是否包含指定 GUID。
    /// </summary>
    public bool ContainsGuid(GUID guid)
    {
        lock (_lock)
        {
            return _guidToPath.ContainsKey(guid.Low);
        }
    }

    /// <summary>
    /// 已登記的資產數量。
    /// </summary>
    public int Count
    {
        get
        {
            lock (_lock)
            {
                return _guidToPath.Count;
            }
        }
    }

    /// <summary>
    /// 清空所有映射。
    /// </summary>
    public void Clear()
    {
        lock (_lock)
        {
            _guidToPath.Clear();
            _pathToGuid.Clear();
        }
    }
}
