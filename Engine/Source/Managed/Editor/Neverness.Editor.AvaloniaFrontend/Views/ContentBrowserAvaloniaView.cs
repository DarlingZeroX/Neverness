using System.IO;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Layout;
using Avalonia.Media;
using Neverness.Editor.Assets.Private.Context;
using Neverness.Editor.AvaloniaFrontend.ContextMenus;
using Neverness.Editor.AvaloniaFrontend.ContentBrowser;
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
/// 内容浏览器 Avalonia View——Unreal Content Browser 风格。
///
/// 布局：
/// ┌─────────────────────────────────────────────┐
/// │ [◀][▶]  Content/Scenes         [🔍 Search]  │ ← 工具栏
/// ├──────────┬────────────────────────────────────┤
/// │ ▾ Content│ ┌──────┐ ┌──────┐ ┌──────┐       │
/// │   Scenes │ │ SCENE│ │ PREFB│ │ MATER│       │
/// │   Prefabs│ │  🗺️  │ │  🧊  │ │  📋  │       │
/// │          │ │ main │ │player│ │ tree │       │
/// │          │ └──────┘ └──────┘ └──────┘       │
/// ├──────────┴────────────────────────────────────┤
/// │ Content/Scenes                  2 items sel.  │ ← 状态栏
/// └─────────────────────────────────────────────┘
/// </summary>
public class ContentBrowserAvaloniaView : AvaloniaViewBase
{
    /* ======================== ViewModel / Controller ======================== */

    private ContentBrowserViewModel? _viewModel;
    private ContentBrowserController? _controller;

    /* ======================== 控件引用 ======================== */

    private TreeView? _directoryTree;
    private WrapPanel? _fileGrid;
    private ScrollViewer? _fileScroll;
    private StackPanel? _breadcrumbBar;
    private ScrollViewer? _breadcrumbScroll;
    private TextBlock? _statusText;
    private TextBlock? _selectionStatus;

    // 框选
    private Panel? _selectionCanvas;
    private Border? _selectionBorder;
    private RubberBandSelection? _rubberBand;

    // 右键菜单
    private AvaloniaContextMenuRenderer? _contextMenuRenderer;
    private string? _selectedItemPath;
    private string? _selectedItemName;
    private bool _selectedItemIsDirectory;

    // 选中状态
    private readonly HashSet<string> _selectedPaths = new();
    private readonly Dictionary<string, Border> _thumbnailBorders = new();

    /* ======================== Unreal 风格配色 ======================== */

    // 主背景
    private static readonly IBrush BgMain = new SolidColorBrush(Color.Parse("#FF2B2B2B"));
    private static readonly IBrush BgPanel = new SolidColorBrush(Color.Parse("#FF353535"));
    private static readonly IBrush BgToolbar = new SolidColorBrush(Color.Parse("#FF3B3B3B"));
    private static readonly IBrush BgStatusBar = new SolidColorBrush(Color.Parse("#FF252525"));
    private static readonly IBrush BgInput = new SolidColorBrush(Color.Parse("#FF1E1E1E"));

    // 缩略图
    private static readonly IBrush BgThumbFile = new SolidColorBrush(Color.Parse("#FF404040"));
    private static readonly IBrush BgThumbDir = new SolidColorBrush(Color.Parse("#FF484848"));
    private static readonly IBrush BgThumbHover = new SolidColorBrush(Color.Parse("#FF505050"));
    private static readonly IBrush BgThumbSelected = new SolidColorBrush(Color.Parse("#FF2A5D9E"));

    // 选区矩形
    private static readonly IBrush SelectionFill = new SolidColorBrush(Color.FromArgb(0x30, 0x21, 0x96, 0xF3));
    private static readonly IBrush SelectionStroke = new SolidColorBrush(Color.FromArgb(0x80, 0x21, 0x96, 0xF3));

    // 选中边框
    private static readonly IBrush BorderSelected = new SolidColorBrush(Color.Parse("#FF2196F3"));

    // 文字
    private static readonly IBrush TextPrimary = new SolidColorBrush(Color.Parse("#FFCCCCCC"));
    private static readonly IBrush TextSecondary = new SolidColorBrush(Color.Parse("#FF888888"));
    private static readonly IBrush TextBright = new SolidColorBrush(Color.Parse("#FFFFFFFF"));

