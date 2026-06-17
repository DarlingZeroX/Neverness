namespace Neverness.Rendering.RenderAssets;

/// <summary>
/// LRU 纹理缓存。
/// 管理 GPU 资源的驻留/驱逐。
/// </summary>
public sealed class TextureCache
{
    private const ulong DefaultMaxMemory = 256UL * 1024 * 1024; // 256 MB

    private readonly Dictionary<ulong, CacheEntry> _entries = new();
    private readonly LinkedList<ulong> _lruList = new(); // 前端 = 最近使用，后端 = 最久未使用
    private ulong _currentMemory;
    private ulong _maxMemory;

    /// <summary>
    /// 创建纹理缓存。
    /// </summary>
    /// <param name="maxMemoryBytes">最大内存限制（字节）</param>
    public TextureCache(ulong maxMemoryBytes = DefaultMaxMemory)
    {
        _maxMemory = maxMemoryBytes;
    }

    /// <summary>当前缓存的内存使用量（字节）。</summary>
    public ulong CurrentMemory => _currentMemory;

    /// <summary>最大内存限制（字节）。</summary>
    public ulong MaxMemory
    {
        get => _maxMemory;
        set => _maxMemory = value;
    }

    /// <summary>缓存条目数量。</summary>
    public int Count => _entries.Count;

    /// <summary>
    /// 插入纹理资源到缓存。
    /// 如果已存在相同 key，先移除旧的。
    /// </summary>
    public void Insert(ulong key, TextureResource resource, ulong memSize)
    {
        // 如果已存在，先移除旧的
        if (_entries.TryGetValue(key, out var existing))
        {
            _currentMemory -= existing.MemorySize;
            _lruList.Remove(existing.LruNode);
            existing.Resource.Dispose();
            _entries.Remove(key);
        }

        // 插入到 LRU 头部（最近使用）
        var lruNode = _lruList.AddFirst(key);

        var entry = new CacheEntry
        {
            Key = key,
            Resource = resource,
            MemorySize = memSize,
            LruNode = lruNode
        };

        _entries[key] = entry;
        _currentMemory += memSize;
    }

    /// <summary>
    /// 获取纹理资源（同时更新 LRU 位置）。
    /// </summary>
    public TextureResource? Get(ulong key)
    {
        if (!_entries.TryGetValue(key, out var entry))
            return null;

        // 移动到 LRU 头部
        _lruList.Remove(entry.LruNode);
        entry.LruNode = _lruList.AddFirst(key);

        return entry.Resource;
    }

    /// <summary>
    /// 检查是否包含指定 key。
    /// </summary>
    public bool Contains(ulong key) => _entries.ContainsKey(key);

    /// <summary>
    /// 移除指定 key 的纹理资源。
    /// </summary>
    public void Remove(ulong key)
    {
        if (!_entries.TryGetValue(key, out var entry))
            return;

        _currentMemory -= entry.MemorySize;
        _lruList.Remove(entry.LruNode);
        entry.Resource.Dispose();
        _entries.Remove(key);
    }

    /// <summary>
    /// 驱逐到目标内存以下，返回驱逐的字节数。
    /// </summary>
    public ulong EvictToTarget(ulong targetBytes)
    {
        ulong evicted = 0;

        while (_currentMemory > targetBytes && _lruList.Count > 0)
        {
            // 从后端取出最久未使用的 key
            var lruKey = _lruList.Last!.Value;
            if (!_entries.TryGetValue(lruKey, out var entry))
            {
                _lruList.RemoveLast();
                continue;
            }

            evicted += entry.MemorySize;
            _currentMemory -= entry.MemorySize;
            _lruList.Remove(entry.LruNode);
            entry.Resource.Dispose();
            _entries.Remove(lruKey);
        }

        return evicted;
    }

    /// <summary>
    /// 清空缓存。
    /// </summary>
    public void Clear()
    {
        foreach (var entry in _entries.Values)
        {
            entry.Resource.Dispose();
        }
        _entries.Clear();
        _lruList.Clear();
        _currentMemory = 0;
    }

    /// <summary>
    /// 缓存条目。
    /// </summary>
    private sealed class CacheEntry
    {
        public required ulong Key { get; init; }
        public required TextureResource Resource { get; init; }
        public required ulong MemorySize { get; init; }
        public required LinkedListNode<ulong> LruNode { get; set; }
    }
}
