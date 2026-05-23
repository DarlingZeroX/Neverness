using Neverness.Editor.Framework.Public;

namespace Neverness.Editor.Framework.Private.Menu;

/// <summary>
/// 菜单树构建器——将扁平的菜单项列表构建为树结构。
/// </summary>
internal static class MenuTreeBuilder
{
    /// <summary>从注册项列表构建菜单树。</summary>
    public static MenuTree Build(IReadOnlyList<EditorMenuItem> items)
    {
        var tree = new MenuTree();

        foreach (var item in items)
        {
            if (item.IsSeparator)
            {
                // 分隔符绑定到其父节点
                var parentPath = GetParentPath(item.Path);
                var parent = EnsureNode(tree, parentPath);
                var sepNode = new MenuTreeNode
                {
                    Name = "",
                    FullPath = item.Path,
                    Parent = parent,
                    Item = item,
                    SortOrder = item.SortOrder,
                };
                parent.Children.Add(sepNode);
                continue;
            }

            var segments = item.Path.Split('/', StringSplitOptions.RemoveEmptyEntries);
            if (segments.Length == 0) continue;

            MenuTreeNode? current = null;
            var currentPath = "";

            for (int i = 0; i < segments.Length; i++)
            {
                currentPath = i == 0 ? segments[0] : currentPath + "/" + segments[i];

                if (i == 0)
                {
                    current = tree.Roots.FirstOrDefault(
                        n => string.Equals(n.Name, segments[0], StringComparison.OrdinalIgnoreCase));
                    if (current == null)
                    {
                        current = new MenuTreeNode
                        {
                            Name = segments[0],
                            FullPath = segments[0],
                            SortOrder = item.SortOrder,
                        };
                        tree.Roots.Add(current);
                    }
                }
                else
                {
                    var child = current!.Children.FirstOrDefault(
                        n => string.Equals(n.Name, segments[i], StringComparison.OrdinalIgnoreCase));
                    if (child == null)
                    {
                        child = new MenuTreeNode
                        {
                            Name = segments[i],
                            FullPath = currentPath,
                            Parent = current,
                            SortOrder = item.SortOrder,
                        };
                        current.Children.Add(child);
                    }
                    current = child;
                }
            }

            // 叶节点绑定 Item
            if (current != null)
            {
                current.Item = item;
                current.SortOrder = item.SortOrder;
            }
        }

        return tree;
    }

    /// <summary>将动态菜单项注入树中。</summary>
    public static void AttachDynamic(MenuTree tree, string menuPath, Action<DynamicMenuBuilder> builder)
    {
        var node = EnsureNode(tree, menuPath);
        node.IsDynamic = true;
        node.DynamicBuilder = builder;
    }

    /// <summary>确保路径对应的节点存在（不存在则创建）。</summary>
    public static MenuTreeNode EnsureNode(MenuTree tree, string path)
    {
        var existing = tree.FindByPath(path);
        if (existing != null) return existing;

        var segments = path.Split('/', StringSplitOptions.RemoveEmptyEntries);
        MenuTreeNode? current = null;
        var currentPath = "";

        for (int i = 0; i < segments.Length; i++)
        {
            currentPath = i == 0 ? segments[0] : currentPath + "/" + segments[i];

            if (i == 0)
            {
                current = tree.Roots.FirstOrDefault(
                    n => string.Equals(n.Name, segments[0], StringComparison.OrdinalIgnoreCase));
                if (current == null)
                {
                    current = new MenuTreeNode { Name = segments[0], FullPath = segments[0] };
                    tree.Roots.Add(current);
                }
            }
            else
            {
                var child = current!.Children.FirstOrDefault(
                    n => string.Equals(n.Name, segments[i], StringComparison.OrdinalIgnoreCase));
                if (child == null)
                {
                    child = new MenuTreeNode
                    {
                        Name = segments[i],
                        FullPath = currentPath,
                        Parent = current,
                    };
                    current.Children.Add(child);
                }
                current = child;
            }
        }

        return current!;
    }

    /// <summary>获取路径的父路径。</summary>
    private static string GetParentPath(string path)
    {
        var idx = path.LastIndexOf('/');
        return idx > 0 ? path[..idx] : "";
    }
}