    // 分割线
    private static readonly IBrush Separator = new SolidColorBrush(Color.Parse("#FF1A1A1A"));

    // 类型标签颜色
    private static readonly IBrush BadgeScene = new SolidColorBrush(Color.Parse("#FF4CAF50"));
    private static readonly IBrush BadgePrefab = new SolidColorBrush(Color.Parse("#FF2196F3"));
    private static readonly IBrush BadgeMaterial = new SolidColorBrush(Color.Parse("#FFFF9800"));
    private static readonly IBrush BadgeTexture = new SolidColorBrush(Color.Parse("#FF9C27B0"));
    private static readonly IBrush BadgeAudio = new SolidColorBrush(Color.Parse("#FFFF5722"));
    private static readonly IBrush BadgeScript = new SolidColorBrush(Color.Parse("#FF00BCD4"));
    private static readonly IBrush BadgeDefault = new SolidColorBrush(Color.Parse("#FF607D8B"));

    /* ======================== 缩略图尺寸 ======================== */

    private const double ThumbIconSize = 80;   // 图标区域高度
    private const double ThumbNameHeight = 16;  // 文件名行高
    private const double ThumbTypeHeight = 12;  // 类型标签行高
    private const double ThumbTextPad = 5;      // 文字区内边距
    private const double ThumbCardWidth = 96;    // 卡片总宽
    private const double ThumbCardHeight = ThumbIconSize + ThumbNameHeight + ThumbTypeHeight + ThumbTextPad * 2;
    private const double ThumbSpacing = 5;

    /* ======================== 构造 / 绑定 ======================== */

    public ContentBrowserAvaloniaView() : base("ContentBrowser") { }

    public override Type ViewModelType => typeof(ContentBrowserViewModel);

