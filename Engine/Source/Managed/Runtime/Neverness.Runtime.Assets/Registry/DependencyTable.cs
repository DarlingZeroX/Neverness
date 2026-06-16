namespace Neverness.Runtime.Assets.Registry;

/// <summary>
/// 資產依賴關係圖（前向 + 反向）。
///
/// 前向：asset → dependencies
/// 反向：dependency → dependents（被誰依賴）
///
/// 支持環檢測（DFS 白/灰/黑標記）。
/// 與 C++ NNDependencyTable 對應。
/// </summary>
public sealed class DependencyTable
{
    // 前向依賴：asset.Guid.Low → List<GUID>
    private readonly Dictionary<ulong, List<GUID>> _forward = new();
    // 反向依賴：dep.Guid.Low → List<GUID>
    private readonly Dictionary<ulong, List<GUID>> _reverse = new();
    private readonly object _lock = new();

    /* ======================== 前向依賴 ======================== */

    /// <summary>
    /// 設定資產的完整依賴列表（替換舊值）。
    /// 自動更新反向依賴。
    /// </summary>
    public void SetDependencies(GUID asset, ReadOnlySpan<GUID> deps)
    {
        lock (_lock)
        {
            var key = asset.Low;

            // 先移除舊的反向依賴
            if (_forward.TryGetValue(key, out var oldDeps))
            {
                foreach (var oldDep in oldDeps)
                {
                    if (_reverse.TryGetValue(oldDep.Low, out var revList))
                    {
                        revList.RemoveAll(g => g == asset);
                        if (revList.Count == 0)
                            _reverse.Remove(oldDep.Low);
                    }
                }
            }

            // 設定新的前向依賴
            var newDeps = new List<GUID>(deps.Length);
            foreach (var dep in deps)
                newDeps.Add(dep);
            _forward[key] = newDeps;

            // 更新反向依賴
            foreach (var dep in deps)
            {
                if (!_reverse.TryGetValue(dep.Low, out var revList))
                {
                    revList = new List<GUID>();
                    _reverse[dep.Low] = revList;
                }
                revList.Add(asset);
            }
        }
    }

    /// <summary>
    /// 添加單條依賴（若已存在則忽略）。
    /// </summary>
    public void AddDependency(GUID asset, GUID dependency)
    {
        lock (_lock)
        {
            var key = asset.Low;
            if (!_forward.TryGetValue(key, out var fwdList))
            {
                fwdList = new List<GUID>();
                _forward[key] = fwdList;
            }

            // 檢查是否已存在
            foreach (var dep in fwdList)
            {
                if (dep == dependency)
                    return;
            }

            fwdList.Add(dependency);

            if (!_reverse.TryGetValue(dependency.Low, out var revList))
            {
                revList = new List<GUID>();
                _reverse[dependency.Low] = revList;
            }
            revList.Add(asset);
        }
    }

    /// <summary>
    /// 移除單條依賴。
    /// </summary>
    public void RemoveDependency(GUID asset, GUID dependency)
    {
        lock (_lock)
        {
            var key = asset.Low;
            if (_forward.TryGetValue(key, out var fwdList))
            {
                fwdList.RemoveAll(g => g == dependency);
                if (fwdList.Count == 0)
                    _forward.Remove(key);
            }

            if (_reverse.TryGetValue(dependency.Low, out var revList))
            {
                revList.RemoveAll(g => g == asset);
                if (revList.Count == 0)
                    _reverse.Remove(dependency.Low);
            }
        }
    }

    /// <summary>
    /// 取得資產的依賴數量。
    /// </summary>
    public uint GetDependencyCount(GUID asset)
    {
        lock (_lock)
        {
            return _forward.TryGetValue(asset.Low, out var list) ? (uint)list.Count : 0;
        }
    }

    /// <summary>
    /// 取得指定索引的依賴 GUID。
    /// </summary>
    /// <returns>是否成功。</returns>
    public bool TryGetDependencyAt(GUID asset, uint index, out GUID dependency)
    {
        lock (_lock)
        {
            if (!_forward.TryGetValue(asset.Low, out var list) || index >= list.Count)
            {
                dependency = GUID.Zero;
                return false;
            }
            dependency = list[(int)index];
            return true;
        }
    }

