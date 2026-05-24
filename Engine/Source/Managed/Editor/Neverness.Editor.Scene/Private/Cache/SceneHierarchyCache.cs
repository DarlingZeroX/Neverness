using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Neverness.Runtime.Engine;

namespace Neverness.Editor.Scene.Private.Cache;

/// <summary>
/// 场景层级缓存——Native Snapshot 的 Managed 端缓存层。
///
/// 触发机制：
///   每帧调用 TryRefresh() → getHierarchyVersion() 比较版本 → 变化才拉取全量 Snapshot。
///   无变化帧：1 次轻量 P/Invoke（纯整数返回），0 数据拷贝。
///
/// 数据流：
///   _snapshotBuffer (byte[])  ← Native 一次性写入 Header + Nodes + NamePool
///       ↓ ParseSnapshot
///   _allNodes (HierarchyNode[]) ← 解析后，名字已转为 string
///   _entityToIndex (Dictionary)  ← O(1) Entity → Node 查找
///       ↓ RebuildVisibleList
///   _visibleList (List<int>)     ← ImGui 直接遍历（DFS depth 跳跃跳过折叠子树）
/// </summary>
public sealed class SceneHierarchyCache
{
    // ── Snapshot 缓冲区（复用，避免每帧 GC）─────────────────────────

    /// <summary>原始字节缓冲区，Native 直接写入。容量不足时翻倍重新分配。</summary>
    private byte[] _snapshotBuffer = [];

    /// <summary>当前缓存的 hierarchyVersion（与 Native 同步）。</summary>
    private ulong _cachedVersion = ulong.MaxValue;

    // ── 解析后的数据 ──

    /// <summary>所有节点（DFS 顺序，与 Native 1:1 对应）。</summary>
    private HierarchyNode[] _allNodes = [];

    /// <summary>Entity → _allNodes 索引，O(1) 查找。</summary>
    private readonly Dictionary<ulong, int> _entityToIndex = new(4096);

    /// <summary>根节点在 _allNodes 中的索引列表。</summary>
    private readonly List<int> _rootIndices = new(256);

    // ── UI 状态（独立于快照，快照刷新不清空）─────────────────────

    /// <summary>展开的 Entity 集合。</summary>
    private readonly HashSet<ulong> _expandedSet = new(256);

    /// <summary>选中的 Entity 集合。</summary>
    private readonly HashSet<ulong> _selectedSet = new(64);

    /// <summary>搜索过滤字符串（小写）。</summary>
    private string _searchText = "";

    /// <summary>是否需要重建可见列表。</summary>
    private bool _visibleDirty = true;

    /// <summary>可见节点索引列表（ImGui 直接遍历，DFS depth 跳跃跳过折叠子树）。</summary>
    private readonly List<int> _visibleList = new(4096);

    // ── 公共 API ──

    /// <summary>当前缓存中的节点总数。</summary>
    public int NodeCount => _allNodes.Length;

    /// <summary>当前同步的 hierarchyVersion。</summary>
    public ulong HierarchyVersion => _cachedVersion;

    /// <summary>搜索过滤字符串。</summary>
    public string SearchText => _searchText;

    /// <summary>所有节点的 DFS 有序数组（直接引用，调用方不应修改）。</summary>
    public HierarchyNode[] AllNodes => _allNodes;

    /// <summary>可见节点的索引列表（脏时自动重建）。</summary>
    public IReadOnlyList<int> VisibleList
    {
        get
        {
            if (_visibleDirty) RebuildVisibleList();
            return _visibleList;
        }
    }

    /// <summary>选中的 Entity 集合。</summary>
    public IReadOnlyCollection<ulong> SelectedEntities => _selectedSet;

    // ── 版本轮询 + 按需拉取 ──

