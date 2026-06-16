using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Layout;
using Avalonia.Media;
using Avalonia.Reactive;
using Neverness.Editor.AvaloniaFrontend.ContextMenus;
using Neverness.Editor.Core.Controllers;
using Neverness.Editor.Core.ViewModels;
using Neverness.Editor.Framework.Private.Menu;
using Neverness.Editor.Scene.Public;

namespace Neverness.Editor.AvaloniaFrontend.Views;

/// <summary>
/// 场景浏览器 Avalonia View——显示实体层级树。
///
/// 实现细节：
/// - TreeView 显示实体层级（虚拟化）
/// - 搜索栏过滤
/// - 工具栏：展开/折叠全部
/// - 右键菜单（重命名/复制/删除）
/// - 拖拽重设父节点
/// - 绑定到 SceneBrowserViewModel + SceneBrowserController
/// </summary>
public class SceneBrowserAvaloniaView : AvaloniaViewBase
{
    private SceneBrowserViewModel? _viewModel;
    private SceneBrowserController? _controller;
    private TreeView? _treeView;
    private TextBox? _searchBox;
    private AvaloniaContextMenuRenderer? _contextMenuRenderer;

    public SceneBrowserAvaloniaView() : base("SceneBrowser")
    {
    }

    public override Type ViewModelType => typeof(SceneBrowserViewModel);

    public override void Bind(object viewModel)
    {
        _viewModel = (SceneBrowserViewModel)viewModel;

        // 创建 Avalonia 控件树
        var panel = new DockPanel();

        // ── 工具栏 ──
        var toolbar = CreateToolbar();
        Avalonia.Controls.DockPanel.SetDock(toolbar, Avalonia.Controls.Dock.Top);
        panel.Children.Add(toolbar);

        // ── 搜索栏 ──
        var searchBar = CreateSearchBar();
        Avalonia.Controls.DockPanel.SetDock(searchBar, Avalonia.Controls.Dock.Top);
        panel.Children.Add(searchBar);

        // ── 实体树 ──
        _treeView = new TreeView
        {
            Background = new SolidColorBrush(Color.Parse("#FF252526")),
            BorderThickness = new Avalonia.Thickness(0),
        };
        _treeView.SelectionChanged += OnTreeViewSelectionChanged;
        _treeView.ContextRequested += OnTreeViewContextRequested;

        // 初始化上下文菜单渲染器
        _contextMenuRenderer = new AvaloniaContextMenuRenderer();

        panel.Children.Add(_treeView);

        // 设置为 UserControl 的内容
        Content = panel;

        // 订阅 ViewModel 变更
        _viewModel.PropertyChanged += OnPropertyChanged;
    }

    public override void Unbind()
    {
        if (_viewModel != null)
            _viewModel.PropertyChanged -= OnPropertyChanged;
        _viewModel = null;
        _controller = null;
        _treeView = null;
        _searchBox = null;
        _contextMenuRenderer = null;
    }

    /// <summary>设置 Controller（由 AvaloniaViewFactory 调用）。</summary>
    public void SetController(SceneBrowserController controller)
    {
        _controller = controller;
    }

    /// <summary>创建工具栏。</summary>
    private StackPanel CreateToolbar()
    {
        var toolbar = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 4,
            Margin = new Avalonia.Thickness(4),
        };

        // 展开全部按钮
        var expandButton = new Button
        {
            Content = "Expand All",
            MinWidth = 80,
            MinHeight = 24,
            Padding = new Avalonia.Thickness(4, 2),
        };
        expandButton.Click += (_, _) => _viewModel?.ExpandAll();
        toolbar.Children.Add(expandButton);

        // 折叠全部按钮
        var collapseButton = new Button
        {
            Content = "Collapse All",
            MinWidth = 80,
            MinHeight = 24,
            Padding = new Avalonia.Thickness(4, 2),
        };
        collapseButton.Click += (_, _) => _viewModel?.CollapseAll();
        toolbar.Children.Add(collapseButton);

