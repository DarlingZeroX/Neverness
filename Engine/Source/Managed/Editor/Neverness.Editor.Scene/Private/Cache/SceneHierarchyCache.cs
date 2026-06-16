using Neverness.Runtime.Scene;
using Neverness.Runtime.Scene.Components;

namespace Neverness.Editor.Scene.Private.Cache;

/// <summary>
/// 场景层级缓存——从 IScene 获取层级数据并缓存。
///
/// 触发机制：
///   每帧调用 TryRefresh() → 比较版本 → 变化才重建缓存。
///
/// 数据流：
///   IScene → 遍历实体 → _allNodes (HierarchyNode[])
///       ↓ RebuildVisibleList
///   _visibleList (List<int>)     ← ImGui 直接遍历（DFS depth 跳跃跳过折叠子树）
/// </summary>
public sealed class SceneHierarchyCache
{
    // ── 版本追踪 ──

    /// <summary>当前缓存的 hierarchyVersion（与 Scene 同步）。</summary>
    private ulong _cachedVersion = ulong.MaxValue;

    /// <summary>当前缓存对应的场景（用于检测场景切换，强制全量刷新）。</summary>
    private IScene? _cachedScene;

    // ── 解析后的数据 ──

    /// <summary>所有节点（DFS 顺序）。</summary>
    private HierarchyNode[] _allNodes = [];

    /// <summary>Entity ID → _allNodes 索引，O(1) 查找。</summary>
    private readonly Dictionary<int, int> _entityToIndex = new(4096);

    /// <summary>根节点在 _allNodes 中的索引列表。</summary>
    private readonly List<int> _rootIndices = new(256);

    // ── UI 状态（独立于快照，快照刷新不清空）─────────────────────

    /// <summary>展开的 Entity ID 集合。</summary>
    private readonly HashSet<int> _expandedSet = new(256);

    /// <summary>选中的 Entity ID 集合。</summary>
    private readonly HashSet<int> _selectedSet = new(64);

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

    /// <summary>选中的 Entity ID 集合。</summary>
    public IReadOnlyCollection<int> SelectedEntities => _selectedSet;

    // ── 版本轮询 + 按需拉取 ──

    /// <summary>
    /// 每帧调用。比较 hierarchyVersion，变化时重建缓存。
    /// </summary>
    /// <returns>true = 缓存已更新；false = 版本未变，跳过</returns>
    public bool TryRefresh(IScene? scene)
    {
        // 0. 场景变化 → 强制全量刷新 + 清除旧 UI 状态
        if (scene != _cachedScene)
        {
            _cachedScene = scene;
            _cachedVersion = ulong.MaxValue;
            _selectedSet.Clear();
            _expandedSet.Clear();
            _searchText = "";
            _visibleDirty = true;
        }

        if (scene == null)
            return false;

        // 1. 版本轮询（通过场景的 HierarchyVersion 属性）
        // TODO: 实现版本轮询
        // 暂时每次都刷新
        var newVersion = (ulong)scene.EntityCount;
        if (newVersion == _cachedVersion)
            return false;

        // 2. 从场景获取层级数据
        RebuildFromScene(scene);

        _cachedVersion = newVersion;
        _visibleDirty = true;
        return true;
    }

