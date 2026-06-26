# 资产变更监视 & 热重载 — 实施记录

> 基于审查报告 `AssetWatcher_HotReload_Review.md` 的改进方案
> 状态: **已完成**（2026-06-25）

---

## Phase 1: P0 严重问题修复

- [x] **1.1 + 1.3 事件队列 + Tick 主线程处理 + Runtime 通知**

**问题:** `FlushRuntimeReloads()` 只在 `Stop()` 时调用；事件从 Timer 线程直接触发。

**实际改动:** `HotReloadCoordinator` 全面重写。

核心变化：
- 新增 `AssetChangeType` 枚举 + `AssetChangeEvent` record struct
- 新增 `_pendingEvents` 队列，`AssetWatcher` 事件通过 `Enqueue()` 入队（Timer 线程安全）
- 新增 `Tick()` 方法：出队 → `ProcessEvent` 分发 → `FlushRuntimeReloads()`
- `AssetWatcher` 事件改为 lambda 入队（不再直接调用 Handle 方法）
- `FlushRuntimeReloads()` 从 `Tick()` 调用（而非仅 `Stop()`）
- `Stop()` 中也调用 `Tick()` 处理剩余事件

```csharp
// 事件入队（AssetWatcher Timer 线程调用）
_watcher.OnAssetChanged += path => Enqueue(AssetChangeEvent.Changed(path));
_watcher.OnAssetCreated += path => Enqueue(AssetChangeEvent.Created(path));
_watcher.OnAssetDeleted += path => Enqueue(AssetChangeEvent.Deleted(path));
_watcher.OnAssetRenamed += (oldPath, newPath) => Enqueue(AssetChangeEvent.Renamed(oldPath, newPath));

// Tick（主线程每帧调用）
public void Tick()
{
    if (_disposed) return;
    List<AssetChangeEvent> events;
    lock (_lock)
    {
        if (_pendingEvents.Count == 0 && _pendingReloadGuids.Count == 0) return;
        events = new List<AssetChangeEvent>(_pendingEvents);
        _pendingEvents.Clear();
    }
    foreach (var evt in events) ProcessEvent(evt);
    FlushRuntimeReloads();
}
```

**修改文件:** `AssetsModuleImp.cs`

```csharp
public static void Tick()
{
    EditorAssetDatabase.SaveIfDirty();
    s_hotReloadCoordinator?.Tick(); // 新增
}
```

- [x] **1.2 ImportStateCache 共享关系**

**实际改动:** `HotReloadCoordinator` 构造函数注释已文档化共享关系。`HandleAssetRenamed` 中直接访问 `ImportPipeline.StateCache.Remove()`。

---

## Phase 2: P1 功能缺陷修复

- [x] **2.1 删除时通知 Runtime**

**实际改动:** `HandleAssetDeleted` 中获取 GUID 后调用 `AssetHandle.Unload(guid)`。

```csharp
private void HandleAssetDeleted(NPath sourcePath)
{
    var virtualPath = new NVirtualPath(sourcePath.FullPath);
    EditorAssetDatabase.TryGetGuid(virtualPath, out var guid);
    EditorAssetDatabase.DeleteAsset(virtualPath);
    ImportPipeline.StateCache.Remove(sourcePath);

    // 幂等安全，未加载则跳过
    if (!guid.IsZero)
    {
        AssetHandle.Unload(guid);
        EmitDeleted(guid, sourcePath);
    }
}
```

- [x] **2.2 重命名时清理旧缓存 + 移动 .meta**

**实际改动:** `HandleAssetRenamed` 中增加 `MetaFileManager.MoveMeta()` + `StateCache.Remove()`。

```csharp
private void HandleAssetRenamed(NPath oldPath, NPath newPath)
{
    MetaFileManager.MoveMeta(oldPath, newPath);         // 移动 .meta
    ImportPipeline.StateCache.Remove(oldPath);          // 清理旧缓存
    EditorAssetDatabase.MoveAsset(...);                 // 更新映射
    HandleAssetCreated(newPath);                        // 重新导入
}
```

- [x] **2.3 修复 ForceFullScan 事件类型**

**实际改动:** `AssetWatcher.ScanDirectory` 中通过 `GetCachedHash` 区分。