    public override void Bind(object viewModel)
    {
        _viewModel = (ContentBrowserViewModel)viewModel;

        var root = new DockPanel { Background = BgMain };

        // ── 工具栏（顶部）──
        var toolbar = CreateToolbar();
        DockPanel.SetDock(toolbar, Avalonia.Controls.Dock.Top);
        root.Children.Add(toolbar);

        // ── 状态栏（底部）──
        var statusBar = CreateStatusBar();
        DockPanel.SetDock(statusBar, Avalonia.Controls.Dock.Bottom);
        root.Children.Add(statusBar);

        // ── 主内容区 ──
        var mainContent = new Grid();
        mainContent.ColumnDefinitions.Add(new ColumnDefinition(220, GridUnitType.Pixel) { MinWidth = 120 });
        mainContent.ColumnDefinitions.Add(new ColumnDefinition(4, GridUnitType.Pixel));
        mainContent.ColumnDefinitions.Add(new ColumnDefinition(1, GridUnitType.Star));

        // 左侧目录树面板（右侧内阴影，左侧无阴影）
        var treePanel = new DockPanel { Background = BgPanel };
        var treeRightShadow = new Border
        {
            Background = Brushes.Transparent,
            IsHitTestVisible = false,
            BoxShadow = new BoxShadows(new BoxShadow
            {
                IsInset = true,
                Color = Color.FromArgb(0xA0, 0x00, 0x00, 0x00),
                Blur = 14,
                Spread = -2,
                OffsetX = -8,
                OffsetY = 0,
            }),
        };
        var treeShadow = new Grid { [Grid.ColumnProperty] = 0, ClipToBounds = true };
        treeShadow.Children.Add(treePanel);
        treeShadow.Children.Add(treeRightShadow);

        var treeHeader = new TextBlock
        {
            Text = "Content",
            FontSize = 11,
            FontWeight = FontWeight.Bold,
            Foreground = TextSecondary,
            Margin = new Thickness(12, 8, 12, 6),
        };
        DockPanel.SetDock(treeHeader, Avalonia.Controls.Dock.Top);
        treePanel.Children.Add(treeHeader);

        _directoryTree = new TreeView
        {
            Background = Brushes.Transparent,
            BorderThickness = new Thickness(0),
            Padding = new Thickness(4, 0, 4, 4),
        };
        treePanel.Children.Add(_directoryTree);

        mainContent.Children.Add(treeShadow);

        // 可拖拽分割线
        var splitter = new GridSplitter
        {
            Width = 4,
            Background = Separator,
            HorizontalAlignment = HorizontalAlignment.Stretch,
            Cursor = new Cursor(StandardCursorType.SizeWestEast),
            [Grid.ColumnProperty] = 1,
        };
        mainContent.Children.Add(splitter);

        // 右侧缩略图区域
        _fileGrid = new WrapPanel
        {
            Orientation = Orientation.Horizontal,
            Margin = new Thickness(ThumbSpacing),
        };

        // 选区矩形（叠加在 WrapPanel 上方，不拦截鼠标）
        _selectionBorder = new Border
        {
            Background = SelectionFill,
            BorderBrush = SelectionStroke,
            BorderThickness = new Thickness(1),
            CornerRadius = new CornerRadius(2),
            IsHitTestVisible = false,
            IsVisible = false,
        };

        // Grid 容器：WrapPanel 受 Grid 宽度约束自动换行，
        // 选区矩形叠加在同一单元格（ZIndex 更高）
        _selectionCanvas = new Grid { ClipToBounds = true, Background = Brushes.Transparent };
        _selectionCanvas.Children.Add(_fileGrid);
        _selectionCanvas.Children.Add(_selectionBorder);

        _fileScroll = new ScrollViewer
        {
            Background = BgMain,
            BorderThickness = new Thickness(0),
            HorizontalScrollBarVisibility = Avalonia.Controls.Primitives.ScrollBarVisibility.Disabled,
            VerticalScrollBarVisibility = Avalonia.Controls.Primitives.ScrollBarVisibility.Auto,
            Content = _selectionCanvas,
        };

        // 右侧面板（左侧内阴影，右侧无阴影）
        var fileLeftShadow = new Border
        {
            Background = Brushes.Transparent,
            IsHitTestVisible = false,
            BoxShadow = new BoxShadows(new BoxShadow
            {
                IsInset = true,
                Color = Color.FromArgb(0xA0, 0x00, 0x00, 0x00),
                Blur = 14,
                Spread = -2,
                OffsetX = 8,
                OffsetY = 0,
            }),
        };
        var fileShadow = new Grid { [Grid.ColumnProperty] = 2, ClipToBounds = true };
        fileShadow.Children.Add(_fileScroll);
        fileShadow.Children.Add(fileLeftShadow);
        mainContent.Children.Add(fileShadow);

        root.Children.Add(mainContent);

        Content = root;

        // 框选管理器
        _contextMenuRenderer = new AvaloniaContextMenuRenderer();
        _rubberBand = new RubberBandSelection(
            _fileScroll!, _selectionCanvas!, _fileGrid!, _selectionBorder!,
            OnRubberBandSelectionChanged);
        _rubberBand.Attach();

        // 事件
        _fileScroll!.PointerPressed += (_, _) => { _selectedItemPath = null; };
        _fileScroll.PointerReleased += OnFileAreaPointerReleased;
        _directoryTree!.PointerReleased += OnDirectoryTreePointerReleased;
        _viewModel.PropertyChanged += OnPropertyChanged;
    }

    public override void Unbind()
    {
        _rubberBand?.Detach();
        _rubberBand = null;
        if (_viewModel != null)
            _viewModel.PropertyChanged -= OnPropertyChanged;
        _viewModel = null;
        _controller = null;
        _directoryTree = null;
        _fileGrid = null;
        _fileScroll = null;
        _breadcrumbBar = null;
        _breadcrumbScroll = null;
        _selectionCanvas = null;
        _selectionBorder = null;
        _statusText = null;
        _selectionStatus = null;
    }

    public void SetController(ContentBrowserController controller)
    {
        _controller = controller;
        RefreshDirectoryTree();
        RefreshBreadcrumb();
        RefreshFileList();
    }

    /* ======================== 工具栏 ======================== */

