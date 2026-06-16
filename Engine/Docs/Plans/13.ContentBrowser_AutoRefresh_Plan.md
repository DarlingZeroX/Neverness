# ContentBrowser 自动刷新计划

> 日期：2026-06-16
> 状态：✅ 已完成

---

## 问题

右键菜单执行 Create Directory / Delete / Rename / Create Asset 后，ContentBrowser UI 不会自动刷新。

**根因**：菜单命令和 UI 刷新是两条完全断开的链路。

```
菜单命令链（断开）：
  ContentBrowserContextMenuContributor
    → ContextMenuManager.GetContext<ContentBrowser>(KeyContentBrowser)
      → ContentBrowser.CreateNewDirectory() / DeleteDirectoryItem() / ...
        → ContentBrowser.RefreshDirectory()  ← 只刷新自己的内存数据
        → ❌ 没有通知 Controller / ViewModel / View

UI 刷新链（正常）：
  ContentBrowserController.RefreshDirectory()
    → IContentBrowserService.Refresh()
      → ContentBrowser.RefreshDirectory()
      → DirectoryChanged 事件
        → Controller.OnDirectoryChanged()
          → ViewModel.UpdateCurrentDirectory()
            → View.OnPropertyChanged("CurrentDirectory")
              → RefreshFileList() + RefreshBreadcrumb()
```

**关键**：`ContentBrowser` 是纯数据+操作类，没有任何事件机制。菜单命令直接调用 `ContentBrowser` 的方法，绕过了 Controller 的 `RefreshDirectory()` 调用。

---

## 方案：ContentBrowser 增加变更通知事件

### 核心思路

在 `ContentBrowser` 上新增 `ContentChanged` 事件，由 **mutating 操作方法**（而非 `RefreshDirectory`）触发。`ContentBrowserService` 监听此事件并转发为 `DirectoryChanged`。Controller → ViewModel → View 的现有刷新链自动生效。

### 设计原则

**RefreshDirectory 是 Query 操作，不应产生副作用事件。**

```
RefreshDirectory = 读取磁盘 = 纯刷新
ContentChanged   = 内容发生变化 = 通知 UI
```

职责分离：数据变更（mutate）和数据刷新（query）是两个不同关注点。

### 改动范围

| # | 文件 | 改动 | 风险 |
|---|------|------|------|
| 1 | `ContentBrowser.cs` | 新增 `event Action? ContentChanged` + `NotifyContentChanged()`，在 mutate 方法中触发 | 低 |
| 2 | `ContentBrowserService.cs` | 构造时订阅 `ContentBrowser.Instance.ContentChanged`，转发为 `DirectoryChanged` | 低 |
| 3 | `AssetCreationMenuContributor.cs` | Create Asset 后补充 `cb.NotifyContentChanged()` | 低 |
| 4 | `ImageDropHandler.cs` | Drop 后补充 `ContentBrowser.Instance?.NotifyContentChanged()` | 低 |
| 5 | `DefaultDropHandler.cs` | Drop 后补充 `ContentBrowser.Instance?.NotifyContentChanged()` | 低 |
| 6 | Controller / ViewModel / View | 不需要修改 | 无 |

### 详细改动

#### Phase 1：ContentBrowser 新增 ContentChanged 事件

**文件**：`Neverness.Editor.Assets/ContentBrowser/ContentBrowser.cs`

```csharp
/// <summary>
/// 内容发生变化时触发（create / delete / rename）。
/// 仅由 mutating 操作触发，RefreshDirectory（Query）不触发。
/// </summary>
public event Action? ContentChanged;

/// <summary>
/// 通知外部调用方触发 ContentChanged（供外部 mutate 操作使用）。
/// </summary>
public void NotifyContentChanged()
{
    ContentChanged?.Invoke();
}
```

在三个 mutate 方法末尾触发：

```csharp
public void CreateNewDirectory(string path)
{
    ...
    RefreshDirectory();
    RefreshDirectoryTreeRoot();
    ContentChanged?.Invoke();       // ← 新增
}

public void DeleteDirectoryItem(ContentItem item)
{
    ...
    RefreshDirectory();
    ContentChanged?.Invoke();       // ← 新增（两个分支各触发一次）
}

public void RenameDirectoryItem(ContentItem item, string name)
{
    ...
    RefreshDirectory();
    RefreshDirectoryTreeRoot();
    ContentChanged?.Invoke();       // ← 新增
}
```

**为什么不在 RefreshDirectory 中触发**：
- `RefreshDirectory` 本质是 Query（读取磁盘），不应产生副作用
- Controller 自己调 `RefreshDirectory()` 时不应收到自己发出的通知，否则形成循环依赖链
- 后续 Asset Registry / Thumbnail Cache / Undo-Redo 都能监听同一事件

