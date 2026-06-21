# 自定义标题栏实现文档

## 概述

移除系统原生标题栏，使用自定义工具栏实现窗口拖动、大小调整、最小化、最大化、关闭功能。

## 实现日期

2026-06-20

## 修改文件

1. `Views/MainEditorWindow.axaml` - XAML 布局
2. `Views/MainEditorWindow.axaml.cs` - C# 逻辑

## 核心技术点

### 1. 移除系统标题栏

```xml
<Window WindowDecorations="None" ...>
```

### 2. 布局结构

```
Window (WindowDecorations="None")
└── Grid (3×3, 边框作为 resize 手柄)
    └── DockPanel (Grid.Row="1" Grid.Column="1")
        ├── Grid (菜单栏 + 窗口控制)
        │   ├── Menu (左侧，File/Edit 等菜单)
        │   ├── Border (中间，拖动区域)
        │   └── StackPanel (右侧，最小化/最大化/关闭按钮)
        ├── StackPanel (工具栏)
        ├── DockPanel (状态栏)
        └── DockControl (Dock 布局)
    ├── 8 个 Border (resize 手柄)
```

### 3. 窗口拖动

Avalonia 不会自动处理拖动，需要手动调用 `BeginMoveDrag()`：

```csharp
// 拖动区域（菜单右侧空白区域）
TitleBar.PointerPressed += (_, e) =>
{
    if (e.GetCurrentPoint(this).Properties.IsLeftButtonPressed)
    {
        BeginMoveDrag(e);
    }
};
```

**关键点**：Menu 控件会拦截鼠标事件，所以拖动区域放在 Grid 的中间列（Column="1"），而不是覆盖整个菜单栏。

### 4. 窗口控制按钮

```csharp
// 最小化
MinimizeButton.Click += (_, _) => WindowState = WindowState.Minimized;

// 最大化/还原切换
MaximizeButton.Click += (_, _) =>
{
    WindowState = WindowState == WindowState.Maximized
        ? WindowState.Normal
        : WindowState.Maximized;
};

// 关闭
CloseButton.Click += (_, _) => Close();

// 双击标题栏最大化/还原
TitleBar.DoubleTapped += (_, _) =>
{
    WindowState = WindowState == WindowState.Maximized
        ? WindowState.Normal
        : WindowState.Maximized;
};
```

### 5. Resize 手柄

使用 3×3 Grid 布局，8 个透明 Border 作为 resize 手柄：

```csharp
private void SetupSide(Control ctl, StandardCursorType cursor, WindowEdge edge)
{
    ctl.Cursor = new Cursor(cursor);
    ctl.PointerPressed += (_, e) =>
    {
        if (WindowState == WindowState.Normal)
        {
            BeginResizeDrag(edge, e);
        }
    };
}

// 初始化
SetupSide(TopLeft, StandardCursorType.TopLeftCorner, WindowEdge.NorthWest);
SetupSide(Top, StandardCursorType.TopSide, WindowEdge.North);
// ... 其他 7 个边角
```

**WindowEdge 枚举值**：
- NorthWest, North, NorthEast
- West, East
- SouthWest, South, SouthEast

### 6. 按钮样式

使用 Segoe MDL2 Assets 图标字体：

```xml
<Button Content="&#xE949;" FontFamily="Segoe MDL2 Assets"/>  <!-- 最小化 -->
<Button Content="&#xE739;" FontFamily="Segoe MDL2 Assets"/>  <!-- 最大化 -->
<Button Content="&#xE106;" FontFamily="Segoe MDL2 Assets"/>  <!-- 关闭 -->
```

悬停效果：

```xml
<Style Selector="Button:pointerover">
    <Setter Property="Background" Value="#FF3C3C3C"/>
</Style>
<Style Selector="Button#CloseButton:pointerover">
    <Setter Property="Background" Value="#FFE81123"/>
    <Setter Property="Foreground" Value="White"/>
</Style>
```

## 布局示意

```
┌─────────────────────────────────────────────────────────┐
│ [5px resize]                                            │
├───┬─────────────────────────────────────────────────┬───┤
│   │ [File][Edit][...]     [拖动区域]     [—][□][✕] │   │
│   ├─────────────────────────────────────────────────┤   │
│   │ [▶][⏸][⏹][💾][↩][↪]  (工具栏)               │   │
│   ├─────────────────────────────────────────────────┤   │
│   │                                                 │   │
│   │              Dock 布局区域                       │   │
│   │                                                 │   │
│   ├─────────────────────────────────────────────────┤   │
│   │ [状态栏]                                        │   │
├───┴─────────────────────────────────────────────────┴───┤
│ [5px resize]                                            │
└─────────────────────────────────────────────────────────┘
```

## 注意事项

1. **Menu 拖动问题**：Menu 控件会拦截鼠标事件，不能直接把整个菜单栏设为拖动区域。解决方案是把拖动区域放在 Grid 的中间列。

2. **WindowEdge 命名**：Avalonia 使用方向命名（NorthWest, North, East...），不是 TopLeft, Top, Right...

3. **WindowDecorations**：使用 `WindowDecorations` 而不是 `SystemDecorations`（已过时）。

4. **BeginMoveDrag**：需要手动调用，Avalonia 不会自动处理拖动。

## 参考

- [Avalonia 官方示例 DecoratedWindow](https://github.com/AvaloniaUI/Avalonia/tree/master/samples/ControlCatalog)
- Avalonia WindowEdge 枚举文档
