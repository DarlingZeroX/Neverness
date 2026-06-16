# ContentBrowser 鼠标框选计划

> 日期：2026-06-16
> 状态：待审查

---

## 目标

实现类似 Windows 文件浏览器的鼠标框选功能：在空白区域按下左键拖拽，画出矩形选区，框内项目实时高亮选中。松开鼠标后选区消失，选中状态保留。

## 当前状态

已实现：
- 单击选中（单选）
- Ctrl+单击（多选切换）
- 点击空白区域取消选中
- 选中视觉效果（蓝色 `#3399FF`）

未实现：
- 鼠标拖拽框选
- 框选过程中实时更新选中状态
- 框选 + Ctrl 组合

## 设计方案

### 核心架构

```
ScrollViewer (已有)
  └── Canvas (新增：SelectionCanvas，与 WrapPanel 同尺寸)
        ├── Rectangle (选区矩形，拖拽时显示)
        └── WrapPanel (已有：缩略图网格)
```

`SelectionCanvas` 作为容器同时包含 `WrapPanel` 和选区矩形。选区矩形用半透明蓝色填充，类似 Windows 的效果。

### 关键类

| 组件 | 说明 |
|------|------|
| `SelectionCanvas` | 自定义 Canvas，包含选区矩形和 WrapPanel 子项 |
| `RubberBandSelection` | 逻辑类：管理拖拽状态、选区矩形更新、命中测试 |
| 选区矩形 `Border` | 半透明蓝色矩形（`#223399FF` 填充 + `#663399FF` 边框） |

### 交互流程

```
PointerPressed（空白区域）
    ├─ 记录拖拽起点 (_dragStart)
    ├─ 如果未按 Ctrl → 清除现有选中
    └─ 标记 _isDragging = true

PointerMoved（拖拽中）
    ├─ 更新选区矩形位置和大小
    ├─ 命中测试：遍历所有缩略图，检查与选区的交集
    │   ├─ 在选区内 → 标记为待选中
    │   └─ 不在选区内 → 如果未按 Ctrl，标记为未选中
    └─ 更新视觉效果

PointerReleased（松开）
    ├─ 应用最终选中状态
    ├─ 隐藏选区矩形
    └─ _isDragging = false
```

### 命中测试

缩略图是 `StackPanel`，位于 `WrapPanel` 内。需要获取每个缩略图相对于 `SelectionCanvas` 的边界：

```csharp
// 获取缩略图在 Canvas 坐标系中的边界
var thumbBounds = new Rect(
    thumbnail.TranslatePoint(new Point(0, 0), canvas).Value,
    thumbnail.Bounds.Size);

// 检查与选区矩形的交集
var selectionRect = new Rect(_dragStart, _dragCurrent);
if (selectionRect.Intersects(thumbBounds))
    // 在选区内
```

### 滚动处理

拖拽时鼠标可能超出 ScrollViewer 可视区域，需要自动滚动：

```
PointerMoved
    ├─ 如果鼠标在 ScrollViewer 顶部 30px 以内 → 向上滚动
    ├─ 如果鼠标在 ScrollViewer 底部 30px 以内 → 向下滚动
    └─ 滚动时更新选区矩形和命中测试
```

使用 `DispatcherTimer`（间隔 50ms）在拖拽期间持续检查鼠标位置并滚动。

### 选区矩形样式

```csharp
var selectionRect = new Border
{
    Background = new SolidColorBrush(Color.FromArgb(0x22, 0x33, 0x99, 0xFF)), // 浅蓝半透明填充
    BorderBrush = new SolidColorBrush(Color.FromArgb(0x66, 0x33, 0x99, 0xFF)), // 蓝色边框
    BorderThickness = new Thickness(1),
    IsHitTestVisible = false,  // 不拦截鼠标事件
    IsVisible = false,         // 默认隐藏
};
```

### 与现有选中状态的集成

| 操作 | 行为 |
|------|------|
| 空白拖拽框选 | 替换选中（清除旧选中，选中框内项目） |
| Ctrl+拖拽框选 | 追加选中（保留旧选中，追加框内项目） |
| 框选过程中 Ctrl 松开再按下 | 切换模式：替换 ↔ 追加 |
| 框选 + 点击混合 | 框选后 Ctrl+单击可切换单个项目的选中状态 |

### 文件结构

```
ContentBrowser/
├── ContentBrowserAvaloniaView.cs    — 主视图（修改：集成框选）
├── RubberBandSelection.cs           — 框选逻辑类（新增）
└── (现有文件不变)
```

### RubberBandSelection 类设计

