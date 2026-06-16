using System.IO;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Layout;
using Avalonia.Media;
using Neverness.Editor.Assets.Private.Context;
using Neverness.Editor.AvaloniaFrontend.ContextMenus;
using Neverness.Editor.Core.Controllers;
using Neverness.Editor.Core.Public;
using Neverness.Editor.Core.ViewModels;
using Neverness.Editor.Framework.Private.Menu;
using Neverness.Runtime.Assets;
using AssetsContentBrowser = Neverness.Editor.Assets.Private.Core.ContentBrowser;
using AssetsContentFile = Neverness.Editor.Assets.Private.Core.ContentFile;
using AssetsContentDirectory = Neverness.Editor.Assets.Private.Core.ContentDirectory;

namespace Neverness.Editor.AvaloniaFrontend.Views;

/// <summary>
/// 内容浏览器 Avalonia View——显示资产目录树和文件列表。
///
/// 实现细节：
/// - 左侧 TreeView 显示目录树
/// - 右侧 ListBox 显示文件列表（列表模式）
/// - 面包屑导航（可点击跳转）
/// - 搜索过滤
/// - 绑定到 ContentBrowserViewModel + ContentBrowserController
/// </summary>
public class ContentBrowserAvaloniaView : AvaloniaViewBase
{
    private ContentBrowserViewModel? _viewModel;
    private ContentBrowserController? _controller;
    private TreeView? _directoryTree;
    private WrapPanel? _fileGrid;
    private ScrollViewer? _fileScroll;
    private StackPanel? _breadcrumbBar;
    private ScrollViewer? _breadcrumbScroll;

    // 右键菜单（使用 ContextMenuManager + AvaloniaContextMenuRenderer）
    private AvaloniaContextMenuRenderer? _contextMenuRenderer;
    private string? _selectedItemPath;
    private string? _selectedItemName;
    private bool _selectedItemIsDirectory;

    // 缩略图尺寸
    private const double ThumbSize = 90;
    private const double ThumbTextHeight = 32;
    private const double ThumbTotalHeight = ThumbSize + ThumbTextHeight;
    private const double ThumbPadding = 8;

    public ContentBrowserAvaloniaView() : base("ContentBrowser")
    {
    }

    public override Type ViewModelType => typeof(ContentBrowserViewModel);

    public override void Bind(object viewModel)
    {
        _viewModel = (ContentBrowserViewModel)viewModel;

        var panel = new DockPanel();

        // ── 工具栏 ──
        var toolbar = CreateToolbar();
        DockPanel.SetDock(toolbar, Avalonia.Controls.Dock.Top);
        panel.Children.Add(toolbar);

        // ── 面包屑导航 ──
        _breadcrumbScroll = new ScrollViewer
        {
            HorizontalScrollBarVisibility = Avalonia.Controls.Primitives.ScrollBarVisibility.Auto,
            VerticalScrollBarVisibility = Avalonia.Controls.Primitives.ScrollBarVisibility.Disabled,
            Background = new SolidColorBrush(Color.Parse("#FF2D2D30")),
            Height = 28,
        };
        _breadcrumbBar = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 2,
            Margin = new Thickness(4, 2),
            VerticalAlignment = VerticalAlignment.Center,
        };
        _breadcrumbScroll.Content = _breadcrumbBar;
        DockPanel.SetDock(_breadcrumbScroll, Avalonia.Controls.Dock.Top);
        panel.Children.Add(_breadcrumbScroll);

        // ── 主内容区：左侧目录树 + 右侧文件列表 ──
        var mainContent = new Grid();
        mainContent.ColumnDefinitions.Add(new ColumnDefinition(200, GridUnitType.Pixel));
        mainContent.ColumnDefinitions.Add(new ColumnDefinition(GridLength.Auto)); // 分割线
        mainContent.ColumnDefinitions.Add(new ColumnDefinition(1, GridUnitType.Star));

