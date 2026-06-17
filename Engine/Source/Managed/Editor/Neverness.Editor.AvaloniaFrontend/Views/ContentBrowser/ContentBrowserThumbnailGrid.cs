using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Layout;
using Avalonia.Media;
using Neverness.Editor.AvaloniaFrontend.ContentBrowser;
using Neverness.Editor.Core.Controllers;
using Neverness.Editor.Core.ViewModels;
using Neverness.Runtime.Assets;
using static Neverness.Editor.AvaloniaFrontend.Views.ContentBrowser.ContentBrowserColors;

namespace Neverness.Editor.AvaloniaFrontend.Views.ContentBrowser;

/// <summary>
/// 内容浏览器缩略图网格——右侧文件/文件夹缩略图列表。
/// </summary>
internal sealed class ContentBrowserThumbnailGrid
{
    private readonly ContentBrowserViewModel _viewModel;
    private readonly ContentBrowserController _controller;
    private readonly Action<string>? _onOpenDirectory;
    private readonly Action<string>? _onOpenFile;
    private readonly Action<int>? _onSelectionChanged;

    private WrapPanel? _fileGrid;
    private ScrollViewer? _fileScroll;
    private Panel? _selectionCanvas;
    private Border? _selectionBorder;
    private RubberBandSelection? _rubberBand;

    // 选中状态
    private readonly HashSet<string> _selectedPaths = new();
    private readonly Dictionary<string, Border> _thumbnailBorders = new();
    private readonly Dictionary<string, TextBlock> _nameLabels = new();

    // 右键菜单状态
    private string? _selectedItemPath;
    private string? _selectedItemName;
    private bool _selectedItemIsDirectory;

    /// <summary>选中的路径集合。</summary>
    internal IReadOnlySet<string> SelectedPaths => _selectedPaths;

    /// <summary>当前选中的项信息（用于右键菜单）。</summary>
    internal string? SelectedItemPath => _selectedItemPath;
    internal string? SelectedItemName => _selectedItemName;
    internal bool SelectedItemIsDirectory => _selectedItemIsDirectory;

    internal ContentBrowserThumbnailGrid(
        ContentBrowserViewModel viewModel,
        ContentBrowserController controller,
        Action<string>? onOpenDirectory,
        Action<string>? onOpenFile,
        Action<int>? onSelectionChanged)
    {
        _viewModel = viewModel;
        _controller = controller;
        _onOpenDirectory = onOpenDirectory;
        _onOpenFile = onOpenFile;
        _onSelectionChanged = onSelectionChanged;
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

        var fileShadow = new Grid { [Grid.ColumnProperty] = 2, ClipToBounds = true };
        fileShadow.Children.Add(_fileScroll);
        fileShadow.Children.Add(fileShadowLeft);
        fileShadow.Children.Add(fileShadowTop);
        fileShadow.Children.Add(fileShadowBottom);

        // 框选管理器
        _rubberBand = new RubberBandSelection(
            _fileScroll, _selectionCanvas, _fileGrid, _selectionBorder,
            OnRubberBandSelectionChanged);
        _rubberBand.Attach();

        // 事件
        _fileScroll.PointerPressed += (_, _) => { _selectedItemPath = null; };
        _fileScroll.PointerReleased += OnFileAreaPointerReleased;

        return fileShadow;
    }

    /// <summary>刷新文件列表。</summary>
    internal void Refresh()
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

