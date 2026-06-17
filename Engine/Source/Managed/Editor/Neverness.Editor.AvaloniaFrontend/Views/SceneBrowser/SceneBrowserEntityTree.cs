using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Layout;
using Avalonia.Media;
using Neverness.Editor.AvaloniaFrontend.ContextMenus;
using Neverness.Editor.Core.ViewModels;
using Neverness.Editor.Framework.Private.Menu;
using Neverness.Editor.Scene.Public;
using static Neverness.Editor.AvaloniaFrontend.Views.SceneBrowser.SceneBrowserColors;

namespace Neverness.Editor.AvaloniaFrontend.Views.SceneBrowser;

/// <summary>
/// 场景浏览器实体树——TreeView 渲染和交互。
/// </summary>
internal sealed class SceneBrowserEntityTree
{
    private readonly SceneBrowserViewModel _viewModel;
    private readonly AvaloniaContextMenuRenderer _contextMenuRenderer;

    private TreeView? _treeView;

    /// <summary>TreeView 控件引用。</summary>
    internal TreeView? TreeView => _treeView;

    internal SceneBrowserEntityTree(SceneBrowserViewModel viewModel)
    {
        _viewModel = viewModel;
        _contextMenuRenderer = new AvaloniaContextMenuRenderer();
    }

    /// <summary>创建实体树控件。</summary>
    internal Control Create(Action<int>? onSelectionChanged)
    {
        _treeView = new TreeView
        {
            Background = BgMain,
            BorderThickness = new Thickness(0),
            Padding = new Thickness(4),
        };

        _treeView.SelectionChanged += (_, e) =>
        {
            if (e.AddedItems.Count > 0 && e.AddedItems[0] is TreeViewItem treeItem && treeItem.Tag is int handle)
            {
                onSelectionChanged?.Invoke(handle);
            }
        };

        _treeView.ContextRequested += OnTreeViewContextRequested;

        return _treeView;
    }

    /// <summary>重建 TreeView。</summary>
    internal void Rebuild()
    {
        if (_treeView == null) return;

        _treeView.Items.Clear();

        foreach (var node in _viewModel.VisibleNodes)
        {
            var treeItem = CreateTreeViewItem(node);
            _treeView.Items.Add(treeItem);
        }
    }

    /* ======================== 私有方法 ======================== */

    /// <summary>创建 TreeViewItem。</summary>
    private TreeViewItem CreateTreeViewItem(EntityNodeVM node)
    {
        var treeItem = new TreeViewItem
        {
            Header = CreateNodeHeader(node),
            Tag = node.Id,
            IsExpanded = _viewModel.IsExpanded(node.Id),
            Padding = new Thickness(2, 4),
        };

        // 展开/折叠事件
        treeItem.PropertyChanged += (_, e) =>
        {
            if (e.Property.Name == nameof(TreeViewItem.IsExpanded))
            {
                _viewModel.SetExpanded(node.Id, treeItem.IsExpanded);
            }
        };

        // 添加子节点
        foreach (var child in node.Children)
        {
            treeItem.Items.Add(CreateTreeViewItem(child));
        }

        return treeItem;
    }

    /// <summary>创建节点头部控件。</summary>
    private static Control CreateNodeHeader(EntityNodeVM node)
    {
        var panel = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 6,
            Margin = new Thickness(4, 2),
        };

        // 展开/折叠图标
        var expandIcon = new TextBlock
        {
            Text = node.HasChildren ? "▾" : "  ",
            FontSize = 10,
            Foreground = TextSecondary,
            VerticalAlignment = VerticalAlignment.Center,
            Width = 12,
            TextAlignment = TextAlignment.Center,
        };
        panel.Children.Add(expandIcon);

        // 实体图标
        var icon = new TextBlock
        {
            Text = "🎮",
            FontSize = 14,
            VerticalAlignment = VerticalAlignment.Center,
        };
        panel.Children.Add(icon);

        // 实体名称
        var name = new TextBlock
        {
            Text = node.Name.ToString(),
            FontSize = 12,
            Foreground = TextPrimary,
            VerticalAlignment = VerticalAlignment.Center,
        };
        panel.Children.Add(name);

        return panel;
    }

    /// <summary>TreeView 上下文请求事件——区分实体节点和空白区域。</summary>
    private void OnTreeViewContextRequested(object? sender, ContextRequestedEventArgs e)
    {
        if (_treeView == null) return;

        // 设置运行时上下文
        var ctx = ContextMenuManager.Instance;
        var world = SceneModule.GetActiveWorld();
        ctx.SetContext(SceneBrowserContextMenu.KeyActiveWorld, world);

        // 通过 e.Source 向上查找是否命中了 TreeViewItem（而非依赖 SelectedItem）
        var hitItem = FindParentTreeViewItem(e.Source as Control);

        if (hitItem != null && hitItem.Tag is int handle && handle > 0)
        {
            // 右键实体节点 → 实体菜单
            ctx.SetContext(SceneBrowserContextMenu.KeyEntityHandle, handle);
            _contextMenuRenderer.BuildAndShow(
                SceneBrowserContextMenu.EntityId,
                ctx,
                hitItem);
        }
        else
        {
            // 右键空白区域 → 背景菜单
            ctx.SetContext(SceneBrowserContextMenu.KeyEntityHandle, 0);
            _contextMenuRenderer.BuildAndShow(
                SceneBrowserContextMenu.BackgroundId,
                ctx,
                _treeView);
        }

        e.Handled = true;
    }

    /// <summary>从指定控件向上查找最近的 TreeViewItem。</summary>
    private static TreeViewItem? FindParentTreeViewItem(Control? control)
    {
        while (control != null)
        {
            if (control is TreeViewItem tvi)
                return tvi;
            control = control.Parent as Control;
        }
        return null;
    }
}