    /// <summary>
    /// 每帧调用。比较 hierarchyVersion，变化时拉取全量 Snapshot 并重建缓存。
    /// 无变化帧：仅 1 次 getHierarchyVersion()（纯整数 P/Invoke），无数据拷贝。
    /// </summary>
    /// <returns>true = 缓存已更新；false = 版本未变，跳过</returns>
    public unsafe bool TryRefresh(ulong sceneHandle)
    {
        // 1. 版本轮询（最热路径，纯整数返回）
        ulong nativeVersion = EditorSceneNativeBridge.GetHierarchyVersion(sceneHandle);
        if (nativeVersion == _cachedVersion)
            return false; // 无变化，0 数据工作量

        // 2. 查询所需大小
        uint needed = EditorSceneNativeBridge.GetSnapshotSize(sceneHandle);
        if (needed == 0)
            return false;

        // 3. 确保缓冲区容量（翻倍预留，减少下次重分配概率）
        if (_snapshotBuffer.Length < needed)
            _snapshotBuffer = new byte[needed * 2];

        // 4. 一次 P/Invoke 拷贝全部数据
        uint written;
        fixed (byte* buf = _snapshotBuffer)
        {
            written = EditorSceneNativeBridge.GetHierarchySnapshot(sceneHandle, buf, (uint)_snapshotBuffer.Length);
        }
        if (written == 0)
            return false;

        // 5. 解析 Snapshot → _allNodes
        fixed (byte* buf = _snapshotBuffer)
        {
            ParseSnapshot(buf, written);
        }

        _cachedVersion = nativeVersion;
        _visibleDirty = true;
        return true;
    }

    /// <summary>
    /// 增量刷新：拉取脏条目并修补缓存。
    ///
    /// 策略：若脏条目数量占总节点数的比例低于阈值（10%），则仅标记受影响节点为脏；
    /// 否则回退为 <see cref="TryRefresh"/>（全量拉取）。
    ///
    /// 注意：当前脏条目仅含 entity + changeFlags，不含节点完整数据。
    /// 增量拉取后仍需 TryRefresh 进行全量同步。
    /// 主要价值在于 Editor 端可以判断"哪些节点需要局部更新"，避免全量 UI 重建。
    /// </summary>
    /// <returns>true = 缓存需更新（_visibleDirty 已标记）；false = 无变化</returns>
    public unsafe bool TryIncrementalRefresh(ulong sceneHandle)
    {
        // 1. 版本轮询
        ulong nativeVersion = EditorSceneNativeBridge.GetHierarchyVersion(sceneHandle);
        if (nativeVersion == _cachedVersion)
            return false;

        // 2. 若尚无节点数据，直接全量拉取
        if (_allNodes.Length == 0)
            return TryRefresh(sceneHandle);

        // 3. 拉取增量脏条目
        const int MaxDirtyEntries = 4096;
        Span<NNDirtyNodeEntry> entries = stackalloc NNDirtyNodeEntry[MaxDirtyEntries];

        uint bytesWritten;
        fixed (NNDirtyNodeEntry* pEntries = entries)
        {
            bytesWritten = EditorSceneNativeBridge.GetIncrementalSnapshot(
                sceneHandle, pEntries, (uint)(MaxDirtyEntries * sizeof(NNDirtyNodeEntry)));
        }

        int entryCount = (int)(bytesWritten / 16); // sizeof(NNDirtyNodeEntry) = 16
        if (entryCount == 0)
        {
            // 无脏条目但版本已变 → 回退全量
            return TryRefresh(sceneHandle);
        }

        // 4. 阈值判断：脏条目超过 10% → 回退全量拉取
        float dirtyRatio = (float)entryCount / _allNodes.Length;
        if (dirtyRatio > 0.1f)
        {
            return TryRefresh(sceneHandle);
        }

        // 5. 应用脏条目到缓存节点
        for (int i = 0; i < entryCount; i++)
        {
            ref readonly var entry = ref entries[i];
            if (_entityToIndex.TryGetValue(entry.Entity, out int nodeIdx))
            {
                var node = _allNodes[nodeIdx];
                node.Flags |= 0x04; // IsDirty 标记
            }
        }

        _cachedVersion = nativeVersion;
        _visibleDirty = true;
        return true;
    }

