using Neverness.Editor.Framework.Public.Mvvm;

namespace Neverness.Editor.Core.ViewModels;

/// <summary>
/// 场景浏览器 ViewModel——管理实体层级树和选中状态。
/// Phase 3 验证用 ViewModel。
/// </summary>
public class SceneBrowserViewModel : ViewModelBase
{
    // ── 实体节点树 ──
    private List<EntityNodeVM> _rootNodes = new();
    private List<EntityNodeVM> _allNodes = new();
    private List<EntityNodeVM> _visibleNodes = new();

    // ── 选中状态 ──
    private ulong _selectedEntityHandle;
    private readonly HashSet<ulong> _multiSelectedHandles = new();

    // ── 过滤 ──
    private string _searchText = "";

    // ── 展开状态 ──
    private readonly HashSet<ulong> _expandedHandles = new();

    // ── 场景状态 ──
    private bool _hasScene;
    private int _nodeCount;

    // ── 属性 ──

    /// <summary>是否有活跃场景。</summary>
    public bool HasScene
    {
        get => _hasScene;
        set => SetProperty(ref _hasScene, value);
    }

    /// <summary>节点总数。</summary>
    public int NodeCount
    {
        get => _nodeCount;
        set => SetProperty(ref _nodeCount, value);
    }

    /// <summary>可见节点列表（过滤后）。</summary>
    public IReadOnlyList<EntityNodeVM> VisibleNodes => _visibleNodes;

    /// <summary>所有节点。</summary>
    public IReadOnlyList<EntityNodeVM> AllNodes => _allNodes;

    /// <summary>当前选中的实体 Handle。</summary>
    public ulong SelectedEntityHandle
    {
        get => _selectedEntityHandle;
        set => SetProperty(ref _selectedEntityHandle, value);
    }

    /// <summary>搜索过滤文本。</summary>
    public string SearchText
    {
        get => _searchText;
        set
        {
            if (SetProperty(ref _searchText, value))
                RebuildVisibleList();
        }
    }

    // ── 方法（Controller 调用） ──

    /// <summary>重建实体节点树。</summary>
    public void RebuildTree(List<EntityNodeVM> rootNodes, List<EntityNodeVM> allNodes)
    {
        _rootNodes = rootNodes;
        _allNodes = allNodes;
        _nodeCount = allNodes.Count;
        RebuildVisibleList();
        OnPropertyChanged(nameof(AllNodes));
        OnPropertyChanged(nameof(NodeCount));
    }

    /// <summary>设置实体展开状态。</summary>
    public void SetExpanded(ulong handle, bool expanded)
    {
        if (expanded) _expandedHandles.Add(handle);
        else _expandedHandles.Remove(handle);
    }

    /// <summary>检查实体是否展开。</summary>
    public bool IsExpanded(ulong handle) => _expandedHandles.Contains(handle);

    /// <summary>全部展开。</summary>
    public void ExpandAll()
    {
        foreach (var node in _allNodes)
            _expandedHandles.Add(node.Handle);
        RebuildVisibleList();
    }

    /// <summary>全部折叠。</summary>
    public void CollapseAll()
    {
        _expandedHandles.Clear();
        RebuildVisibleList();
    }

    /// <summary>设置选中实体。</summary>
    public void Select(ulong handle, bool addToSelection = false)
    {
        if (!addToSelection)
        {
            _multiSelectedHandles.Clear();
        }
        _selectedEntityHandle = handle;
        _multiSelectedHandles.Add(handle);
        OnPropertyChanged(nameof(SelectedEntityHandle));
    }

    /// <summary>清空选择。</summary>
    public void ClearSelection()
    {
        _selectedEntityHandle = 0;
        _multiSelectedHandles.Clear();
        OnPropertyChanged(nameof(SelectedEntityHandle));
    }

    /// <summary>检查是否选中。</summary>
    public bool IsSelected(ulong handle) => _multiSelectedHandles.Contains(handle);

    /// <summary>重建可见列表（过滤 + 展开状态）。</summary>
    private void RebuildVisibleList()
    {
        _visibleNodes.Clear();

        if (!string.IsNullOrEmpty(_searchText))
        {
            // 搜索模式：显示所有匹配节点
            foreach (var node in _allNodes)
            {
                if (node.Name.Contains(_searchText, StringComparison.OrdinalIgnoreCase))
                    _visibleNodes.Add(node);
            }
        }
        else
        {
            // 正常模式：DFS 遍历，按展开状态裁剪
            foreach (var root in _rootNodes)
            {
                BuildVisibleRecursive(root);
            }
        }

        OnPropertyChanged(nameof(VisibleNodes));
    }

    private void BuildVisibleRecursive(EntityNodeVM node)
    {
        _visibleNodes.Add(node);

        if (node.HasChildren && IsExpanded(node.Handle))
        {
            foreach (var child in node.Children)
            {
                BuildVisibleRecursive(child);
            }
        }
    }
}

/// <summary>实体节点 ViewModel。</summary>
public class EntityNodeVM
{
    public ulong Handle { get; init; }
    public string Name { get; set; } = "";
    public ulong ParentHandle { get; init; }
    public List<EntityNodeVM> Children { get; init; } = new();
    public bool IsRoot => ParentHandle == 0;
    public int Depth { get; init; }
    public bool HasChildren => Children.Count > 0;
}
