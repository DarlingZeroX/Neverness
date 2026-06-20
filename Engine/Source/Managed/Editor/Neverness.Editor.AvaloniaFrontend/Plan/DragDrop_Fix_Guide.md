# Avalonia DragDrop 修复指南

## 问题描述

从 Windows 资源管理器拖拽文件到 Content Browser 时，没有任何事件触发，或者触发后出现线程访问异常。

---

## 问题一：DragDrop 事件完全不触发

### 现象

- 拖拽文件到窗口上，`DragEnter`、`DragOver`、`Drop` 事件均不触发
- 控制台无任何输出

### 根本原因

**Avalonia UI 线程没有设置 STA（Single-Threaded Apartment）模式。**

Windows 的 OLE DragDrop 使用 COM 协议，要求调用线程必须是 STA 模式。如果线程以 MTA 模式运行，所有 COM 相关功能（拖放、剪贴板、文件对话框）都不会工作。

### 调用链分析

```
NevernessEditor.Program.Main()                    // 无 [STAThread]
  → EditorApplicationRunner.Run()
    → InstallAvaloniaFrontend()
      → AvaloniaEditorHost.Start()
        → new Thread("AvaloniaUIThread")           // ← 没有 SetApartmentState(STA)!
          → Program.RunAvaloniaApp()
```

### 修复方案

在 `AvaloniaEditorHost.cs` 中，创建线程后、启动前，设置 STA：

```csharp
_avaloniaThread = new Thread(() =>
{
    try
    {
        _isRunning = true;
        _startupEvent.Set();
        Program.RunAvaloniaApp();
    }
    catch (Exception ex)
    {
        Console.Error.WriteLine($"[AvaloniaEditorHost] Avalonia 应用异常: {ex}");
    }
    finally
    {
        _isRunning = false;
    }
})
{
    Name = "AvaloniaUIThread",
    IsBackground = true
};

// ✅ 关键修复：设置 STA 模式
_avaloniaThread.SetApartmentState(ApartmentState.STA);
_avaloniaThread.Start();
```

### 修改文件

- `Neverness.Editor.AvaloniaFrontend/Public/AvaloniaEditorHost.cs`

---

## 问题二：拖放后线程访问异常

### 现象

- 拖放事件触发了
- 但抛出异常：`System.InvalidOperationException: "The calling thread cannot access this object because a different thread owns it."`

### 根本原因

**后台线程直接操作了 UI 对象。**

调用链：

```
拖放事件 (Avalonia UI 线程 STA)
  → OnFilesDropped()
    → _controller.ImportDroppedFiles()
      → Task.Run(() => {                           // ← 切换到后台线程
          _dropImportService.ImportFiles()
            → ImageDropHandler.Handle()
              → ContentBrowser.Instance.RefreshDirectory()   // ← UI 操作！
              → ContentBrowser.Instance.NotifyContentChanged() // ← UI 操作！
                → ContentChanged → DirectoryChanged
                  → UI 订阅者在后台线程被调用 → 💥
        })
```

### 修复方案

**三处修改：**

#### 1. ImageDropHandler.Handle() — 移除直接 UI 调用

Assets 模块不应该直接操作 UI，UI 刷新由调用方负责。

```csharp
// ❌ 之前：在后台线程直接调用 UI 操作
ContentBrowser.Instance?.RefreshDirectory();
ContentBrowser.Instance?.NotifyContentChanged();

// ✅ 现在：只做数据操作
EditorAssetDatabase.MarkDirty(meta.Guid);
return true;
```

#### 2. ContentBrowserController.ImportDroppedFiles() — 使用 SynchronizationContext

在构造时捕获 UI 线程上下文，Task.Run 完成后切回 UI 线程。

```csharp
// 构造时捕获
private readonly SynchronizationContext? _uiSyncContext;

public ContentBrowserController(...)
{
    _uiSyncContext = SynchronizationContext.Current;
    // ...
}

// Task.Run 完成后切回 UI 线程
Task.Run(() =>
{
    var (successCount, failCount) = _dropImportService.ImportFiles(filePaths, currentDir);

    void UiAction()
    {
        ShowToast(...);
        RefreshDirectory();
    }

    if (_uiSyncContext != null)
        _uiSyncContext.Post(_ => UiAction(), null);
    else
        UiAction();
});
```