**为什么需要 NotifyContentChanged()**：
- C# event 只能在声明类内部 Invoke，外部代码无法直接触发
- 外部 mutate 调用方（AssetCreationMenuContributor、DropHandler）需要通过此方法通知变更

#### Phase 2：ContentBrowserService 监听事件

**文件**：`Neverness.Editor.Assets/ContentBrowser/ContentBrowserService.cs`

```csharp
public ContentBrowserService()
{
    var cb = ContentBrowser.Instance;
    if (cb != null)
    {
        cb.ContentChanged += OnContentChanged;
    }
}

private void OnContentChanged()
{
    DirectoryChanged?.Invoke(CurrentDirectory);
}
```

**效果**：`ContentBrowser.ContentChanged` → `ContentBrowserService.DirectoryChanged` → Controller → ViewModel → View，现有刷新链自动串联。

### 不需要改动的文件

- `ContentBrowserController.cs` — 已订阅 `DirectoryChanged`，无需修改
- `ContentBrowserViewModel.cs` — 已有 `UpdateCurrentDirectory` + `MarkRefreshed`，无需修改
- `ContentBrowserAvaloniaView.cs` — 已响应 `CurrentDirectory` 变更，无需修改
- `ContentBrowserContextMenuContributor.cs` — "Refresh" 菜单项是 Query 操作，不触发事件

### 验证

| 场景 | 预期结果 |
|------|----------|
| 右键 → Create Directory | 目录树 + 文件列表自动刷新 |
| 右键 → Create Material | 文件列表自动刷新 |
| 右键文件 → Remove | 文件列表自动刷新 |
| 右键文件 → Rename | 文件列表自动刷新 |
| 工具栏 → 新建文件夹 | 目录树 + 文件列表自动刷新（原有功能不受影响） |

### 无循环依赖

```
Controller.RefreshDirectory()
    → ContentBrowser.RefreshDirectory()  ← 纯 Query，不触发 ContentChanged
    → 完成

菜单命令（CreateNewDirectory 等）
    → ContentBrowser.RefreshDirectory()  ← 纯 Query
    → ContentChanged?.Invoke()           ← 只在 mutate 之后触发
    → Service.DirectoryChanged
    → Controller.OnDirectoryChanged()
    → ViewModel.Update()
```

Controller 自己调 `RefreshDirectory()` 时不会收到事件，因为事件只在 mutate 方法中触发。

---

## 未来演进方向

后续可扩展为带类型的事件：

```csharp
public enum ContentChangeType
{
    Created,
    Deleted,
    Renamed,
    Modified
}

public record ContentChangedEventArgs(ContentChangeType Type, string Path);

public event Action<ContentChangedEventArgs>? ContentChanged;
```

多个系统可监听同一事件：

```
Create Material
    ↓
ContentChanged(Created)
    ├─ View 刷新
    ├─ Thumbnail 生成
    ├─ AssetRegistry 更新
    └─ UndoStack 记录
```

当前阶段保持 `event Action? ContentChanged`，简单够用。

---

## 执行记录

```
Phase 1: ContentBrowser.cs 新增 ContentChanged 事件 + NotifyContentChanged()，在 mutate 方法中触发  ✅
    │
    v
Phase 2: ContentBrowserService.cs 订阅并转发为 DirectoryChanged  ✅
    │
    v
Phase 3: 外部调用点补充 NotifyContentChanged()  ✅
    │   - AssetCreationMenuContributor.cs（Create Asset）
    │   - ImageDropHandler.cs（Drop Image）
    │   - DefaultDropHandler.cs（Drop File）
    │
    v
验证：Editor.Assets 编译通过，0 错误  ✅
```

---

## 参考文件

- `Engine/Source/Managed/Editor/Neverness.Editor.Assets/ContentBrowser/ContentBrowser.cs`
- `Engine/Source/Managed/Editor/Neverness.Editor.Assets/ContentBrowser/ContentBrowserService.cs`
- `Engine/Source/Managed/Editor/Neverness.Editor.Core/Controllers/ContentBrowserController.cs`
- `Engine/Source/Managed/Editor/Neverness.Editor.Core/ViewModels/ContentBrowserViewModel.cs`
- `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Views/ContentBrowserAvaloniaView.cs`

---

## 变更记录

| 日期 | 版本 | 变更内容 |
|------|------|----------|
| 2026-06-16 | v1.0 | 初始计划 |
| 2026-06-16 | v2.0 | 按设计审查反馈修正：ContentChanged 在 mutate 方法中触发，不在 RefreshDirectory 中触发；新增 NotifyContentChanged() 公共方法；补充外部调用点（AssetCreationMenuContributor、DropHandler） |
