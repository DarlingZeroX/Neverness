using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Neverness.Editor.AvaloniaFrontend.ContentBrowser;
using Neverness.Editor.Core.Controllers;

namespace Neverness.Editor.AvaloniaFrontend.Views.ContentBrowser.ThumbnailGrid;

/// <summary>
/// 缩略图交互控制器——管理缩略图的点击、拖拽、悬停、右键等交互行为。
/// </summary>
internal sealed class ThumbnailPresenter
{
    private readonly ContentBrowserController _controller;
    private readonly ThumbnailRenameService _renameService;
    private readonly Action<string>? _onOpenDirectory;
    private readonly Action<string>? _onOpenFile;
    private readonly Action<int>? _onSelectionChanged;

    // 选中状态
    private readonly HashSet<string> _selectedPaths = new();
    private readonly Dictionary<string, Border> _thumbnailBorders = new();
    private readonly Dictionary<string, bool> _thumbnailIsDirectory = new();

    // 拖拽状态
    private bool _isDragPending;
    private Point _dragStartPos;
    private PointerPressedEventArgs? _dragPressedArgs;
    private const double DragThreshold = 5.0;

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

    /// <summary>右键菜单请求事件（点击缩略图项）。</summary>
    internal event Action<Control, string, string, bool>? OnContextMenuRequested;

    /// <summary>背景右键菜单请求事件（点击空白区域）。</summary>
    internal event Action<Control>? OnBackgroundContextMenuRequested;

    internal ThumbnailPresenter(
        ContentBrowserController controller,
        ThumbnailRenameService renameService,
        Action<string>? onOpenDirectory,
        Action<string>? onOpenFile,
        Action<int>? onSelectionChanged)
    {
        _controller = controller;
        _renameService = renameService;
        _onOpenDirectory = onOpenDirectory;
        _onOpenFile = onOpenFile;
        _onSelectionChanged = onSelectionChanged;
    }

    /// <summary>为缩略图绑定交互事件。</summary>
    internal void AttachEvents(Control thumbnail, string path, string name, bool isDirectory)
    {
        // thumbnail 是 outer Border，其 Child 是 card Border
        if (thumbnail is Border outerBorder && outerBorder.Child is Border cardBorder)
            _thumbnailBorders[path] = cardBorder;
        _thumbnailIsDirectory[path] = isDirectory;

        // 单击选中 + 记录拖拽起始位置
        thumbnail.PointerPressed += (_, e) =>
        {
            var point = e.GetCurrentPoint(thumbnail);
            if (!point.Properties.IsLeftButtonPressed) return;

            // 如果正在重命名，不处理点击
            if (_renameService.IsRenaming) return;

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

            // 记录拖拽起始位置（用于区分单击和拖拽）
            _isDragPending = true;
            _dragStartPos = e.GetPosition(thumbnail);
            _dragPressedArgs = e; // 保存原始事件用于 DoDragDropAsync

            e.Handled = true;
        };

        // 拖拽启动——左键按住移动超过阈值时启动资产拖拽
        thumbnail.PointerMoved += async (_, e) =>
        {
            if (!_isDragPending || _dragPressedArgs == null) return;
            if (!e.GetCurrentPoint(thumbnail).Properties.IsLeftButtonPressed)
            {
                _isDragPending = false;
                _dragPressedArgs = null;
                return;
            }

            var currentPos = e.GetPosition(thumbnail);
            var delta = currentPos - _dragStartPos;
            if (Math.Abs(delta.X) < DragThreshold && Math.Abs(delta.Y) < DragThreshold)
                return;

            // 超过阈值 → 启动拖拽
            _isDragPending = false;

            // 获取虚拟路径（可选）
            string? virtualPath = null;
            try
            {
                var vPath = _controller.GetAssetVirtualPath(path);
                if (!vPath.IsEmpty)
                    virtualPath = vPath.ToString();
            }
            catch
            {
                // 获取虚拟路径失败不影响拖拽
            }

            var transfer = AssetDragFormats.CreateTransfer(path, virtualPath);
            var pressedArgs = _dragPressedArgs;
            _dragPressedArgs = null;
            await Avalonia.Input.DragDrop.DoDragDropAsync(pressedArgs, transfer, DragDropEffects.Copy);
        };

        // 释放——重置拖拽状态
        thumbnail.PointerReleased += (s, e) =>
        {
            _isDragPending = false;
            _dragPressedArgs = null;

            if (e.InitialPressMouseButton != MouseButton.Right) return;

            _selectedItemPath = path;
            _selectedItemName = name;
            _selectedItemIsDirectory = isDirectory;

            // 触发右键菜单（由外部处理）
            OnContextMenuRequested?.Invoke(s as Control ?? thumbnail, path, name, isDirectory);
            e.Handled = true;
        };

        // 双击
        thumbnail.DoubleTapped += (_, _) =>
        {
            _isDragPending = false;
            _dragPressedArgs = null;
            if (isDirectory) _onOpenDirectory?.Invoke(path);
            else _onOpenFile?.Invoke(path);
        };

        // 悬停
        thumbnail.PointerEntered += (_, _) =>
        {
            if (!_selectedPaths.Contains(path))
            {
                var card = _thumbnailBorders[path];
                ThumbnailView.UpdateHoverVisual(card, true, false, isDirectory);
            }
            thumbnail.Cursor = new Cursor(StandardCursorType.Hand);
        };
        thumbnail.PointerExited += (_, _) =>
        {
            if (!_selectedPaths.Contains(path))
            {
                var card = _thumbnailBorders[path];
                ThumbnailView.UpdateHoverVisual(card, false, false, isDirectory);
            }
        };
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

    /// <summary>处理框选变化回调。</summary>
    internal void OnRubberBandSelectionChanged(IReadOnlyList<Control> selected, bool append)
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

    /// <summary>文件区域右键释放——空白区域显示背景菜单。</summary>
    internal void OnFileAreaPointerReleased(object? sender, PointerReleasedEventArgs e, ScrollViewer fileScroll)
    {
        if (e.InitialPressMouseButton != MouseButton.Right) return;
        if (!string.IsNullOrEmpty(_selectedItemPath)) return;

        // 点击空白区域，触发背景菜单
        OnBackgroundContextMenuRequested?.Invoke(fileScroll);
        e.Handled = true;
    }

    /// <summary>批量更新选中状态的视觉效果。</summary>
    private void UpdateSelectionVisuals()
    {
        foreach (var (path, border) in _thumbnailBorders)
        {
            var isSelected = _selectedPaths.Contains(path);
            var isDirectory = _thumbnailIsDirectory.GetValueOrDefault(path);
            ThumbnailView.UpdateSelectionVisual(border, isSelected, isDirectory);
        }

        _onSelectionChanged?.Invoke(_selectedPaths.Count);
    }
}