    /// <summary>从场景重建层级数据。</summary>
    private void RebuildFromScene(IScene scene)
    {
        // 收集所有实体
        var entities = new List<IEntity>();
        scene.Query<TransformComponent>().ForEach((ref TransformComponent t, IEntity entity) =>
        {
            entities.Add(entity);
        });

        int nodeCount = entities.Count;

        // 复用或重建节点数组
        if (_allNodes.Length != nodeCount)
        {
            var newNodes = new HierarchyNode[nodeCount];
            for (int j = 0; j < nodeCount; j++)
                newNodes[j] = new HierarchyNode();
            _allNodes = newNodes;
        }

        // 预分配 Dictionary 容量
        if (_entityToIndex.Capacity < nodeCount)
            _entityToIndex.EnsureCapacity(nodeCount);

        _entityToIndex.Clear();
        _rootIndices.Clear();

        // 填充节点数据
        for (int i = 0; i < nodeCount; i++)
        {
            var entity = entities[i];
            var node = _allNodes[i];

            node.EntityId = entity.Id;
            node.Name = entity.Name ?? $"Entity_{entity.Id}";
            node.FlatIndex = i;

            // 获取父节点
            var parent = scene.GetParent(entity);
            node.Parent = parent?.Id ?? -1;

            // 获取子节点数量
            var children = scene.GetChildren(entity);
            node.ChildCount = (uint)children.Count;

            // 计算深度
            node.Depth = ComputeDepth(scene, entity);

            // 更新标志位
            node.Flags = 0; // IsActive = true by default
            if (node.Parent < 0)
                node.Flags |= 0x01; // IsActive

            _entityToIndex[entity.Id] = i;

            if (node.Parent < 0)
                _rootIndices.Add(i);
        }
    }

    /// <summary>计算实体的层级深度。</summary>
    private uint ComputeDepth(IScene scene, IEntity entity)
    {
        uint depth = 0;
        var current = entity;
        while (current != null && current.IsValid)
        {
            var parent = scene.GetParent(current);
            if (parent == null || !parent.IsValid) break;
            current = parent;
            depth++;
        }
        return depth;
    }

    // ── 可见列表重建（DFS depth 跳跃）─────────────────────────────

    /// <summary>
    /// 重建可见节点列表。
    ///
    /// 核心优化：利用 DFS 顺序 + depth 字段，折叠时直接跳过整个子树。
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
            if (!hasSearch && node.ChildCount > 0 && !_expandedSet.Contains(node.EntityId))
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
            int cur = nodes[i].Parent;
            while (cur >= 0 && _entityToIndex.TryGetValue(cur, out int idx))
            {
                _expandedSet.Add(cur);
                cur = nodes[idx].Parent;
            }
        }
    }

    // ── 展开 / 折叠 ──

    /// <summary>检查实体是否展开。</summary>
    public bool IsExpanded(int entityId) => _expandedSet.Contains(entityId);

    /// <summary>设置实体展开状态。</summary>
    public void SetExpanded(int entityId, bool expanded)
    {
        if (expanded) _expandedSet.Add(entityId);
        else _expandedSet.Remove(entityId);
        _visibleDirty = true;
    }

    /// <summary>切换实体展开状态。</summary>
    public void ToggleExpanded(int entityId)
    {
        if (!_expandedSet.Remove(entityId))
            _expandedSet.Add(entityId);
        _visibleDirty = true;
    }

    /// <summary>展开所有有子节点的节点。</summary>
    public void ExpandAll()
    {
        var nodes = _allNodes;
        for (int i = 0; i < nodes.Length; i++)
            if (nodes[i].ChildCount > 0)
                _expandedSet.Add(nodes[i].EntityId);
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
    public bool IsSelected(int entityId) => _selectedSet.Contains(entityId);

    /// <summary>选中实体。additive=false 时先清空已有选中。</summary>
    public void Select(int entityId, bool additive = false)
    {
        if (!additive) _selectedSet.Clear();
        _selectedSet.Add(entityId);
    }

    /// <summary>取消选中实体。</summary>
    public void Deselect(int entityId) => _selectedSet.Remove(entityId);

    // ── 搜索 ──

    /// <summary>设置搜索过滤文本。</summary>
    public void SetSearch(string text)
    {
        if (_searchText == text) return;
        _searchText = text ?? "";
        _visibleDirty = true;
    }

    // ── 查找 ──

    /// <summary>按 Entity ID 查找节点。</summary>
    public HierarchyNode? GetNode(int entityId)
        => _entityToIndex.TryGetValue(entityId, out int i) ? _allNodes[i] : null;
}