        // 节点计数
        var countLabel = new TextBlock
        {
            Text = $"Nodes: {_viewModel?.NodeCount ?? 0}",
            VerticalAlignment = VerticalAlignment.Center,
            Margin = new Avalonia.Thickness(8, 0),
            Foreground = new SolidColorBrush(Color.Parse("#FF999999")),
        };
        toolbar.Children.Add(countLabel);

        return toolbar;
    }

    /// <summary>创建搜索栏。</summary>
    private StackPanel CreateSearchBar()
    {
        var searchBar = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 4,
            Margin = new Avalonia.Thickness(4, 0, 4, 4),
        };

        _searchBox = new TextBox
        {
            PlaceholderText = "Search entities...",
            MinWidth = 200,
            MinHeight = 24,
        };
        _searchBox.TextChanged += (_, e) =>
        {
            if (_viewModel != null)
                _viewModel.SearchText = _searchBox.Text ?? "";
        };
        searchBar.Children.Add(_searchBox);

        return searchBar;
    }

    /// <summary>TreeView 选中变更。</summary>
    private void OnTreeViewSelectionChanged(object? sender, SelectionChangedEventArgs e)
    {
        if (_viewModel == null || _controller == null) return;

        if (e.AddedItems.Count > 0 && e.AddedItems[0] is TreeViewItem treeItem && treeItem.Tag is int handle)
        {
            _viewModel.Select(handle);
        }
    }

    /// <summary>ViewModel 属性变更。</summary>
    private void OnPropertyChanged(string propertyName)
    {
        if (propertyName == nameof(SceneBrowserViewModel.VisibleNodes))
        {
            RebuildTreeView();
        }
    }

    /// <summary>重建 TreeView。</summary>
    private void RebuildTreeView()
    {
        if (_treeView == null || _viewModel == null) return;

        _treeView.Items.Clear();

        foreach (var node in _viewModel.VisibleNodes)
        {
            var treeItem = CreateTreeViewItem(node);
            _treeView.Items.Add(treeItem);
        }
    }

    /// <summary>创建 TreeViewItem。</summary>
    private TreeViewItem CreateTreeViewItem(EntityNodeVM node)
    {
        var treeItem = new TreeViewItem
        {
            Header = CreateNodeHeader(node),
            Tag = node.Id,
            IsExpanded = _viewModel?.IsExpanded(node.Id) ?? false,
        };

        // 展开/折叠事件
        treeItem.PropertyChanged += (_, e) =>
        {
            if (e.Property.Name == nameof(TreeViewItem.IsExpanded))
            {
                _viewModel?.SetExpanded(node.Id, treeItem.IsExpanded);
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
    private StackPanel CreateNodeHeader(EntityNodeVM node)
    {
        var panel = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 4,
        };

        // 实体图标（TODO: 使用 EditorIcons）
        var icon = new TextBlock
        {
            Text = "🎬", // 临时图标
            FontSize = 12,
        };
        panel.Children.Add(icon);

        // 实体名称
        var name = new TextBlock
        {
            Text = node.Name.ToString(),
            FontSize = 12,
            VerticalAlignment = VerticalAlignment.Center,
        };
        panel.Children.Add(name);

        return panel;
    }

    // ── 右键菜单 ──

    /// <summary>TreeView 上下文请求事件——区分实体节点和空白区域。</summary>
    private void OnTreeViewContextRequested(object? sender, ContextRequestedEventArgs e)
    {
        if (_treeView == null || _contextMenuRenderer == null) return;

        // 设置运行时上下文
        var ctx = ContextMenuManager.Instance;
        var world = SceneModule.GetActiveWorld();
        ctx.SetContext(SceneBrowserContextMenu.KeyActiveWorld, world);

        // 判断右键位置：实体节点 or 空白区域
        if (_treeView.SelectedItem is TreeViewItem treeItem && treeItem.Tag is int handle && handle > 0)
        {
            // 右键实体节点 → 实体菜单
            ctx.SetContext(SceneBrowserContextMenu.KeyEntityHandle, handle);
            _contextMenuRenderer.BuildAndShow(
                SceneBrowserContextMenu.EntityId,
                ctx,
                treeItem);
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
}
