using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Media;
using Neverness.Editor.Core.Controllers;
using Neverness.Editor.Core.ViewModels;

namespace Neverness.Editor.AvaloniaFrontend.Views;

/// <summary>
/// 内容浏览器 Avalonia View——显示资产目录树和文件网格。
///
/// 实现细节：
/// - 左侧 TreeView 显示目录树
/// - 右侧 ListBox 显示文件网格/列表
/// - 面包屑导航
/// - 搜索过滤
/// - 绑定到 ContentBrowserViewModel + ContentBrowserController
/// </summary>
public class ContentBrowserAvaloniaView : AvaloniaViewBase
{
    private ContentBrowserViewModel? _viewModel;
    private ContentBrowserController? _controller;
    private TreeView? _directoryTree;
    private ListBox? _fileGrid;
    private TextBlock? _breadcrumb;

    public ContentBrowserAvaloniaView() : base("ContentBrowser")
    {
    }

    public override Type ViewModelType => typeof(ContentBrowserViewModel);

    public override void Bind(object viewModel)
    {
        _viewModel = (ContentBrowserViewModel)viewModel;

        // 创建 Avalonia 控件树
        var panel = new DockPanel();

        // ── 工具栏 ──
        var toolbar = CreateToolbar();
        Avalonia.Controls.DockPanel.SetDock(toolbar, Avalonia.Controls.Dock.Top);
        panel.Children.Add(toolbar);

        // ── 面包屑导航 ──
        var breadcrumbBar = CreateBreadcrumbBar();
        Avalonia.Controls.DockPanel.SetDock(breadcrumbBar, Avalonia.Controls.Dock.Top);
        panel.Children.Add(breadcrumbBar);

        // ── 主内容区：左侧目录树 + 右侧文件网格 ──
        var mainContent = new Grid();
        mainContent.ColumnDefinitions.Add(new ColumnDefinition(200, GridUnitType.Pixel));
        mainContent.ColumnDefinitions.Add(new ColumnDefinition(1, GridUnitType.Star));

        // 左侧目录树
        _directoryTree = new TreeView
        {
            Background = new SolidColorBrush(Color.Parse("#FF252526")),
            BorderThickness = new Avalonia.Thickness(0),
        };
        Grid.SetColumn(_directoryTree, 0);
        mainContent.Children.Add(_directoryTree);

        // 右侧文件网格
        _fileGrid = new ListBox
        {
            Background = new SolidColorBrush(Color.Parse("#FF1E1E1E")),
            BorderThickness = new Avalonia.Thickness(0),
        };
        Grid.SetColumn(_fileGrid, 1);
        mainContent.Children.Add(_fileGrid);

        panel.Children.Add(mainContent);

        // 订阅 ViewModel 变更
        _viewModel.PropertyChanged += OnPropertyChanged;
    }

    public override void Unbind()
    {
        if (_viewModel != null)
            _viewModel.PropertyChanged -= OnPropertyChanged;
        _viewModel = null;
        _controller = null;
        _directoryTree = null;
        _fileGrid = null;
        _breadcrumb = null;
    }

    /// <summary>设置 Controller（由 AvaloniaViewFactory 调用）。</summary>
    public void SetController(ContentBrowserController controller)
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

        // 后退按钮
        var backButton = new Button
        {
            Content = "←",
            Width = 24,
            Height = 24,
        };
        backButton.Click += (_, _) => _controller?.GoBack();
        toolbar.Children.Add(backButton);

        // 刷新按钮
        var refreshButton = new Button
        {
            Content = "↻",
            Width = 24,
            Height = 24,
        };
        refreshButton.Click += (_, _) => _controller?.RefreshDirectory();
        toolbar.Children.Add(refreshButton);

        // 搜索框
        var searchBox = new TextBox
        {
            Watermark = "Search assets...",
            Width = 200,
            Height = 24,
        };
        searchBox.TextChanged += (_, e) =>
        {
            if (_viewModel != null)
                _viewModel.SearchFilter = searchBox.Text ?? "";
        };
        toolbar.Children.Add(searchBox);

        return toolbar;
    }

    /// <summary>创建面包屑导航。</summary>
    private StackPanel CreateBreadcrumbBar()
    {
        var breadcrumbBar = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 2,
            Margin = new Avalonia.Thickness(4, 0, 4, 4),
            Background = new SolidColorBrush(Color.Parse("#FF2D2D30")),
        };

        _breadcrumb = new TextBlock
        {
            Text = _viewModel?.CurrentDirectory ?? "Assets",
            FontSize = 12,
            VerticalAlignment = VerticalAlignment.Center,
            Margin = new Avalonia.Thickness(4),
            Foreground = new SolidColorBrush(Color.Parse("#FFCCCCCC")),
        };
        breadcrumbBar.Children.Add(_breadcrumb);

        return breadcrumbBar;
    }

    /// <summary>ViewModel 属性变更。</summary>
    private void OnPropertyChanged(string propertyName)
    {
        if (propertyName == nameof(ContentBrowserViewModel.CurrentDirectory))
        {
            if (_breadcrumb != null)
                _breadcrumb.Text = _viewModel?.CurrentDirectory ?? "Assets";
        }
    }
}