```csharp
/// <summary>
/// 鼠标框选管理器——在 WrapPanel 上实现 Windows 风格的拖拽选区。
/// </summary>
public sealed class RubberBandSelection
{
    // 依赖
    private readonly ScrollViewer _scrollViewer;
    private readonly Canvas _canvas;
    private readonly WrapPanel _wrapPanel;
    private readonly Border _selectionRect;

    // 状态
    private bool _isDragging;
    private Point _dragStart;
    private Point _dragCurrent;
    private bool _isCtrlHeld;

    // 自动滚动
    private DispatcherTimer? _autoScrollTimer;
    private const double AutoScrollZone = 30; // 距边缘 30px 触发滚动
    private const double AutoScrollSpeed = 12; // 每次滚动像素

    // 选中回调
    private readonly Action<IReadOnlyList<Control>, bool> _onSelectionChanged;

    /// <summary>
    /// 构造函数。
    /// </summary>
    /// <param name="scrollViewer">文件列表的 ScrollViewer</param>
    /// <param name="canvas">SelectionCanvas（包含 WrapPanel）</param>
    /// <param name="wrapPanel">缩略图 WrapPanel</param>
    /// <param name="onSelectionChanged">选中变更回调 (选中的控件列表, 是否追加模式)</param>
    public RubberBandSelection(
        ScrollViewer scrollViewer,
        Canvas canvas,
        WrapPanel wrapPanel,
        Action<IReadOnlyList<Control>, bool> onSelectionChanged);

    /// <summary>附加事件监听。</summary>
    public void Attach();

    /// <summary>分离事件监听。</summary>
    public void Detach();

    // 内部方法
    private void OnPointerPressed(object? sender, PointerPressedEventArgs e);
    private void OnPointerMoved(object? sender, PointerEventArgs e);
    private void OnPointerReleased(object? sender, PointerReleasedEventArgs e);
    private void UpdateSelectionRect();
    private void PerformHitTest();
    private void StartAutoScroll();
    private void StopAutoScroll();
    private void OnAutoScrollTick(object? sender, EventArgs e);
}
```

### ContentBrowserAvaloniaView 修改

```csharp
// Bind() 中：
// 1. 将 _fileGrid 包装到 Canvas 中
_selectionCanvas = new Canvas { ClipToBounds = true };
_selectionCanvas.Children.Add(_fileGrid);
_fileScroll.Content = _selectionCanvas;

// 2. 创建选区矩形 Border
_selectionBorder = new Border { ... };
_selectionCanvas.Children.Add(_selectionBorder);

// 3. 创建 RubberBandSelection 实例
_rubberBand = new RubberBandSelection(
    _fileScroll, _selectionCanvas, _fileGrid,
    OnRubberBandSelectionChanged);
_rubberBand.Attach();

// 4. 选中回调
void OnRubberBandSelectionChanged(IReadOnlyList<Control> selected, bool append)
{
    if (!append) _selectedPaths.Clear();
    foreach (var ctrl in selected)
    {
        if (ctrl.Tag is string path)
            _selectedPaths.Add(path);
    }
    UpdateSelectionVisuals();
}
```

### 现有代码影响

| 现有代码 | 影响 |
|----------|------|
| `container.PointerPressed`（单击选中） | 需要区分单击和拖拽起始。短按 = 单击选中，长按拖拽 = 框选。用距离阈值（3px）区分 |
| `container.PointerEntered`（悬停高亮） | 框选时禁用悬停高亮，避免视觉冲突 |
| `_fileScroll.PointerPressed`（空白点击取消选中） | 框选起始时也触发取消选中（如果未按 Ctrl），与现有逻辑兼容 |
| `_thumbnailIconAreas`（选中视觉效果字典） | 框选时通过 `UpdateSelectionVisuals()` 更新，复用现有机制 |

### 距离阈值区分单击和拖拽

```csharp
private const double DragThreshold = 3.0; // 像素

// PointerPressed 中：
_dragStart = position;
_isDragging = false; // 不立即标记为拖拽

// PointerMoved 中：
if (!_isDragging)
{
    var delta = position - _dragStart;
    if (Math.Abs(delta.X) > DragThreshold || Math.Abs(delta.Y) > DragThreshold)
    {
        _isDragging = true;
        // 开始显示选区矩形
    }
}
```

这样短按（移动 < 3px）走单击逻辑，长按拖拽走框选逻辑。

---

## 验证

| 场景 | 预期结果 |
|------|----------|
| 空白区域拖拽 | 画出蓝色半透明矩形，框内项目实时高亮 |
| 松开鼠标 | 矩形消失，框内项目保持选中 |
| Ctrl+拖拽 | 保留已有选中，追加框内项目 |
| 拖拽到边缘 | 自动滚动 |
| 拖拽距离 < 3px | 走单击逻辑，不触发框选 |
| 框选过程中鼠标移出窗口 | 自动滚动到边缘，选区继续扩展 |
| 框选 + Ctrl+单击混合 | 框选后可以 Ctrl+单击切换个别项目 |

---

## 未来扩展

- **Shift+单击**：范围选中（从上次选中到当前点击之间的所有项目）
- **键盘框选**：Shift+方向键扩展选区
- **选区预览**：框选时 tooltip 显示 "已选中 N 个项目"
