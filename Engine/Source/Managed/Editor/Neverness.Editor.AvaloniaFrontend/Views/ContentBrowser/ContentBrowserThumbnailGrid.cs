using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Layout;
using Avalonia.Media;
using Neverness.Editor.AvaloniaFrontend.ContentBrowser;
using Neverness.Editor.AvaloniaFrontend.DragDrop;
using Neverness.Editor.Core.Controllers;
using Neverness.Editor.Core.ViewModels;
using Neverness.Editor.AvaloniaFrontend.Views.ContentBrowser.ThumbnailGrid;
using static Neverness.Editor.AvaloniaFrontend.Views.ContentBrowser.ContentBrowserColors;

namespace Neverness.Editor.AvaloniaFrontend.Views.ContentBrowser;

/// <summary>
/// 内容浏览器缩略图网格——右侧文件/文件夹缩略图列表。
/// 职责：容器布局、文件列表刷新、协调子组件。
/// </summary>
internal sealed class ContentBrowserThumbnailGrid
{
    private readonly ContentBrowserViewModel _viewModel;
    private readonly ContentBrowserController _controller;

    // 子组件
    private readonly ThumbnailPresenter _presenter;
    private readonly ThumbnailRenameService _renameService;

    // UI 控件
    private WrapPanel? _fileGrid;
    private ScrollViewer? _fileScroll;
    private Panel? _selectionCanvas;
    private Border? _selectionBorder;
    private RubberBandSelection? _rubberBand;

    // 外部文件拖入相关
    private AvaloniaDropHandler? _dropHandler;
    private Border? _dropHighlight;

    /// <summary>选中的路径集合。</summary>
    internal IReadOnlySet<string> SelectedPaths => _presenter.SelectedPaths;

    /// <summary>当前选中的项信息（用于右键菜单）。</summary>
    internal string? SelectedItemPath => _presenter.SelectedItemPath;
    internal string? SelectedItemName => _presenter.SelectedItemName;
    internal bool SelectedItemIsDirectory => _presenter.SelectedItemIsDirectory;

    /// <summary>右键菜单请求事件（点击缩略图项）。</summary>
    internal event Action<Control, string, string, bool>? OnContextMenuRequested
    {
        add => _presenter.OnContextMenuRequested += value;
        remove => _presenter.OnContextMenuRequested -= value;
    }

    /// <summary>背景右键菜单请求事件（点击空白区域）。</summary>
    internal event Action<Control>? OnBackgroundContextMenuRequested
    {
        add => _presenter.OnBackgroundContextMenuRequested += value;
        remove => _presenter.OnBackgroundContextMenuRequested -= value;
    }

    /// <summary>重命名提交事件（path, newName）。</summary>
    internal event Action<string, string>? OnRenameCommitted
    {
        add => _renameService.OnRenameCommitted += value;
        remove => _renameService.OnRenameCommitted -= value;
    }

    internal ContentBrowserThumbnailGrid(
        ContentBrowserViewModel viewModel,
        ContentBrowserController controller,
        Action<string>? onOpenDirectory,
        Action<string>? onOpenFile,
        Action<int>? onSelectionChanged)
    {
        _viewModel = viewModel;
        _controller = controller;

        // 初始化子组件
        _renameService = new ThumbnailRenameService();
        _presenter = new ThumbnailPresenter(controller, _renameService, onOpenDirectory, onOpenFile, onSelectionChanged);
    }

    /// <summary>创建缩略图区域控件（含左侧内阴影）。</summary>
    internal Control Create()
    {
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

        // 四边内阴影（左侧 + 顶部 + 底部，每层一个 Border）
        var fileShadowLeft = new Border
        {
            Background = Brushes.Transparent,
            IsHitTestVisible = false,
            BoxShadow = new BoxShadows(new BoxShadow
            {
                IsInset = true,
                Color = Color.FromArgb(0x40, 0x00, 0x00, 0x00),
                Blur = 4,
                Spread = -1,
                OffsetX = 3,
                OffsetY = 0,
            }),
        };
        var fileShadowTop = new Border
        {
            Background = Brushes.Transparent,
            IsHitTestVisible = false,
            BoxShadow = new BoxShadows(new BoxShadow
            {
                IsInset = true,
                Color = Color.FromArgb(0x30, 0x00, 0x00, 0x00),
                Blur = 4,
                Spread = -1,
                OffsetX = 0,
                OffsetY = 3,
            }),
        };
        var fileShadowBottom = new Border
        {
            Background = Brushes.Transparent,
            IsHitTestVisible = false,
            BoxShadow = new BoxShadows(new BoxShadow
            {
                IsInset = true,
                Color = Color.FromArgb(0x30, 0x00, 0x00, 0x00),
                Blur = 4,
                Spread = -1,
                OffsetX = 0,
                OffsetY = -3,
            }),
        };

        var fileShadow = new Grid
        {
            [Grid.ColumnProperty] = 2,
            ClipToBounds = true,
            Background = Brushes.Transparent, // 必须有 Background 才能接收拖拽 hit-test
        };
        fileShadow.Children.Add(_fileScroll);
        fileShadow.Children.Add(fileShadowLeft);
        fileShadow.Children.Add(fileShadowTop);
        fileShadow.Children.Add(fileShadowBottom);

        // 框选管理器
        _rubberBand = new RubberBandSelection(
            _fileScroll, _selectionCanvas, _fileGrid, _selectionBorder,
            _presenter.OnRubberBandSelectionChanged);
        _rubberBand.Attach();

        // 事件
        _fileScroll.PointerPressed += (_, _) => _presenter.SetSelectedItem(null, null, false);
        _fileScroll.PointerReleased += (s, e) => _presenter.OnFileAreaPointerReleased(s, e, _fileScroll!);

        // 添加拖拽高亮边框（初始隐藏）
        _dropHighlight = new Border
        {
            Background = Brushes.Transparent,
            BorderBrush = new SolidColorBrush(Color.FromArgb(0x80, 0x21, 0x96, 0xF3)),
            BorderThickness = new Thickness(3),
            CornerRadius = new CornerRadius(4),
            IsHitTestVisible = false,
            IsVisible = false,
            ZIndex = 100,
        };
        _selectionCanvas.Children.Add(_dropHighlight);

        // 初始化外部文件拖拽处理器
        _dropHandler = new AvaloniaDropHandler();
        _dropHandler.Attach(fileShadow);
        _dropHandler.FilesDropped += OnFilesDropped;
        _dropHandler.DragEnter += OnExternalDragEnter;
        _dropHandler.DragLeave += OnExternalDragLeave;

        return fileShadow;
    }