    // ── Snapshot 解析 ──

    /// <summary>
    /// 解析二进制 Snapshot 数据为 _allNodes。
    /// 布局：[Header 32B][Nodes * nodeCount * 40B][NamePool * namePoolBytes]
    /// </summary>
    public unsafe void ParseSnapshot(byte* buf, uint size)
    {
        // 校验 Header 最小大小
        if (size < (uint)sizeof(NNSceneSnapshotHeader))
            return;

        var header = *(NNSceneSnapshotHeader*)buf;

        // 校验魔数
        if (header.Magic != 0x56475343) // 'VGSC'
            return;

        int nodeCount = (int)header.NodeCount;

        // 指针定位：Nodes 紧跟 Header，NamePool 紧跟 Nodes
        var nodePtr = (NNSceneNodeSnapshot*)(buf + sizeof(NNSceneSnapshotHeader));
        var namePool = (byte*)(nodePtr + nodeCount);

        // 复用或重建节点数组
        if (_allNodes.Length != nodeCount)
        {
            var newNodes = new HierarchyNode[nodeCount];
            for (int j = 0; j < nodeCount; j++)
                newNodes[j] = new HierarchyNode();
            _allNodes = newNodes;
        }

        // 预分配 Dictionary 容量（避免中途 resize）
        if (_entityToIndex.Capacity < nodeCount)
            _entityToIndex.EnsureCapacity(nodeCount);

        _entityToIndex.Clear();
        _rootIndices.Clear();

        for (int i = 0; i < nodeCount; i++)
        {
            ref readonly var raw = ref nodePtr[i];
            var node = _allNodes[i];

            node.Entity     = raw.Entity;
            node.Parent     = raw.Parent;
            node.Depth      = raw.Depth;
            node.ChildCount = raw.ChildCount;
            node.Flags      = raw.Flags;
            node.FlatIndex  = i;

            // 从 namePool 读取 UTF-8 字符串（nameLen 可为 0）
            node.Name = raw.NameLen > 0
                ? System.Text.Encoding.UTF8.GetString(namePool + raw.NameOffset, (int)raw.NameLen)
                : "";

            _entityToIndex[raw.Entity] = i;

            if (raw.Parent == 0)
                _rootIndices.Add(i);
        }
    }

    // ── 可见列表重建（DFS depth 跳跃）─────────────────────────────

    /// <summary>
    /// 重建可见节点列表。
    ///
    /// 核心优化：利用 Snapshot 的 DFS 顺序 + depth 字段，折叠时直接跳过整个子树。
    /// 不需要递归——DFS 连续输出保证：所有后代在当前节点之后，且 depth 更大。
    /// 遇到 depth &lt;= 当前 depth 的节点时，子树结束。
    ///
    /// 搜索模式：强制展开所有命中节点的祖先路径，确保命中节点可见。
    /// </summary>
    public void RebuildVisibleList()
    {
        if (!_visibleDirty) return;
        _visibleList.Clear();

        bool hasSearch = !string.IsNullOrEmpty(_searchText);

        // 搜索模式：先展开所有命中节点的祖先路径
        if (hasSearch)
            ExpandSearchPaths();

        var nodes = _allNodes;
        int count = nodes.Length;

        // 快速路径：全部展开时直接 add 所有索引
        if (!hasSearch && _expandedSet.Count == 0 && count > 0)
        {
            // 全部折叠：只显示根节点
            for (int k = 0; k < _rootIndices.Count; k++)
            {
                int ri = _rootIndices[k];
                _visibleList.Add(ri);

                // 如果根节点有子节点且被折叠 → 跳过其子树
                if (nodes[ri].ChildCount > 0)
                {
                    uint skipDepth = nodes[ri].Depth;
                    int j = ri + 1;
                    while (j < count && nodes[j].Depth > skipDepth)
                        j++;
                }
            }
            _visibleDirty = false;
            return;
        }

        // 通用路径：逐节点遍历 + DFS depth 跳跃
        int i = 0;
        while (i < count)
        {
            var node = nodes[i];

            // 搜索模式下：过滤不匹配节点
            if (hasSearch && !node.Name.AsSpan().Contains(_searchText.AsSpan(), StringComparison.OrdinalIgnoreCase))
            {
                i++;
                continue;
            }

            // 可见 → 加入列表
            _visibleList.Add(i);

            // 未展开 + 有子节点 + 非搜索模式 → 跳过整个子树
            if (!hasSearch && node.ChildCount > 0 && !_expandedSet.Contains(node.Entity))
            {
                uint skipDepth = node.Depth;
                i++;
                // DFS 顺序保证：depth > skipDepth 的连续节点都是后代
                while (i < count && nodes[i].Depth > skipDepth)
                    i++;
                continue;
            }

            i++;
        }

        _visibleDirty = false;
    }