    /// <summary>
    /// 取得資產的所有依賴（只讀副本）。
    /// </summary>
    public IReadOnlyList<GUID> GetDependencies(GUID asset)
    {
        lock (_lock)
        {
            return _forward.TryGetValue(asset.Low, out var list)
                ? list.ToArray()
                : Array.Empty<GUID>();
        }
    }

    /* ======================== 反向依賴 ======================== */

    /// <summary>
    /// 取得反向依賴數量（誰依賴了此資產）。
    /// </summary>
    public uint GetReverseDependencyCount(GUID asset)
    {
        lock (_lock)
        {
            return _reverse.TryGetValue(asset.Low, out var list) ? (uint)list.Count : 0;
        }
    }

    /// <summary>
    /// 取得指定索引的反向依賴 GUID。
    /// </summary>
    public bool TryGetReverseDependencyAt(GUID asset, uint index, out GUID dependent)
    {
        lock (_lock)
        {
            if (!_reverse.TryGetValue(asset.Low, out var list) || index >= list.Count)
            {
                dependent = GUID.Zero;
                return false;
            }
            dependent = list[(int)index];
            return true;
        }
    }

    /// <summary>
    /// 取得所有反向依賴（只讀副本）。
    /// </summary>
    public IReadOnlyList<GUID> GetReverseDependencies(GUID asset)
    {
        lock (_lock)
        {
            return _reverse.TryGetValue(asset.Low, out var list)
                ? list.ToArray()
                : Array.Empty<GUID>();
        }
    }

    /* ======================== 圖查詢 ======================== */

    /// <summary>
    /// 清除資產的所有前向依賴（同時清理反向引用）。
    /// </summary>
    public void ClearDependencies(GUID asset)
    {
        lock (_lock)
        {
            var key = asset.Low;
            if (!_forward.TryGetValue(key, out var fwdList))
                return;

            // 移除反向引用
            foreach (var dep in fwdList)
            {
                if (_reverse.TryGetValue(dep.Low, out var revList))
                {
                    revList.RemoveAll(g => g == asset);
                    if (revList.Count == 0)
                        _reverse.Remove(dep.Low);
                }
            }

            _forward.Remove(key);
        }
    }

    /// <summary>
    /// 檢測依賴圖中是否存在環（DFS 白/灰/黑標記）。
    /// </summary>
    public bool HasCycle()
    {
        lock (_lock)
        {
            // 0=白（未訪問）, 1=灰（在棧中）, 2=黑（已完成）
            var color = new Dictionary<ulong, int>();
            var inStack = new HashSet<ulong>();

            foreach (var key in _forward.Keys)
            {
                if (!color.ContainsKey(key))
                {
                    if (DfsVisit(key, color, inStack))
                        return true;
                }
            }
            return false;
        }
    }

    private bool DfsVisit(ulong node, Dictionary<ulong, int> color, HashSet<ulong> inStack)
    {
        color[node] = 1; // 灰
        inStack.Add(node);

        if (_forward.TryGetValue(node, out var deps))
        {
            foreach (var dep in deps)
            {
                var depKey = dep.Low;
                if (color.TryGetValue(depKey, out var c))
                {
                    if (c == 1) // 灰色節點 → 發現環
                        return true;
                }
                else
                {
                    if (DfsVisit(depKey, color, inStack))
                        return true;
                }
            }
        }

        color[node] = 2; // 黑
        inStack.Remove(node);
        return false;
    }

    /// <summary>
    /// 節點數量（合併前向和反向的所有節點）。
    /// </summary>
    public int GetNodeCount()
    {
        lock (_lock)
        {
            var nodes = new HashSet<ulong>();
            foreach (var key in _forward.Keys)
                nodes.Add(key);
            foreach (var key in _reverse.Keys)
                nodes.Add(key);
            return nodes.Count;
        }
    }

    /// <summary>
    /// 邊數量（前向依賴總數）。
    /// </summary>
    public int GetEdgeCount()
    {
        lock (_lock)
        {
            var total = 0;
            foreach (var deps in _forward.Values)
                total += deps.Count;
            return total;
        }
    }

    /// <summary>
    /// 清空所有依賴關係。
    /// </summary>
    public void Clear()
    {
        lock (_lock)
        {
            _forward.Clear();
            _reverse.Clear();
        }
    }
}