    /// <summary>刷新文件列表。</summary>
    internal void Refresh()
    {
        if (_fileGrid == null || _controller == null) return;

        _fileGrid.Children.Clear();

        var filter = _viewModel?.SearchFilter?.Trim() ?? "";

        try
        {
            // 子目录
            foreach (var dir in _controller.GetSubdirectories())
            {
                if (!string.IsNullOrEmpty(filter) &&
                    !dir.Name.Contains(filter, StringComparison.OrdinalIgnoreCase))
                    continue;

                var thumb = ThumbnailView.Create(dir.Name, "📁", dir.SystemPath.FullPath,
                    isDirectory: true, typeLabel: "DIR", isSelected: _presenter.SelectedPaths.Contains(dir.SystemPath.FullPath));
                _presenter.AttachEvents(thumb, dir.SystemPath.FullPath, dir.Name, isDirectory: true);
                _fileGrid.Children.Add(thumb);
            }

            // 文件
            foreach (var file in _controller.GetFiles())
            {
                if (!string.IsNullOrEmpty(filter) &&
                    !file.Name.Contains(filter, StringComparison.OrdinalIgnoreCase))
                    continue;

                var (icon, label, badgeColor) = ThumbnailFileInfoProvider.GetInfo(file.Extension, file.AssetType);
                var thumb = ThumbnailView.Create(file.Name, icon, file.SystemPath.FullPath,
                    isDirectory: false, typeLabel: label,
                    isSelected: _presenter.SelectedPaths.Contains(file.SystemPath.FullPath),
                    badgeColor: badgeColor);
                _presenter.AttachEvents(thumb, file.SystemPath.FullPath, file.Name, isDirectory: false);
                _fileGrid.Children.Add(thumb);
            }
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"[ContentBrowser] 文件列表加载失败: {ex.Message}");
        }
    }

    /// <summary>清除选中状态。</summary>
    internal void ClearSelection() => _presenter.ClearSelection();

    /// <summary>设置右键菜单选中项信息。</summary>
    internal void SetSelectedItem(string? path, string? name, bool isDirectory)
        => _presenter.SetSelectedItem(path, name, isDirectory);

    /// <summary>开始内联重命名。</summary>
    internal void BeginRename(string path, string currentName)
    {
        // 找到对应的缩略图控件
        if (_fileGrid == null) return;
        foreach (var child in _fileGrid.Children)
        {
            if (child is Control ctrl && ctrl.Tag is string tag && tag == path)
            {
                _renameService.BeginRename(ctrl, path, currentName);
                return;
            }
        }
    }

    /// <summary>释放资源。</summary>
    internal void Dispose()
    {
        _dropHandler?.DetachAll();
        _dropHandler = null;

        _rubberBand?.Detach();
        _rubberBand = null;
    }

    // ── 外部文件拖入处理 ──

    private void OnFilesDropped(string[] files)
    {
        if (_dropHighlight != null)
            _dropHighlight.IsVisible = false;

        var imageExtensions = new HashSet<string>(
            new[] { ".png", ".jpg", ".jpeg", ".tga", ".bmp", ".dds", ".hdr" },
            StringComparer.OrdinalIgnoreCase);

        var imageFiles = files.Where(f => imageExtensions.Contains(Path.GetExtension(f))).ToArray();

        if (imageFiles.Length == 0)
            return;

        _controller.ImportDroppedFiles(imageFiles);
    }

    private void OnExternalDragEnter()
    {
        if (_dropHighlight != null)
            _dropHighlight.IsVisible = true;
    }

    private void OnExternalDragLeave()
    {
        if (_dropHighlight != null)
            _dropHighlight.IsVisible = false;
    }
}