        _onSelectionChanged?.Invoke(_selectedPaths.Count);
    }

    /// <summary>清除选中状态。</summary>
    internal void ClearSelection()
    {
        _selectedPaths.Clear();
        UpdateSelectionVisuals();
    }

    /// <summary>设置右键菜单选中项信息。</summary>
    internal void SetSelectedItem(string? path, string? name, bool isDirectory)
    {
        _selectedItemPath = path;
        _selectedItemName = name;
        _selectedItemIsDirectory = isDirectory;
    }

    /// <summary>释放资源。</summary>
    internal void Dispose()
    {
        _rubberBand?.Detach();
        _rubberBand = null;
    }

    /* ======================== 缩略图创建 ======================== */

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
                Color = Color.FromArgb(0xA0, 0x00, 0x00, 0x00),
                Blur = 12,
                OffsetX = 0,
                OffsetY = 4,
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
        var nameLabel = new TextBlock
        {
            Text = name,
            FontSize = 11,
            Foreground = isSelected ? TextBright : TextPrimary,
            TextTrimming = TextTrimming.CharacterEllipsis,
            MaxLines = 1,
            Height = ThumbNameHeight,
            VerticalAlignment = VerticalAlignment.Center,
        };
        textArea.Children.Add(nameLabel);
        _nameLabels[path] = nameLabel;

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
            if (isDirectory) _onOpenDirectory?.Invoke(path);
            else _onOpenFile?.Invoke(path);
        };

        // 右键
        outer.PointerReleased += (s, e) =>
        {
            if (e.InitialPressMouseButton != MouseButton.Right) return;

            _selectedItemPath = path;
            _selectedItemName = name;
            _selectedItemIsDirectory = isDirectory;

            // 触发右键菜单（由外部处理）
            OnContextMenuRequested?.Invoke(s as Control ?? outer, path, name, isDirectory);
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

    /// <summary>右键菜单请求事件（点击缩略图项）。</summary>
    internal event Action<Control, string, string, bool>? OnContextMenuRequested;

    /// <summary>背景右键菜单请求事件（点击空白区域）。</summary>
    internal event Action<Control>? OnBackgroundContextMenuRequested;

    /// <summary>文件区域右键释放——空白区域显示背景菜单。</summary>
    private void OnFileAreaPointerReleased(object? sender, PointerReleasedEventArgs e)
    {
        if (e.InitialPressMouseButton != MouseButton.Right) return;
        if (!string.IsNullOrEmpty(_selectedItemPath)) return;

        // 点击空白区域，触发背景菜单
        OnBackgroundContextMenuRequested?.Invoke(_fileScroll!);
        e.Handled = true;
    }

    /* ======================== 内联重命名 ======================== */

    /// <summary>重命名提交事件（path, newName）。</summary>
    internal event Action<string, string>? OnRenameCommitted;

    /// <summary>开始内联重命名——将文件名 TextBlock 替换为 TextBox。</summary>
    internal void BeginRename(string path, string currentName)
    {
        if (!_nameLabels.TryGetValue(path, out var nameLabel)) return;

        // 找到 nameLabel 的父容器（StackPanel）
        if (nameLabel.Parent is not StackPanel textArea) return;

        // 隐藏原文件名
        nameLabel.IsVisible = false;

        // 创建内联编辑框
        var textBox = new TextBox
        {
            Text = currentName,
            FontSize = 11,
            MinWidth = 60,
            Height = ThumbNameHeight,
            VerticalAlignment = VerticalAlignment.Center,
            Margin = new Thickness(-2, 0, -2, 0),
            Padding = new Thickness(2, 0),
            Background = new SolidColorBrush(Color.Parse("#FF1E1E1E")),
            Foreground = TextBright,
            BorderBrush = new SolidColorBrush(Color.Parse("#FF007ACC")),
            BorderThickness = new Thickness(1),
        };

        // 选中文件名（不含扩展名）
        var dotIndex = currentName.LastIndexOf('.');
        textBox.SelectionStart = 0;
        textBox.SelectionEnd = dotIndex > 0 ? dotIndex : currentName.Length;

        // 提交重命名
        void CommitRename()
        {
            var newName = textBox.Text?.Trim() ?? "";
            if (!string.IsNullOrEmpty(newName) && newName != currentName)
            {
                OnRenameCommitted?.Invoke(path, newName);
            }
            CancelRename(path, nameLabel, textBox);
        }

        // 取消重命名
        void CancelRenameHandler()
        {
            CancelRename(path, nameLabel, textBox);
        }

        textBox.KeyDown += (_, e) =>
        {
            if (e.Key == Key.Enter)
            {
                CommitRename();
                e.Handled = true;
            }
            else if (e.Key == Key.Escape)
            {
                CancelRenameHandler();
                e.Handled = true;
            }
        };

        // 失焦也提交
        textBox.LostFocus += (_, _) => CommitRename();

        textArea.Children.Add(textBox);

        // 聚焦并延迟选中（等控件加载完）
        textBox.AttachedToVisualTree += (_, _) =>
        {
            textBox.Focus();
            textBox.SelectAll();
        };
    }

    /// <summary>取消重命名——移除 TextBox，恢复 TextBlock。</summary>
    private static void CancelRename(string path, TextBlock nameLabel, TextBox textBox)
    {
        if (textBox.Parent is StackPanel textArea)
            textArea.Children.Remove(textBox);
        nameLabel.IsVisible = true;
    }

    /* ======================== 框选回调 ======================== */

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

        _onSelectionChanged?.Invoke(_selectedPaths.Count);
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
}
