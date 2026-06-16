using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Layout;
using Avalonia.Media;
using Avalonia.Threading;

namespace Neverness.Editor.AvaloniaFrontend.ContentBrowser;

/// <summary>
/// 鼠标框选管理器——在 WrapPanel 上实现 Windows 风格的拖拽选区。
///
/// 使用方式：
///   1. 创建 Canvas 作为容器，WrapPanel 作为子项
///   2. 创建选区 Border，添加到 Canvas
///   3. new RubberBandSelection(scrollViewer, canvas, wrapPanel, selectionBorder, onSelectionChanged)
///   4. 调用 Attach()
/// </summary>
public sealed class RubberBandSelection
{
    /* ======================== 常量 ======================== */

    /// <summary>拖拽距离阈值——小于此值视为单击，大于视为框选。</summary>
    private const double DragThreshold = 3.0;

    /// <summary>自动滚动触发区域（距边缘像素）。</summary>
    private const double AutoScrollZone = 30.0;

    /// <summary>自动滚动速度（像素/次）。</summary>
    private const double AutoScrollSpeed = 12.0;

    /// <summary>自动滚动间隔（毫秒）。</summary>
    private const int AutoScrollIntervalMs = 30;

    /* ======================== 选区矩形样式 ======================== */

    private static readonly Color SelectionFill = Color.FromArgb(0x22, 0x33, 0x99, 0xFF);
    private static readonly Color SelectionStroke = Color.FromArgb(0x66, 0x33, 0x99, 0xFF);

    /* ======================== 依赖 ======================== */

    private readonly ScrollViewer _scrollViewer;
    private readonly Panel _canvas;
    private readonly WrapPanel _wrapPanel;
    private readonly Border _selectionBorder;

    /// <summary>
    /// 选中变更回调。
    /// 参数 1：选中的控件列表（每个控件的 Tag 是 path）
    /// 参数 2：是否为追加模式（Ctrl）
    /// </summary>
    private readonly Action<IReadOnlyList<Control>, bool> _onSelectionChanged;

    /* ======================== 状态 ======================== */

    private bool _isDragging;
    private bool _isPendingDrag; // PointerPressed 后尚未超过阈值
    private Point _dragStartCanvas; // Canvas 坐标系中的拖拽起点
    private Point _dragCurrentCanvas; // Canvas 坐标系中的当前点
    private bool _isCtrlHeld;

    // 自动滚动
    private DispatcherTimer? _autoScrollTimer;
    private double _lastMouseYInViewer; // 鼠标在 ScrollViewer 中的 Y 坐标

    /* ======================== 构造 ======================== */

    public RubberBandSelection(
        ScrollViewer scrollViewer,
        Panel canvas,
        WrapPanel wrapPanel,
        Border selectionBorder,
        Action<IReadOnlyList<Control>, bool> onSelectionChanged)
    {
        _scrollViewer = scrollViewer ?? throw new ArgumentNullException(nameof(scrollViewer));
        _canvas = canvas ?? throw new ArgumentNullException(nameof(canvas));
        _wrapPanel = wrapPanel ?? throw new ArgumentNullException(nameof(wrapPanel));
        _selectionBorder = selectionBorder ?? throw new ArgumentNullException(nameof(selectionBorder));
        _onSelectionChanged = onSelectionChanged ?? throw new ArgumentNullException(nameof(onSelectionChanged));
    }

    /* ======================== 附加/分离 ======================== */

    /// <summary>附加事件监听。</summary>
    public void Attach()
    {
        _canvas.PointerPressed += OnPointerPressed;
        _canvas.PointerMoved += OnPointerMoved;
        _canvas.PointerReleased += OnPointerReleased;
        _canvas.PointerExited += OnPointerExited;
    }

    /// <summary>分离事件监听。</summary>
    public void Detach()
    {
        _canvas.PointerPressed -= OnPointerPressed;
        _canvas.PointerMoved -= OnPointerMoved;
        _canvas.PointerReleased -= OnPointerReleased;
        _canvas.PointerExited -= OnPointerExited;
        StopAutoScroll();
        HideSelectionRect();
    }

    /* ======================== 事件处理 ======================== */