    private Control CreateToolbar()
    {
        var toolbar = new DockPanel
        {
            Background = BgToolbar,
            Height = 36,
            Margin = new Thickness(0),
        };

        // 导航按钮
        var navPanel = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 1,
            Margin = new Thickness(4, 4, 8, 4),
            VerticalAlignment = VerticalAlignment.Center,
        };

        var backBtn = CreateToolButton("◀", 28);
        backBtn.Click += (_, _) => _controller?.GoBack();
        navPanel.Children.Add(backBtn);

        var fwdBtn = CreateToolButton("▶", 28);
        navPanel.Children.Add(fwdBtn);

        DockPanel.SetDock(navPanel, Avalonia.Controls.Dock.Left);
        toolbar.Children.Add(navPanel);

        // 搜索框（右侧）
        var searchBox = new TextBox
        {
            PlaceholderText = "🔍  Search...",
            Background = BgInput,
            Foreground = TextPrimary,
            BorderBrush = Separator,
            BorderThickness = new Thickness(1),
            CornerRadius = new CornerRadius(3),
            MinWidth = 180,
            MaxWidth = 260,
            Height = 24,
            Padding = new Thickness(8, 2),
            FontSize = 12,
            VerticalAlignment = VerticalAlignment.Center,
            Margin = new Thickness(0, 0, 8, 0),
        };
        searchBox.TextChanged += (_, _) =>
        {
            if (_viewModel != null)
                _viewModel.SearchFilter = searchBox.Text ?? "";
        };
        DockPanel.SetDock(searchBox, Avalonia.Controls.Dock.Right);
        toolbar.Children.Add(searchBox);