        // 左侧目录树
        _directoryTree = new TreeView
        {
            Background = new SolidColorBrush(Color.Parse("#FF252526")),
            BorderThickness = new Thickness(0),
        };
        Grid.SetColumn(_directoryTree, 0);
        mainContent.Children.Add(_directoryTree);

        // 分割线
        var splitter = new GridSplitter
        {
            Width = 4,
            Background = new SolidColorBrush(Color.Parse("#FF3F3F46")),
            HorizontalAlignment = HorizontalAlignment.Stretch,
        };
        Grid.SetColumn(splitter, 1);
        mainContent.Children.Add(splitter);

        // 右侧缩略图网格
        _fileGrid = new WrapPanel
        {
            Orientation = Orientation.Horizontal,
            Margin = new Thickness(ThumbPadding),
        };
        _fileScroll = new ScrollViewer
        {
            Background = new SolidColorBrush(Color.Parse("#FF1E1E1E")),
            BorderThickness = new Thickness(0),
            HorizontalScrollBarVisibility = Avalonia.Controls.Primitives.ScrollBarVisibility.Disabled,
            VerticalScrollBarVisibility = Avalonia.Controls.Primitives.ScrollBarVisibility.Auto,
            Content = _fileGrid,
        };
        Grid.SetColumn(_fileScroll, 2);
        mainContent.Children.Add(_fileScroll);

        panel.Children.Add(mainContent);

        Content = panel;

        // 创建右键菜单渲染器（从 ContextMenuManager 读取 EditorMenuItem）
        _contextMenuRenderer = new AvaloniaContextMenuRenderer();

        // 文件区域右键（空白处 = 背景菜单）
        // PointerPressed 先清除选中，PointerReleased 再判断
        _fileScroll!.PointerPressed += (_, _) => { _selectedItemPath = null; };
        _fileScroll.PointerReleased += OnFileAreaPointerReleased;

        // 目录树右键
        _directoryTree!.PointerReleased += OnDirectoryTreePointerReleased;

