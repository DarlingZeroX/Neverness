using Neverness.Editor.Framework.Public;

namespace Neverness.Editor.Framework.Private.Menu;

/// <summary>
/// 菜单树节点——表示菜单树中的一个节点（子菜单或叶菜单项）。
/// </summary>
internal sealed class MenuTreeNode
{
    /// <summary>节点显示名（路径最后一段）。</summary>
    public string Name { get; set; } = "";

    /// <summary>完整路径（如 "File/New Scene"）。</summary>
    public string FullPath { get; set; } = "";

    /// <summary>父节点（顶级菜单为 null）。</summary>
    public MenuTreeNode? Parent { get; set; }

    /// <summary>子节点列表。</summary>
    public List<MenuTreeNode> Children { get; } = [];

    /// <summary>叶节点绑定的菜单项。</summary>
    public EditorMenuItem? Item { get; set; }

    /// <summary>排序权重。</summary>
    public int SortOrder { get; set; }

    /// <summary>是否为动态菜单（运行时生成子项）。</summary>
    public bool IsDynamic { get; set; }

    /// <summary>动态菜单项供应器回调。</summary>
    public Action<DynamicMenuBuilder>? DynamicBuilder { get; set; }

    /// <summary>是否为叶子节点（实际菜单项）。</summary>
    public bool IsLeaf => Item != null || IsDynamic;

    /// <summary>是否为分隔符。</summary>
    public bool IsSeparator => Item?.IsSeparator == true;

    /// <summary>排序后的子节点（按 SortOrder 升序，同级按名称排序）。</summary>
    public IEnumerable<MenuTreeNode> SortedChildren =>
        Children.OrderBy(n => n.SortOrder).ThenBy(n => n.Name, StringComparer.OrdinalIgnoreCase);
}

/// <summary>
/// 菜单树——管理根节点集合。
/// </summary>
internal sealed class MenuTree
{
    /// <summary>顶级菜单节点（File、Edit、Window、Help 等）。</summary>
    public List<MenuTreeNode> Roots { get; } = [];

    /// <summary>排序后的根节点。</summary>
    public IEnumerable<MenuTreeNode> SortedRoots =>
        Roots.OrderBy(n => n.SortOrder).ThenBy(n => n.Name, StringComparer.OrdinalIgnoreCase);

    /// <summary>按完整路径查找节点。</summary>
    public MenuTreeNode? FindByPath(string fullPath) =>
        FindRecursive(Roots, fullPath);

    private static MenuTreeNode? FindRecursive(List<MenuTreeNode> nodes, string path)
    {
        foreach (var node in nodes)
        {
            if (string.Equals(node.FullPath, path, StringComparison.OrdinalIgnoreCase))
                return node;
            var found = FindRecursive(node.Children, path);
            if (found != null) return found;
        }
        return null;
    }

    /// <summary>清空树。</summary>
    public void Clear() => Roots.Clear();
}