```csharp
if (_stateCache.GetCachedHash(filePath) != null)
    OnAssetChanged?.Invoke(filePath);   // 已有文件变化
else
    OnAssetCreated?.Invoke(filePath);   // 新文件
```

---

## Phase 3: 健壮性增强

- [x] **3.1 FileSystemWatcher 错误恢复**

**实际改动:** `AssetWatcher` 新增 `_errorCount` + `_recoveryScanTimer`。首次错误时启动 Timer（每 30 秒），恢复后自动停止。`Stop()` 清理 Timer。

```csharp
private int _errorCount;
private Timer? _recoveryScanTimer;

private void OnWatcherError(object sender, ErrorEventArgs e)
{
    var count = Interlocked.Increment(ref _errorCount);
    if (count == 1) // 首次错误才启动
    {
        _recoveryScanTimer?.Dispose();
        _recoveryScanTimer = new Timer(RecoveryScanCallback, null,
            TimeSpan.FromSeconds(30), TimeSpan.FromSeconds(30));
    }
}

private void RecoveryScanCallback(object? state)
{
    try
    {
        ForceFullScan();
        Interlocked.Exchange(ref _errorCount, 0); // 重置
        _recoveryScanTimer?.Dispose();             // 停止 Timer
        _recoveryScanTimer = null;
    }
    catch (Exception ex) { /* 日志 */ }
}
```

- [x] **3.2 批量变更合并**

**实际改动:** 通过事件队列天然实现。`Tick()` 一次性处理所有积累事件，复用 `ProcessEvent`。无需单独的 `ProcessBatch` 路径。

- [x] **3.3 热重载事件通知 UI**

**实际改动:** 新增 `EditorEventType.AssetReloaded` + `AssetReloadedEventPayload`。`HotReloadCoordinator` 通过 `EventBus` 属性发送事件。

```csharp
// 新增类型
public readonly record struct AssetReloadedEventPayload(GUID Guid, NPath Path);

// HotReloadCoordinator 新增属性
public IEditorEventBus? EventBus { get; set; }

// 事件发送
private void EmitReloaded(GUID guid, NPath path)
{
    EventBus?.Emit(new EditorEvent(EditorEventType.AssetReloaded,
        new AssetReloadedEventPayload(guid, path)));
}

// AssetsModuleImp 注入
s_hotReloadCoordinator.EventBus = EditorCoreModule.Context.Events;
```

**修改文件:** `IEditorEventBus.cs`

```csharp
public enum EditorEventType
{
    // ... 现有值 ...
    AssetReloaded,  // 新增
}
```

---

## Phase 4: UI 集成

- [x] **4.1 ContentBrowser 自动刷新**

**实际改动:** `ContentBrowserService` 构造函数接受 `IEditorEventBus?`，订阅 `AssetReloaded` 事件。

```csharp
public ContentBrowserService(IEditorEventBus? eventBus = null)
{
    // ... 现有 ContentChanged 订阅 ...

    eventBus?.Subscribe(EditorEventType.AssetReloaded, _ =>
    {
        Refresh();
    });
}
```

**修改文件:** `AssetsModuleImp.cs` 注入 EventBus

```csharp
EditorCoreModule.Context.RegisterService<IContentBrowserService>(
    new ContentBrowserService(EditorCoreModule.Context.Events));
```

- [ ] **4.2 Inspector 通知** — 未实施，事件基础设施已就绪
- [ ] **4.3 热重载状态栏** — 未实施，事件基础设施已就绪

---

## 实际影响的文件清单

| 文件 | 改动类型 | 行数变化 |
|------|---------|---------|
| `HotReloadCoordinator.cs` | 全面重写 | ~250 行（原 243 行） |
| `AssetWatcher.cs` | 增量修改 | +40 行 |
| `AssetsModuleImp.cs` | 增量修改 | +3 行 |
| `ContentBrowserService.cs` | 增量修改 | +7 行 |
| `IEditorEventBus.cs` | 增量修改 | +1 行 |

**新增类型（在 HotReloadCoordinator.cs 中）:**
- `AssetChangeType` enum — 4 值
- `AssetChangeEvent` record struct — 事件数据
- `AssetReloadedEventPayload` record struct — 事件载荷

编译验证: `Neverness.Editor.Core` + `Neverness.Editor.Assets` 均 0 错误。