    /// <summary>展开搜索命中节点的所有祖先（沿 Parent 链向上）。</summary>
    private void ExpandSearchPaths()
    {
        var nodes = _allNodes;
        int count = nodes.Length;
        var searchTextSpan = _searchText.AsSpan();

        for (int i = 0; i < count; i++)
        {
            if (!nodes[i].Name.AsSpan().Contains(searchTextSpan, StringComparison.OrdinalIgnoreCase))
                continue;

            // 向上展开祖先链
            ulong cur = nodes[i].Parent;
            while (cur != 0 && _entityToIndex.TryGetValue(cur, out int idx))
            {
                _expandedSet.Add(cur);
                cur = nodes[idx].Parent;
            }
        }
    }

    // ── 展开 / 折叠 ──

    /// <summary>检查实体是否展开。</summary>
    public bool IsExpanded(ulong entity) => _expandedSet.Contains(entity);

    /// <summary>设置实体展开状态。</summary>
    public void SetExpanded(ulong entity, bool expanded)
    {
        if (expanded) _expandedSet.Add(entity);
        else _expandedSet.Remove(entity);
        _visibleDirty = true;
    }

    /// <summary>切换实体展开状态。</summary>
    public void ToggleExpanded(ulong entity)
    {
        if (!_expandedSet.Remove(entity))
            _expandedSet.Add(entity);
        _visibleDirty = true;
    }

    /// <summary>展开所有有子节点的节点。</summary>
    public void ExpandAll()
    {
        var nodes = _allNodes;
        for (int i = 0; i < nodes.Length; i++)
            if (nodes[i].ChildCount > 0)
                _expandedSet.Add(nodes[i].Entity);
        _visibleDirty = true;
    }

    /// <summary>折叠所有节点。</summary>
    public void CollapseAll()
    {
        _expandedSet.Clear();
        _visibleDirty = true;
    }

    // ── 选中 ──

    /// <summary>检查实体是否被选中。</summary>
    public bool IsSelected(ulong entity) => _selectedSet.Contains(entity);

    /// <summary>选中实体。additive=false 时先清空已有选中。</summary>
    public void Select(ulong entity, bool additive = false)
    {
        if (!additive) _selectedSet.Clear();
        _selectedSet.Add(entity);
    }

    /// <summary>取消选中实体。</summary>
    public void Deselect(ulong entity) => _selectedSet.Remove(entity);

    // ── 搜索 ──

    /// <summary>设置搜索过滤文本。</summary>
    public void SetSearch(string text)
    {
        if (_searchText == text) return;
        _searchText = text ?? "";
        _visibleDirty = true;
    }

    // ── 查找 ──

    /// <summary>按 Entity 句柄查找节点。</summary>
    public HierarchyNode? GetNode(ulong entity)
        => _entityToIndex.TryGetValue(entity, out int i) ? _allNodes[i] : null;
}
