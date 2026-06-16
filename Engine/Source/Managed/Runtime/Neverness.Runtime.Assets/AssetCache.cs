namespace Neverness.Runtime.Assets;

/// <summary>
/// 快取條目。
/// </summary>
public sealed class CacheEntry
{
    /// <summary>資產 GUID。</summary>
    public GUID Guid;

    /// <summary>此資產佔用的記憶體（位元組）。</summary>
    public long MemorySize;

    /// <summary>對應的 AssetHandle 原始值。</summary>
    public ulong Handle;

    /// <summary>釘選：不被自動驅逐。</summary>
    public bool Pinned;
}

/// <summary>
/// LRU 資產快取（按最後存取時間驅逐）。
///
/// 當記憶體使用量超過預算時，自動驅逐最少使用的資產。
/// 使用 LinkedList 實現 O(1) 驅逐。
///
/// 與 C++ NNAssetCache 對應。
/// </summary>
public sealed class AssetCache
{
    /// <summary>預設記憶體預算：512MB。</summary>
    public const long DefaultBudget = 512L * 1024 * 1024;

    /* ======================== 內部結構 ======================== */

    /// <summary>快取 slot：持有條目和 LRU 鏈表節點引用。</summary>
    private sealed class CacheSlot
    {
        public CacheEntry Entry;
        public LinkedListNode<GUID> Node;

        public CacheSlot(CacheEntry entry, LinkedListNode<GUID> node)
        {
            Entry = entry;
            Node = node;
        }
    }

    /* ======================== 內部狀態 ======================== */

    private long _budget;
    private long _currentUsage;

    /// <summary>LRU 鏈表：front = 最近使用，back = 最久未使用。</summary>
    private readonly LinkedList<GUID> _lruList = new();

    /// <summary>GUID.Low → CacheSlot。</summary>
    private readonly Dictionary<ulong, CacheSlot> _cache = new();

    private readonly object _lock = new();

    /* ======================== 建構 ======================== */

    public AssetCache(long memoryBudget = DefaultBudget)
    {
        _budget = memoryBudget;
    }

    /* ======================== 公共 API ======================== */

    /// <summary>設定記憶體預算（位元組）。</summary>
    public long MemoryBudget
    {
        get { lock (_lock) return _budget; }
        set { lock (_lock) _budget = value; }
    }

    /// <summary>取得當前記憶體使用量。</summary>
    public long CurrentUsage
    {
        get { lock (_lock) return _currentUsage; }
    }

    /// <summary>取得快取條目數量。</summary>
    public int EntryCount
    {
        get { lock (_lock) return _cache.Count; }
    }

    /// <summary>
    /// 新增或更新快取條目（觸發 LRU 標記）。
    /// 若已存在則更新大小和 Handle，並移到 LRU 前端。
    /// </summary>
    public void Touch(GUID guid, long memorySize, ulong handle)
    {
        lock (_lock)
        {
            if (_cache.TryGetValue(guid.Low, out var slot))
            {
                // 已存在：移至 LRU 鏈表前端
                _lruList.Remove(slot.Node);
                _lruList.AddFirst(slot.Node);
                _currentUsage -= slot.Entry.MemorySize;
                slot.Entry.MemorySize = memorySize;
                slot.Entry.Handle = handle;
                _currentUsage += memorySize;
            }
            else
            {
                // 新增
                var entry = new CacheEntry
                {
                    Guid = guid,
                    MemorySize = memorySize,
                    Handle = handle,
                    Pinned = false
                };
                var node = _lruList.AddFirst(guid);
                _cache[guid.Low] = new CacheSlot(entry, node);
                _currentUsage += memorySize;
            }
        }
    }

    /// <summary>釘選資產（不被驅逐）。</summary>
    public void Pin(GUID guid)
    {
        lock (_lock)
        {
            if (_cache.TryGetValue(guid.Low, out var slot))
                slot.Entry.Pinned = true;
        }
    }

    /// <summary>解除釘選。</summary>
    public void Unpin(GUID guid)
    {
        lock (_lock)
        {
            if (_cache.TryGetValue(guid.Low, out var slot))
                slot.Entry.Pinned = false;
        }
    }

    /// <summary>移除指定資產。</summary>
    public bool Remove(GUID guid)
    {
        lock (_lock)
        {
            if (!_cache.TryGetValue(guid.Low, out var slot))
                return false;

            _currentUsage -= slot.Entry.MemorySize;
            _lruList.Remove(slot.Node);
            _cache.Remove(guid.Low);
            return true;
        }
    }

    /// <summary>
    /// 驅逐最少使用之資產直到釋放 requestedBytes。
    /// 回傳實際釋放的位元組數。
    /// </summary>
    /// <param name="requestedBytes">需要釋放的位元組數。</param>
    /// <param name="onEvict">驅逐回調：當資產被驅逐時呼叫。</param>
    public long Evict(long requestedBytes, Action<GUID, ulong>? onEvict)
    {
        lock (_lock)
        {
            long freed = 0;

            // 從 LRU 鏈表尾端開始驅逐（最久未使用）
            var node = _lruList.Last;
            while (node != null && freed < requestedBytes)
            {
                var prev = node.Previous; // 先保存前一個（因為可能移除當前）

                if (_cache.TryGetValue(node.Value.Low, out var slot) && !slot.Entry.Pinned)
                {
                    freed += slot.Entry.MemorySize;
                    onEvict?.Invoke(slot.Entry.Guid, slot.Entry.Handle);

                    _currentUsage -= slot.Entry.MemorySize;
                    _lruList.Remove(node);
                    _cache.Remove(node.Value.Low);
                }

                node = prev;
            }

            return freed;
        }
    }

    /// <summary>清除所有非釘選條目。</summary>
    public void Clear(Action<GUID, ulong>? onEvict)
    {
        lock (_lock)
        {
            // 先通知非釘選條目
            if (onEvict != null)
            {
                foreach (var slot in _cache.Values)
                {
                    if (!slot.Entry.Pinned)
                        onEvict(slot.Entry.Guid, slot.Entry.Handle);
                }
            }

            // 移除非釘選條目
            var node = _lruList.Last;
            while (node != null)
            {
                var prev = node.Previous;

                if (_cache.TryGetValue(node.Value.Low, out var slot) && !slot.Entry.Pinned)
                {
                    _currentUsage -= slot.Entry.MemorySize;
                    _lruList.Remove(node);
                    _cache.Remove(node.Value.Low);
                }

                node = prev;
            }
        }
    }

    /// <summary>查詢資產是否在快取中。</summary>
    public bool Contains(GUID guid)
    {
        lock (_lock)
        {
            return _cache.ContainsKey(guid.Low);
        }
    }

    /// <summary>
    /// 取得快取條目（若存在）。
    /// 同時觸發 LRU 標記（移到前端）。
    /// </summary>
    public bool TryGetEntry(GUID guid, out CacheEntry? entry)
    {
        lock (_lock)
        {
            if (_cache.TryGetValue(guid.Low, out var slot))
            {
                // 移至 LRU 前端
                _lruList.Remove(slot.Node);
                _lruList.AddFirst(slot.Node);
                entry = slot.Entry;
                return true;
            }
            entry = null;
            return false;
        }
    }
}