        // 面包屑（中间填充）
        _breadcrumbBar = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 0,
            VerticalAlignment = VerticalAlignment.Center,
            Margin = new Thickness(8, 0, 8, 0),
        };
        _breadcrumbScroll = new ScrollViewer
        {
            HorizontalScrollBarVisibility = Avalonia.Controls.Primitives.ScrollBarVisibility.Auto,
            VerticalScrollBarVisibility = Avalonia.Controls.Primitives.ScrollBarVisibility.Disabled,
            Content = _breadcrumbBar,
            VerticalAlignment = VerticalAlignment.Center,
        };
        toolbar.Children.Add(_breadcrumbScroll);

        return toolbar;
    }

    private Button CreateToolButton(string text, double size)
    {
        var btn = new Button
        {
            Content = text,
            Width = size,
            Height = size,
            Padding = new Thickness(0),
            Background = Brushes.Transparent,
            BorderThickness = new Thickness(0),
            Foreground = TextSecondary,
            FontSize = 12,
            CornerRadius = new CornerRadius(3),
            HorizontalContentAlignment = HorizontalAlignment.Center,
            VerticalContentAlignment = VerticalAlignment.Center,
        };
        btn.PointerEntered += (_, _) => btn.Background = new SolidColorBrush(Color.Parse("#FF4A4A4A"));
        btn.PointerExited += (_, _) => btn.Background = Brushes.Transparent;
        return btn;
    }

    /* ======================== 状态栏 ======================== */

    private Control CreateStatusBar()
    {
        var bar = new DockPanel
        {
            Background = BgStatusBar,
            Height = 24,
            Margin = new Thickness(0),
        };

        _statusText = new TextBlock
        {
            Text = "",
            FontSize = 11,
            Foreground = TextSecondary,
            VerticalAlignment = VerticalAlignment.Center,
            Margin = new Thickness(12, 0, 0, 0),
        };
        DockPanel.SetDock(_statusText, Avalonia.Controls.Dock.Left);
        bar.Children.Add(_statusText);

        _selectionStatus = new TextBlock
        {
            Text = "",
            FontSize = 11,
            Foreground = TextSecondary,
            VerticalAlignment = VerticalAlignment.Center,
            Margin = new Thickness(0, 0, 12, 0),
            HorizontalAlignment = HorizontalAlignment.Right,
        };
        DockPanel.SetDock(_selectionStatus, Avalonia.Controls.Dock.Right);
        bar.Children.Add(_selectionStatus);

        return bar;
    }

    private void UpdateStatusBar()
    {
        if (_statusText != null)
        {
            var dir = _viewModel?.CurrentDirectory ?? "";
            var assetDir = _viewModel?.AssetDirectory ?? "";
            _statusText.Text = string.IsNullOrEmpty(assetDir) ? dir : Path.GetRelativePath(assetDir, dir);
        }

        if (_selectionStatus != null)
        {
            var count = _selectedPaths.Count;
            _selectionStatus.Text = count > 0 ? $"{count} item{(count > 1 ? "s" : "")} selected" : "";
        }
    }

    /* ======================== 面包屑 ======================== */

    private void RefreshBreadcrumb()
    {
        if (_breadcrumbBar == null || _viewModel == null) return;

        _breadcrumbBar.Children.Clear();

        var currentDir = _viewModel.CurrentDirectory;
        var assetDir = _viewModel.AssetDirectory;

        if (string.IsNullOrEmpty(currentDir) || string.IsNullOrEmpty(assetDir))
            return;

        // 根目录
        AddBreadcrumbSegment("Content", assetDir, isFirst: true);

        var relativePath = Path.GetRelativePath(assetDir, currentDir);
        if (relativePath == ".") return;

        var segments = relativePath.Split(Path.DirectorySeparatorChar, StringSplitOptions.RemoveEmptyEntries);
        var accumulatedPath = assetDir;

        foreach (var segment in segments)
        {
            accumulatedPath = Path.Combine(accumulatedPath, segment);
            AddBreadcrumbSegment(segment, accumulatedPath, isFirst: false);
        }

        UpdateStatusBar();
    }

    private void AddBreadcrumbSegment(string label, string path, bool isFirst)
    {
        if (!isFirst)
        {
            _breadcrumbBar!.Children.Add(new TextBlock
            {
                Text = "›",
                Foreground = TextSecondary,
                FontSize = 12,
                VerticalAlignment = VerticalAlignment.Center,
                Margin = new Thickness(4, 0),
            });
        }

        var btn = new Button
        {
            Content = label,
            Padding = new Thickness(4, 1),
            Background = Brushes.Transparent,
            BorderThickness = new Thickness(0),
            Foreground = TextPrimary,
            FontSize = 12,
            CornerRadius = new CornerRadius(2),
        };
        var capturedPath = path;
        btn.Click += (_, _) => _controller?.OpenDirectory(capturedPath);
        btn.PointerEntered += (_, _) => btn.Foreground = TextBright;
        btn.PointerExited += (_, _) => btn.Foreground = TextPrimary;
        _breadcrumbBar!.Children.Add(btn);
    }

    /* ======================== 目录树 ======================== */

    private void RefreshDirectoryTree()
    {
        if (_directoryTree == null || _controller == null) return;

        _directoryTree.Items.Clear();
        _directoryTree.SelectionChanged -= OnDirectoryTreeSelectionChanged;
        _directoryTree.SelectionChanged += OnDirectoryTreeSelectionChanged;

        try
        {
            var root = _controller.GetDirectoryTreeRoot();
            if (root == null) return;

            var rootItem = new TreeViewItem
            {
                Header = CreateTreeHeader("Content", isRoot: true),
                IsExpanded = true,
                Tag = root.SystemPath.FullPath,
            };

            foreach (var child in root.Directories)
                rootItem.Items.Add(CreateTreeNode(child));

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
            _controller.OpenDirectory(path);
    }

    private TreeViewItem CreateTreeNode(ContentDirectoryNode node)
    {
        var item = new TreeViewItem
        {
            Header = CreateTreeHeader(node.Name, isRoot: false),
            Tag = node.SystemPath.FullPath,
        };

        foreach (var child in node.Directories)
            item.Items.Add(CreateTreeNode(child));

        return item;
    }

    private StackPanel CreateTreeHeader(string name, bool isRoot)
    {
        var header = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 6,
            Margin = new Thickness(0, 1),
        };

        header.Children.Add(new TextBlock
        {
            Text = isRoot ? "▾" : "▸",
            FontSize = 10,
            Foreground = TextSecondary,
            VerticalAlignment = VerticalAlignment.Center,
            Width = 12,
            TextAlignment = TextAlignment.Center,
        });

        header.Children.Add(new TextBlock
        {
            Text = "📁",
            FontSize = 13,
            VerticalAlignment = VerticalAlignment.Center,
        });

        header.Children.Add(new TextBlock
        {
            Text = name,
            FontSize = 12,
            Foreground = isRoot ? TextBright : TextPrimary,
            FontWeight = isRoot ? FontWeight.Bold : FontWeight.Normal,
            VerticalAlignment = VerticalAlignment.Center,
        });

        return header;
    }

    private void HighlightDirectoryTreeNode(string? targetPath)
    {
        if (_directoryTree == null || string.IsNullOrEmpty(targetPath)) return;
        FindAndSelectTreeNode(_directoryTree.Items, targetPath);
    }

    private bool FindAndSelectTreeNode(System.Collections.IList items, string targetPath)
    {
        foreach (var obj in items)
        {
            if (obj is not TreeViewItem item) continue;

            if (item.Tag is string path && path == targetPath)
            {
                item.IsSelected = true;
                item.BringIntoView();
                return true;
            }

            if (FindAndSelectTreeNode(item.Items, targetPath))
            {
                item.IsExpanded = true;
                return true;
            }
        }
        return false;
    }

    /* ======================== 缩略图网格 ======================== */

    private void RefreshFileList()
    {
        if (_fileGrid == null || _controller == null) return;

        _fileGrid.Children.Clear();
        _thumbnailBorders.Clear();

        var filter = _viewModel?.SearchFilter?.Trim() ?? "";

        try
        {
            // 子目录
            foreach (var dir in _controller.GetSubdirectories())
            {
                if (!string.IsNullOrEmpty(filter) &&
                    !dir.Name.Contains(filter, StringComparison.OrdinalIgnoreCase))
                    continue;

                _fileGrid.Children.Add(CreateThumbnail(dir.Name, "📁", dir.SystemPath.FullPath, isDirectory: true, typeLabel: "DIR"));
            }

            // 文件
            foreach (var file in _controller.GetFiles())
            {
                if (!string.IsNullOrEmpty(filter) &&
                    !file.Name.Contains(filter, StringComparison.OrdinalIgnoreCase))
                    continue;

                var (icon, label, badgeColor) = GetFileInfo(file.Extension, file.AssetType);
                _fileGrid.Children.Add(CreateThumbnail(file.Name, icon, file.SystemPath.FullPath, isDirectory: false, typeLabel: label, badgeColor: badgeColor));
            }
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"[ContentBrowser] 文件列表加载失败: {ex.Message}");
        }

        UpdateStatusBar();
    }

    private Control CreateThumbnail(string name, string icon, string path, bool isDirectory, string typeLabel, IBrush? badgeColor = null)
    {
        var isSelected = _selectedPaths.Contains(path);

        // 外层容器（提供阴影空间 + Tag 用于事件识别）
        var shadowPad = 4; // 阴影扩展空间
        var outer = new Border
        {
            Width = ThumbCardWidth + shadowPad * 2,
            Height = ThumbCardHeight + shadowPad * 2,
            Margin = new Thickness(ThumbSpacing / 2 - shadowPad),
            Padding = new Thickness(shadowPad),
            Background = Brushes.Transparent,
            Tag = path,
        };

        // 卡片 Border（圆角 + 选中高亮 + 阴影）
        var card = new Border
        {
            Width = ThumbCardWidth,
            Height = ThumbCardHeight,
            CornerRadius = new CornerRadius(4),
            Background = isSelected ? BgThumbSelected : (isDirectory ? BgThumbDir : BgThumbFile),
            BorderBrush = isSelected ? BorderSelected : Brushes.Transparent,
            BorderThickness = new Thickness(isSelected ? 2 : 0),
            BoxShadow = new BoxShadows(new BoxShadow
            {
                Color = Color.FromArgb(0x60, 0x00, 0x00, 0x00),
                Blur = 6,
                OffsetX = 0,
                OffsetY = 2,
            }),
        };

        outer.Child = card;

        // 卡片内部垂直布局
        var cardContent = new DockPanel();

        // ── 图标区域（居中）──
        var iconArea = new Panel
        {
            Height = ThumbIconSize,
        };
        iconArea.Children.Add(new TextBlock
        {
            Text = icon,
            FontSize = 32,
            HorizontalAlignment = HorizontalAlignment.Center,
            VerticalAlignment = VerticalAlignment.Center,
        });
        DockPanel.SetDock(iconArea, Avalonia.Controls.Dock.Top);
        cardContent.Children.Add(iconArea);

        // ── 文字区域（下方）──
        var textArea = new StackPanel
        {
            Margin = new Thickness(ThumbTextPad, ThumbTextPad, ThumbTextPad, ThumbTextPad),
        };

        // 文件名
        textArea.Children.Add(new TextBlock
        {
            Text = name,
            FontSize = 11,
            Foreground = isSelected ? TextBright : TextPrimary,
            TextTrimming = TextTrimming.CharacterEllipsis,
            MaxLines = 1,
            Height = ThumbNameHeight,
            VerticalAlignment = VerticalAlignment.Center,
        });

        // 资产类型
        textArea.Children.Add(new TextBlock
        {
            Text = typeLabel,
            FontSize = 9,
            Foreground = badgeColor ?? TextSecondary,
            FontWeight = FontWeight.Bold,
            Height = ThumbTypeHeight,
            VerticalAlignment = VerticalAlignment.Center,
        });

        cardContent.Children.Add(textArea);
        card.Child = cardContent;

        _thumbnailBorders[path] = card;

        // ── 交互事件（挂在 outer 上，视觉效果改 card）──

        // 单击选中
        outer.PointerPressed += (_, e) =>
        {
            var point = e.GetCurrentPoint(outer);
            if (!point.Properties.IsLeftButtonPressed) return;

            var isCtrl = e.KeyModifiers.HasFlag(KeyModifiers.Control);
            if (isCtrl)
            {
                if (_selectedPaths.Contains(path))
                    _selectedPaths.Remove(path);
                else
                    _selectedPaths.Add(path);
            }
            else
            {
                _selectedPaths.Clear();
                _selectedPaths.Add(path);
            }

            UpdateSelectionVisuals();
            e.Handled = true;
        };

        // 双击
        outer.DoubleTapped += (_, _) =>
        {
            if (isDirectory) _controller?.OpenDirectory(path);
            else _controller?.OpenFile(path);
        };

        // 右键
        outer.PointerReleased += (s, e) =>
        {
            if (e.InitialPressMouseButton != MouseButton.Right) return;

            _selectedItemPath = path;
            _selectedItemName = name;
            _selectedItemIsDirectory = isDirectory;

            var ctx = ContextMenuManager.Instance;
            ctx.SetContext(ContentBrowserContextMenu.KeyPath, path);
            ctx.SetContext(ContentBrowserContextMenu.KeyContentBrowser, AssetsContentBrowser.Instance);

            Neverness.Editor.Assets.Private.Core.ContentItem contentItem = isDirectory
                ? new AssetsContentDirectory { Name = name, SystemPath = new NPath(path) }
                : new AssetsContentFile { Name = name, SystemPath = new NPath(path), Extension = Path.GetExtension(path) };
            ctx.SetContext(ContentBrowserContextMenu.KeyItem, contentItem);

            _contextMenuRenderer?.BuildAndShow(ContentBrowserContextMenu.ItemId, ctx, s as Control ?? outer);
            e.Handled = true;
        };

        // 悬停
        outer.PointerEntered += (_, _) =>
        {
            if (!_selectedPaths.Contains(path))
                card.Background = BgThumbHover;
            outer.Cursor = new Cursor(StandardCursorType.Hand);
        };
        outer.PointerExited += (_, _) =>
        {
            if (!_selectedPaths.Contains(path))
                card.Background = isDirectory ? BgThumbDir : BgThumbFile;
        };

        return outer;
    }

    /* ======================== 文件信息 ======================== */

    private static (string Icon, string Label, IBrush BadgeColor) GetFileInfo(string extension, string? assetType)
    {
        return extension?.ToLower() switch
        {
            ".png" or ".jpg" or ".jpeg" or ".bmp" or ".tga" or ".hdr"
                => ("🖼️", "TEX", BadgeTexture),
            ".fbx" or ".obj" or ".gltf" or ".glb"
                => ("🧊", "MESH", BadgeDefault),
            ".wav" or ".mp3" or ".ogg"
                => ("🎵", "AUD", BadgeAudio),
            ".mp4" or ".avi" or ".mov"
                => ("🎬", "VID", BadgeAudio),
            ".cs" or ".cpp" or ".h" or ".py"
                => ("📝", "CODE", BadgeScript),
            ".json" or ".xml" or ".yaml" or ".yml"
                => ("📋", "CFG", BadgeDefault),
            ".txt" or ".md"
                => ("📄", "TXT", BadgeDefault),
            ".shader" or ".hlsl" or ".glsl"
                => ("☀️", "SHD", BadgeMaterial),
            ".scene"
                => ("🗺️", "SCENE", BadgeScene),
            ".prefab"
                => ("🧊", "PREFB", BadgePrefab),
            ".mat"
                => ("📋", "MATER", BadgeMaterial),
            ".html"
                => ("🌐", "HTML", BadgeDefault),
            ".lua"
                => ("📜", "LUA", BadgeScript),
            _ => ("📄", "FILE", BadgeDefault)
        };
    }

    /* ======================== 选中状态 ======================== */

    private void OnRubberBandSelectionChanged(IReadOnlyList<Control> selected, bool append)
    {
        if (!append)
            _selectedPaths.Clear();

        foreach (var ctrl in selected)
        {
            if (ctrl.Tag is string path)
                _selectedPaths.Add(path);
        }

        UpdateSelectionVisuals();
    }

    private void UpdateSelectionVisuals()
    {
        foreach (var (path, border) in _thumbnailBorders)
        {
            var isSelected = _selectedPaths.Contains(path);
            border.Background = isSelected ? BgThumbSelected : BgThumbFile;
            border.BorderBrush = isSelected ? BorderSelected : Brushes.Transparent;
            border.BorderThickness = new Thickness(isSelected ? 2 : 0);
        }

        UpdateStatusBar();
    }

    /* ======================== 右键事件 ======================== */

    private void OnFileAreaPointerReleased(object? sender, PointerReleasedEventArgs e)
    {
        if (e.InitialPressMouseButton != MouseButton.Right) return;
        if (!string.IsNullOrEmpty(_selectedItemPath)) return;

        var ctx = ContextMenuManager.Instance;
        ctx.SetContext(ContentBrowserContextMenu.KeyPath, _viewModel?.CurrentDirectory ?? "");
        ctx.SetContext(ContentBrowserContextMenu.KeyContentBrowser, AssetsContentBrowser.Instance);

        _contextMenuRenderer?.BuildAndShow(ContentBrowserContextMenu.BackgroundId, ctx, _fileScroll!);
        e.Handled = true;
    }

    private void OnDirectoryTreePointerReleased(object? sender, PointerReleasedEventArgs e)
    {
        if (e.InitialPressMouseButton != MouseButton.Right) return;

        if (_directoryTree?.SelectedItem is TreeViewItem item && item.Tag is string path)
        {
            _selectedItemPath = path;
            _selectedItemName = (item.Header as StackPanel)?.Children
                .OfType<TextBlock>().LastOrDefault()?.Text ?? "";
            _selectedItemIsDirectory = true;

            var ctx = ContextMenuManager.Instance;
            ctx.SetContext(ContentBrowserContextMenu.KeyPath, path);
            ctx.SetContext(ContentBrowserContextMenu.KeyContentBrowser, AssetsContentBrowser.Instance);

            _contextMenuRenderer?.BuildAndShow(ContentBrowserContextMenu.BackgroundId, ctx, item);
        }

        e.Handled = true;
    }

    /* ======================== ViewModel 变更 ======================== */

    private void OnPropertyChanged(string? propertyName)
    {
        if (propertyName == nameof(ContentBrowserViewModel.CurrentDirectory))
        {
            _selectedPaths.Clear();
            RefreshBreadcrumb();
            RefreshFileList();
            HighlightDirectoryTreeNode(_viewModel?.CurrentDirectory);
        }
        else if (propertyName == nameof(ContentBrowserViewModel.SearchFilter))
        {
            RefreshFileList();
        }
    }
}