        // 订阅 ViewModel 变更
        _viewModel.PropertyChanged += OnPropertyChanged;
        // 注意：数据加载在 SetController() 中执行，因为 Bind() 先于 SetController() 调用
    }

    public override void Unbind()
    {
        if (_viewModel != null)
            _viewModel.PropertyChanged -= OnPropertyChanged;
        _viewModel = null;
        _controller = null;
        _directoryTree = null;
        _fileGrid = null;
        _fileScroll = null;
        _breadcrumbBar = null;
        _breadcrumbScroll = null;
    }

    public void SetController(ContentBrowserController controller)
    {
        _controller = controller;

        // Controller 就绪后加载数据
        RefreshDirectoryTree();
        RefreshBreadcrumb();
        RefreshFileList();
    }

    // ── 工具栏 ──
    private StackPanel CreateToolbar()
    {
        var toolbar = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 4,
            Margin = new Thickness(4),
        };

        var backButton = new Button
        {
            Content = "←",
            MinWidth = 24,
            MinHeight = 24,
            Padding = new Thickness(4),
        };
        backButton.Click += (_, _) => _controller?.GoBack();
        toolbar.Children.Add(backButton);

        var refreshButton = new Button
        {
            Content = "↻",
            MinWidth = 24,
            MinHeight = 24,
            Padding = new Thickness(4),
        };
        refreshButton.Click += (_, _) => _controller?.RefreshDirectory();
        toolbar.Children.Add(refreshButton);

        var searchBox = new TextBox
        {
            PlaceholderText = "Search assets...",
            MinWidth = 200,
            MinHeight = 24,
        };
        searchBox.TextChanged += (_, _) =>
        {
            if (_viewModel != null)
                _viewModel.SearchFilter = searchBox.Text ?? "";
        };
        toolbar.Children.Add(searchBox);

        return toolbar;
    }

    // ── 面包屑导航 ──
    private void RefreshBreadcrumb()
    {
        if (_breadcrumbBar == null || _viewModel == null) return;

        _breadcrumbBar.Children.Clear();

        var currentDir = _viewModel.CurrentDirectory;
        var assetDir = _viewModel.AssetDirectory;

        if (string.IsNullOrEmpty(currentDir) || string.IsNullOrEmpty(assetDir))
            return;

        // 根目录按钮
        var rootButton = new Button
        {
            Content = "Assets",
            Padding = new Thickness(4, 0),
            Background = Brushes.Transparent,
            BorderThickness = new Thickness(0),
            Foreground = new SolidColorBrush(Color.Parse("#FFCCCCCC")),
            FontSize = 12,
        };
        rootButton.Click += (_, _) => _controller?.OpenDirectory(assetDir);
        _breadcrumbBar.Children.Add(rootButton);

        // 相对路径分段
        var relativePath = Path.GetRelativePath(assetDir, currentDir);
        if (relativePath == ".") return;

        var segments = relativePath.Split(Path.DirectorySeparatorChar, StringSplitOptions.RemoveEmptyEntries);
        var accumulatedPath = assetDir;

        foreach (var segment in segments)
        {
            accumulatedPath = Path.Combine(accumulatedPath, segment);

            _breadcrumbBar.Children.Add(new TextBlock
            {
                Text = ">",
                Foreground = new SolidColorBrush(Color.Parse("#FF656565")),
                FontSize = 12,
                VerticalAlignment = VerticalAlignment.Center,
            });

            var segmentPath = accumulatedPath; // 闭包捕获
            var segmentButton = new Button
            {
                Content = segment,
                Padding = new Thickness(4, 0),
                Background = Brushes.Transparent,
                BorderThickness = new Thickness(0),
                Foreground = new SolidColorBrush(Color.Parse("#FFCCCCCC")),
                FontSize = 12,
            };
            segmentButton.Click += (_, _) => _controller?.OpenDirectory(segmentPath);
            _breadcrumbBar.Children.Add(segmentButton);
        }
    }

    // ── 目录树 ──
    private void RefreshDirectoryTree()
    {
        if (_directoryTree == null || _controller == null) return;

        _directoryTree.Items.Clear();

        // 单击目录树节点 → 打开对应目录（先移除旧监听防止重复）
        _directoryTree.SelectionChanged -= OnDirectoryTreeSelectionChanged;
        _directoryTree.SelectionChanged += OnDirectoryTreeSelectionChanged;

        try
        {
            var root = _controller.GetDirectoryTreeRoot();
            if (root == null) return;

            // 根节点
            var rootItem = new TreeViewItem
            {
                Header = CreateNodeHeader("📁", "Assets"),
                IsExpanded = true,
                Tag = root.SystemPath.FullPath,
            };

            // 递归添加子目录
            foreach (var child in root.Directories)
            {
                rootItem.Items.Add(CreateDirectoryNode(child));
            }

            _directoryTree.Items.Add(rootItem);
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"[ContentBrowser] 目录树加载失败: {ex.Message}");
        }
    }

    private void OnDirectoryTreeSelectionChanged(object? sender, SelectionChangedEventArgs e)
    {
        if (_controller == null || e.AddedItems.Count == 0) return;

        if (e.AddedItems[0] is TreeViewItem item && item.Tag is string path)
        {
            _controller.OpenDirectory(path);
        }
    }

    private TreeViewItem CreateDirectoryNode(ContentDirectoryNode node)
    {
        var item = new TreeViewItem
        {
            Header = CreateNodeHeader("📁", node.Name),
            Tag = node.SystemPath.FullPath,
        };

        foreach (var child in node.Directories)
        {
            item.Items.Add(CreateDirectoryNode(child));
        }

        return item;
    }

    private StackPanel CreateNodeHeader(string icon, string name)
    {
        var header = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 4,
        };
        header.Children.Add(new TextBlock
        {
            Text = icon,
            FontSize = 12,
        });
        header.Children.Add(new TextBlock
        {
            Text = name,
            FontSize = 12,
            VerticalAlignment = VerticalAlignment.Center,
        });
        return header;
    }

    // ── 缩略图网格 ──
    private void RefreshFileList()
    {
        if (_fileGrid == null || _controller == null) return;

        _fileGrid.Children.Clear();

        try
        {
            // 子目录
            var subdirectories = _controller.GetSubdirectories();
            foreach (var dir in subdirectories)
            {
                _fileGrid.Children.Add(CreateThumbnail(dir.Name, "📁", dir.SystemPath.FullPath, true));
            }

            // 文件
            var files = _controller.GetFiles();
            foreach (var file in files)
            {
                var icon = GetFileIcon(file.Extension);
                _fileGrid.Children.Add(CreateThumbnail(file.Name, icon, file.SystemPath.FullPath, false));
            }
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"[ContentBrowser] 文件列表加载失败: {ex.Message}");
        }
    }

    /// <summary>创建 90x90 缩略图控件。</summary>
    private Control CreateThumbnail(string name, string icon, string path, bool isDirectory)
    {
        // 整个缩略图容器
        var container = new StackPanel
        {
            Width = ThumbSize + ThumbPadding * 2,
            Height = ThumbTotalHeight,
            Margin = new Thickness(2),
            Tag = path,
        };

        // 图标区域（90x90）
        var iconArea = new Panel
        {
            Width = ThumbSize,
            Height = ThumbSize,
            Background = new SolidColorBrush(Color.Parse(isDirectory ? "#FF3F3F46" : "#FF2D2D30")),
            HorizontalAlignment = HorizontalAlignment.Center,
        };
        iconArea.Children.Add(new TextBlock
        {
            Text = icon,
            FontSize = 32,
            HorizontalAlignment = HorizontalAlignment.Center,
            VerticalAlignment = VerticalAlignment.Center,
        });
        container.Children.Add(iconArea);

        // 文件名区域
        var nameText = new TextBlock
        {
            Text = name,
            FontSize = 11,
            Foreground = Brushes.White,
            HorizontalAlignment = HorizontalAlignment.Center,
            TextTrimming = TextTrimming.CharacterEllipsis,
            MaxLines = 2,
            TextAlignment = TextAlignment.Center,
            Margin = new Thickness(2, 4, 2, 0),
            Width = ThumbSize,
        };
        container.Children.Add(nameText);

        // 双击导航
        container.DoubleTapped += (_, _) =>
        {
            if (isDirectory)
                _controller?.OpenDirectory(path);
            else
                _controller?.OpenFile(path);
        };

        // 右键菜单（记录选中项后弹出菜单）
        container.PointerReleased += (s, e) =>
        {
            if (e.InitialPressMouseButton != MouseButton.Right) return;

            _selectedItemPath = path;
            _selectedItemName = name;
            _selectedItemIsDirectory = isDirectory;

            // 设置 ContextMenuManager 上下文
            var ctx = ContextMenuManager.Instance;
            ctx.SetContext(ContentBrowserContextMenu.KeyPath, path);
            ctx.SetContext(ContentBrowserContextMenu.KeyContentBrowser, AssetsContentBrowser.Instance);

            // 创建 ContentItem 供菜单命令使用（Rename/Remove/ShowInExplorer/CopyName）
            Neverness.Editor.Assets.Private.Core.ContentItem contentItem = isDirectory
                ? new AssetsContentDirectory { Name = name, SystemPath = new NPath(path) }
                : new AssetsContentFile { Name = name, SystemPath = new NPath(path), Extension = System.IO.Path.GetExtension(path) };
            ctx.SetContext(ContentBrowserContextMenu.KeyItem, contentItem);

            // 渲染 Item 菜单
            _contextMenuRenderer?.BuildAndShow(
                ContentBrowserContextMenu.ItemId,
                ctx,
                s as Control ?? container);
            e.Handled = true;
        };

        // 悬停高亮
        var iconBg = isDirectory ? "#FF3F3F46" : "#FF2D2D30";
        var hoverBg = "#FF3E3E42";
        container.PointerEntered += (_, _) =>
        {
            iconArea.Background = new SolidColorBrush(Color.Parse(hoverBg));
            container.Cursor = new Avalonia.Input.Cursor(Avalonia.Input.StandardCursorType.Hand);
        };
        container.PointerExited += (_, _) =>
        {
            iconArea.Background = new SolidColorBrush(Color.Parse(iconBg));
        };

        return container;
    }

    private static string GetFileIcon(string extension)
    {
        return extension?.ToLower() switch
        {
            ".png" or ".jpg" or ".jpeg" or ".bmp" or ".tga" => "🖼️",
            ".fbx" or ".obj" or ".gltf" or ".glb" => "🧊",
            ".wav" or ".mp3" or ".ogg" => "🎵",
            ".mp4" or ".avi" or ".mov" => "🎬",
            ".cs" or ".cpp" or ".h" or ".py" => "📝",
            ".json" or ".xml" or ".yaml" or ".yml" => "📋",
            ".txt" or ".md" => "📄",
            ".shader" or ".hlsl" or ".glsl" => "☀️",
            ".scene" => "🗺️",
            ".prefab" => "🧊",
            ".html" => "🌐",
            ".lua" => "📜",
            _ => "📄"
        };
    }

    // ── 右键事件处理 ──

    /// <summary>文件区域右键——判断点击的是缩略图还是空白处。</summary>
    private void OnFileAreaPointerReleased(object? sender, PointerReleasedEventArgs e)
    {
        if (e.InitialPressMouseButton != MouseButton.Right) return;

        // 如果 _selectedItemPath 已被缩略图的 PointerReleased 设置过，说明点在了项目上
        if (!string.IsNullOrEmpty(_selectedItemPath)) return;

        // 点在空白处 → 背景菜单
        var ctx = ContextMenuManager.Instance;
        ctx.SetContext(ContentBrowserContextMenu.KeyPath, _viewModel?.CurrentDirectory ?? "");
        ctx.SetContext(ContentBrowserContextMenu.KeyContentBrowser, AssetsContentBrowser.Instance);

        _contextMenuRenderer?.BuildAndShow(
            ContentBrowserContextMenu.BackgroundId,
            ctx,
            _fileScroll!);

        e.Handled = true;
    }

    /// <summary>目录树右键。</summary>
    private void OnDirectoryTreePointerReleased(object? sender, PointerReleasedEventArgs e)
    {
        if (e.InitialPressMouseButton != MouseButton.Right) return;

        // 获取右键点击的节点
        if (_directoryTree?.SelectedItem is TreeViewItem item && item.Tag is string path)
        {
            _selectedItemPath = path;
            _selectedItemName = (item.Header as StackPanel)?.Children
                .OfType<TextBlock>().LastOrDefault()?.Text ?? "";
            _selectedItemIsDirectory = true;

            // 设置上下文
            var ctx = ContextMenuManager.Instance;
            ctx.SetContext(ContentBrowserContextMenu.KeyPath, path);
            ctx.SetContext(ContentBrowserContextMenu.KeyContentBrowser, AssetsContentBrowser.Instance);

            // 渲染背景菜单（目录节点也用背景菜单：Create Directory / Refresh / Show in Explorer）
            _contextMenuRenderer?.BuildAndShow(
                ContentBrowserContextMenu.BackgroundId,
                ctx,
                item);
        }

        e.Handled = true;
    }

    // ── ViewModel 属性变更 ──
    private void OnPropertyChanged(string? propertyName)
    {
        if (propertyName == nameof(ContentBrowserViewModel.CurrentDirectory))
        {
            RefreshBreadcrumb();
            RefreshFileList();
            // TODO: 高亮目录树中对应的节点
        }
        else if (propertyName == nameof(ContentBrowserViewModel.SearchFilter))
        {
            // TODO: 根据搜索过滤文件列表
            RefreshFileList();
        }
    }
}