> 注意：Core 模块不依赖 Avalonia，所以用 `SynchronizationContext` 而不是 `Dispatcher.UIThread`。

#### 3. ContentBrowserInteractions.OnPropertyChanged() — UI 线程保护

ViewModel 的 `PropertyChanged` 事件可能在后台线程触发（由 `RefreshDirectory` → `UpdateViewModel` 链式调用），需要切到 UI 线程。

```csharp
// ❌ 之前：直接执行 UI 操作
private void OnPropertyChanged(string? propertyName)
{
    if (propertyName == nameof(ContentBrowserViewModel.CurrentDirectory))
    {
        _thumbnailGrid.ClearSelection();        // 💥 后台线程调用
        _toolbar.RefreshBreadcrumb();
        _thumbnailGrid.Refresh();
        // ...
    }
}

// ✅ 现在：用 Dispatcher.UIThread.Invoke 包裹
private void OnPropertyChanged(string? propertyName)
{
    Dispatcher.UIThread.Invoke(() =>
    {
        if (propertyName == nameof(ContentBrowserViewModel.CurrentDirectory))
        {
            _thumbnailGrid.ClearSelection();
            _toolbar.RefreshBreadcrumb();
            _thumbnailGrid.Refresh();
            // ...
        }
    });
}
```

### 修改文件

- `Neverness.Editor.Assets/Import/Importers/ImageDropHandler.cs`
- `Neverness.Editor.Core/Controllers/ContentBrowserController.cs`
- `Neverness.Editor.AvaloniaFrontend/Views/ContentBrowser/ContentBrowserInteractions.cs`

---

## 辅助修改（调试过程中添加）

以下修改是为了更好的拖放体验，非必需但推荐保留：

### 1. AvaloniaDropHandler — 使用 AddHandler 注册事件

```csharp
// 使用 AddHandler + RoutingStrategies.Bubble
host.AddHandler(Avalonia.Input.DragDrop.DragEnterEvent, OnDragEnter, RoutingStrategies.Bubble);
host.AddHandler(Avalonia.Input.DragDrop.DragOverEvent, OnDragOver, RoutingStrategies.Bubble);
host.AddHandler(Avalonia.Input.DragDrop.DragLeaveEvent, OnDragLeave, RoutingStrategies.Bubble);
host.AddHandler(Avalonia.Input.DragDrop.DropEvent, OnDrop, RoutingStrategies.Bubble);
```

### 2. ContentBrowserThumbnailGrid — fileShadow 添加 Background

控件必须有 Background 才能接收拖拽 hit-test：

```csharp
var fileShadow = new Grid
{
    [Grid.ColumnProperty] = 2,
    ClipToBounds = true,
    Background = Brushes.Transparent, // ← 必须有 Background
};
```

---

## 经验总结

| 问题 | 原因 | 解决方案 |
|------|------|----------|
| DragDrop 事件不触发 | 线程不是 STA 模式 | `SetApartmentState(ApartmentState.STA)` |
| 后台线程操作 UI | Task.Run 中直接调用 UI 方法 | `SynchronizationContext.Post()` 或 `Dispatcher.UIThread.Invoke()` |
| PropertyChanged 后台触发 | ViewModel 更新在后台线程 | 订阅者内部切 UI 线程 |
| 控件无法 hit-test | 没有 Background | 设置 `Background = Brushes.Transparent` |

### 线程安全原则

1. **UI 控件只能由创建它的线程访问**（Avalonia STA 线程）
2. **Task.Run 内部不能直接操作 UI**，必须切回 UI 线程
3. **ViewModel PropertyChanged 可能在任意线程触发**，订阅者需要自己保证线程安全
4. **Core 模块不依赖 Avalonia**，用 `SynchronizationContext` 而非 `Dispatcher`

---

## 相关文件

- `AvaloniaEditorHost.cs` — Avalonia 线程启动
- `AvaloniaDropHandler.cs` — 拖放事件处理
- `ContentBrowserThumbnailGrid.cs` — 缩略图网格 UI
- `ContentBrowserInteractions.cs` — 交互逻辑
- `ContentBrowserController.cs` — 控制器
- `ImageDropHandler.cs` — 图片导入处理器
- `ContentBrowserService.cs` — 内容浏览器服务
