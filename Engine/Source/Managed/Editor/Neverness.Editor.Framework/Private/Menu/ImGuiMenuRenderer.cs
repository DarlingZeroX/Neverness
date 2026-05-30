using System.Numerics;
using Hexa.NET.ImGui;
using Neverness.Editor.Framework.Public;

namespace Neverness.Editor.Framework.Private.Menu;

/// <summary>
/// ImGui 菜单渲染器——递归遍历 MenuTree 绘制主菜单栏。
/// </summary>
public static class ImGuiMenuRenderer
{
    /// <summary>渲染主菜单栏（完整生命周期：Begin → 菜单项 → End）。</summary>
    public static void RenderMainMenuBar(MenuTree tree)
    {
        if (!ImGui.BeginMainMenuBar()) return;

        RenderMenuItems(tree);

        ImGui.EndMainMenuBar();
    }

    /// <summary>
    /// 开始主菜单栏渲染（返回 false 时不应回调后续操作）。
    /// 与 <see cref="EndMainMenuBar"/> 配对使用，用于在菜单栏内插入自定义控件。
    /// </summary>
    public static bool BeginMainMenuBar() => ImGui.BeginMainMenuBar();

    /// <summary>结束主菜单栏渲染。</summary>
    public static void EndMainMenuBar() => ImGui.EndMainMenuBar();

    /// <summary>仅渲染菜单项（不含 Begin/End 包裹）。</summary>
    public static void RenderMenuItems(MenuTree tree)
    {
        foreach (var root in tree.SortedRoots)
        {
            RenderMenuNode(root);
        }
    }

    /// <summary>渲染弹出菜单（用于 Context Menu）。</summary>
    public static void RenderPopupMenu(MenuTreeNode node)
    {
        foreach (var child in node.SortedChildren)
        {
            RenderMenuNodeOrItem(child);
        }
    }

    /// <summary>递归渲染单个菜单节点。</summary>
    private static void RenderMenuNode(MenuTreeNode node)
    {
        // 动态菜单：先调用供应器生成临时子项
        if (node.IsDynamic && node.DynamicBuilder != null)
        {
            if (ImGui.BeginMenu(FormatNodeLabel(node)))
            {
                var builder = new DynamicMenuBuilder();
                node.DynamicBuilder(builder);
                foreach (var item in builder.Build())
                {
                    RenderMenuItemDirect(item);
                }
                ImGui.EndMenu();
            }
            return;
        }

        // 纯子菜单（有子节点但自身不是叶节点）
        if (!node.IsLeaf)
        {
            if (ImGui.BeginMenu(FormatNodeLabel(node)))
            {
                RenderChildren(node);
                ImGui.EndMenu();
            }
            return;
        }

        // 叶节点
        if (node.Item != null)
        {
            RenderMenuItemDirect(node.Item.Value);
        }
    }

    /// <summary>渲染子节点列表。</summary>
    private static void RenderChildren(MenuTreeNode node)
    {
        string? lastGroup = null;

        foreach (var child in node.SortedChildren)
        {
            // 分组分隔
            var group = child.Item?.Group ?? "";
            if (!string.IsNullOrEmpty(group) && lastGroup != null && group != lastGroup)
            {
                ImGui.Separator();
            }
            lastGroup = group;

            // 分隔符
            if (child.IsSeparator)
            {
                ImGui.Separator();
                continue;
            }

            RenderMenuNodeOrItem(child);
        }
    }

    /// <summary>渲染节点——如果是子菜单则 BeginMenu，否则 MenuItem。</summary>
    private static void RenderMenuNodeOrItem(MenuTreeNode node)
    {
        if (node.IsDynamic)
        {
            RenderMenuNode(node);
            return;
        }

        if (!node.IsLeaf)
        {
            RenderMenuNode(node);
            return;
        }

        if (node.Item != null)
        {
            RenderMenuItemDirect(node.Item.Value);
        }
    }

    /// <summary>渲染单个 MenuItem。</summary>
    private static void RenderMenuItemDirect(EditorMenuItem item)
    {
        if (item.IsSeparator)
        {
            ImGui.Separator();
            return;
        }

        var label = FormatItemLabel(item);
        var shortcut = string.IsNullOrEmpty(item.Shortcut) ? null : item.Shortcut;
        var enabled = true;
        var command = item.Command;

        // CanExecute 检查
        if (command?.CanExecute != null)
        {
            enabled = command.CanExecute();
        }

        // IsChecked（null 视为非勾选项）
        var selected = command?.IsChecked?.Invoke() ?? false;
        var hasCheck = command?.IsChecked != null;

        if (hasCheck)
        {
            if (ImGui.MenuItem(label, shortcut, selected, enabled))
            {
                command?.Execute(default);
            }
        }
        else
        {
            if (ImGui.MenuItem(label, shortcut, false, enabled))
            {
                command?.Execute(default);
            }
        }

        // Tooltip
        if (!string.IsNullOrEmpty(item.Tooltip) && ImGui.IsItemHovered(ImGuiHoveredFlags.DelayShort))
        {
            ImGui.SetTooltip(item.Tooltip);
        }
    }

    /// <summary>格式化节点标签：图标 + 名称。</summary>
    private static string FormatNodeLabel(MenuTreeNode node)
    {
        var item = node.Item;
        if (item == null) return node.Name;
        return FormatItemLabel(item.Value);
    }

    /// <summary>格式化菜单项标签：图标 + 名称。</summary>
    private static string FormatItemLabel(EditorMenuItem item)
    {
        var displayName = GetLastSegment(item.Path);
        if (string.IsNullOrEmpty(item.Icon)) return displayName;
        return item.Icon + " " + displayName;
    }

    /// <summary>提取路径最后一段作为显示名。</summary>
    private static string GetLastSegment(string path)
    {
        var idx = path.LastIndexOf('/');
        return idx >= 0 ? path[(idx + 1)..] : path;
    }
}