    private void OnPointerPressed(object? sender, PointerPressedEventArgs e)
    {
        // 只处理左键
        var pt = e.GetCurrentPoint(_canvas);
        if (!pt.Properties.IsLeftButtonPressed) return;

        // 检查是否点击在缩略图上——如果是，不触发框选（由缩略图自己的单击处理）
        var hitControl = FindThumbnailAt(e.GetPosition(_canvas));
        if (hitControl != null) return;

        // 点在空白区域 → 开始潜在拖拽
        _dragStartCanvas = e.GetPosition(_canvas);
        _dragCurrentCanvas = _dragStartCanvas;
        _isCtrlHeld = e.KeyModifiers.HasFlag(KeyModifiers.Control);
        _isPendingDrag = true;
        _isDragging = false;

        // 如果未按 Ctrl，清除现有选中
        if (!_isCtrlHeld)
        {
            _onSelectionChanged(Array.Empty<Control>(), false);
        }

        // 防止事件冒泡到 ScrollViewer 的默认拖拽行为
        e.Handled = true;

        // 记录鼠标在 ScrollViewer 中的 Y 坐标（用于自动滚动）
        _lastMouseYInViewer = e.GetPosition(_scrollViewer).Y;

        // 捕获指针，确保即使鼠标移出也能收到事件
        e.Pointer.Capture(_canvas);
    }

    private void OnPointerMoved(object? sender, PointerEventArgs e)
    {
        if (!_isPendingDrag && !_isDragging) return;

        var currentPos = e.GetPosition(_canvas);
        _isCtrlHeld = e.KeyModifiers.HasFlag(KeyModifiers.Control);
        _lastMouseYInViewer = e.GetPosition(_scrollViewer).Y;

        if (_isPendingDrag)
        {
            // 检查是否超过拖拽阈值
            var delta = currentPos - _dragStartCanvas;
            if (Math.Abs(delta.X) < DragThreshold && Math.Abs(delta.Y) < DragThreshold)
                return;

            // 超过阈值 → 开始框选
            _isPendingDrag = false;
            _isDragging = true;
        }

        _dragCurrentCanvas = currentPos;

        // 更新选区矩形
        UpdateSelectionRect();

        // 命中测试
        PerformHitTest();

        // 自动滚动
        HandleAutoScroll(e.GetPosition(_scrollViewer).Y);
    }

    private void OnPointerReleased(object? sender, PointerReleasedEventArgs e)
    {
        if (!_isDragging && !_isPendingDrag)
        {
            e.Pointer.Capture(null);
            return;
        }

        if (_isDragging)
        {
            // 应用最终选中状态
            PerformHitTest();

            // 隐藏选区矩形
            HideSelectionRect();

            // 停止自动滚动
            StopAutoScroll();
        }

        _isDragging = false;
        _isPendingDrag = false;
        e.Pointer.Capture(null);
    }

    private void OnPointerExited(object? sender, PointerEventArgs e)
    {
        // 鼠标离开 Canvas 时启动自动滚动（如果正在拖拽）
        if (_isDragging)
        {
            StartAutoScroll();
        }
    }

    /* ======================== 选区矩形 ======================== */

    private void UpdateSelectionRect()
    {
        var x = Math.Min(_dragStartCanvas.X, _dragCurrentCanvas.X);
        var y = Math.Min(_dragStartCanvas.Y, _dragCurrentCanvas.Y);
        var w = Math.Abs(_dragCurrentCanvas.X - _dragStartCanvas.X);
        var h = Math.Abs(_dragCurrentCanvas.Y - _dragStartCanvas.Y);

        // 考虑 ScrollViewer 的滚动偏移
        var scrollOffset = _scrollViewer.Offset.Y;
        y -= scrollOffset;

        Canvas.SetLeft(_selectionBorder, x);
        Canvas.SetTop(_selectionBorder, y);
        _selectionBorder.Width = w;
        _selectionBorder.Height = h;
        _selectionBorder.IsVisible = true;
    }

    private void HideSelectionRect()
    {
        _selectionBorder.IsVisible = false;
    }

    /* ======================== 命中测试 ======================== */

    private void PerformHitTest()
    {
        var selectionRect = GetSelectionRectInCanvasCoords();
        var selected = new List<Control>();

        foreach (var child in _wrapPanel.Children)
        {
            if (child is not Control ctrl) continue;

            // 获取控件在 Canvas 坐标系中的边界
            var topLeft = TranslateToCanvas(ctrl);
            if (topLeft == null) continue;

            var bounds = new Rect(topLeft.Value, ctrl.Bounds.Size);

            // 检查交集
            if (selectionRect.Intersects(bounds))
            {
                selected.Add(ctrl);
            }
        }

        _onSelectionChanged(selected, _isCtrlHeld);
    }

    /// <summary>获取选区矩形（Canvas 坐标系，已考虑滚动偏移）。</summary>
    private Rect GetSelectionRectInCanvasCoords()
    {
        var scrollOffset = _scrollViewer.Offset.Y;

        var x = Math.Min(_dragStartCanvas.X, _dragCurrentCanvas.X);
        var y = Math.Min(_dragStartCanvas.Y, _dragCurrentCanvas.Y) + scrollOffset;
        var w = Math.Abs(_dragCurrentCanvas.X - _dragStartCanvas.X);
        var h = Math.Abs(_dragCurrentCanvas.Y - _dragStartCanvas.Y);

        return new Rect(x, y, w, h);
    }

    /// <summary>将控件坐标转换为 Canvas 坐标系。</summary>
    private Point? TranslateToCanvas(Control control)
    {
        var pt = control.TranslatePoint(new Point(0, 0), _canvas);
        return pt;
    }

    /// <summary>查找指定位置的缩略图控件。</summary>
    private Control? FindThumbnailAt(Point canvasPoint)
    {
        foreach (var child in _wrapPanel.Children)
        {
            if (child is not Control ctrl) continue;

            var topLeft = TranslateToCanvas(ctrl);
            if (topLeft == null) continue;

            var bounds = new Rect(topLeft.Value, ctrl.Bounds.Size);
            if (bounds.Contains(canvasPoint))
                return ctrl;
        }
        return null;
    }

    /* ======================== 自动滚动 ======================== */

    private void HandleAutoScroll(double mouseYInViewer)
    {
        var viewerHeight = _scrollViewer.Bounds.Height;
        if (viewerHeight <= 0) return;

        if (mouseYInViewer < AutoScrollZone)
        {
            // 接近顶部 → 向上滚动
            StartAutoScroll();
        }
        else if (mouseYInViewer > viewerHeight - AutoScrollZone)
        {
            // 接近底部 → 向下滚动
            StartAutoScroll();
        }
        else
        {
            StopAutoScroll();
        }
    }

    private void StartAutoScroll()
    {
        if (_autoScrollTimer != null) return;

        _autoScrollTimer = new DispatcherTimer
        {
            Interval = TimeSpan.FromMilliseconds(AutoScrollIntervalMs)
        };
        _autoScrollTimer.Tick += OnAutoScrollTick;
        _autoScrollTimer.Start();
    }

    private void StopAutoScroll()
    {
        if (_autoScrollTimer == null) return;
        _autoScrollTimer.Stop();
        _autoScrollTimer.Tick -= OnAutoScrollTick;
        _autoScrollTimer = null;
    }

    private void OnAutoScrollTick(object? sender, EventArgs e)
    {
        if (!_isDragging)
        {
            StopAutoScroll();
            return;
        }

        var viewerHeight = _scrollViewer.Bounds.Height;
        var mouseY = _lastMouseYInViewer;

        double scrollDelta = 0;
        if (mouseY < AutoScrollZone)
        {
            scrollDelta = -AutoScrollSpeed;
        }
        else if (mouseY > viewerHeight - AutoScrollZone)
        {
            scrollDelta = AutoScrollSpeed;
        }

        if (Math.Abs(scrollDelta) > 0.01)
        {
            var offset = _scrollViewer.Offset;
            _scrollViewer.Offset = new Vector(offset.X, Math.Max(0, offset.Y + scrollDelta));

            // 更新选区矩形（因为滚动偏移变了）
            UpdateSelectionRect();
            PerformHitTest();
        }
    }
}
