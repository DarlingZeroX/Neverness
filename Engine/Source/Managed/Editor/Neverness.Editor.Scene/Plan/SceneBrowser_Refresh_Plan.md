# SceneBrowser 上下文菜单执行后刷新计划

## 问题

`SceneBrowserContextMenuContributor` 的 Add Entity / Delete 命令直接操作 SceneWorld（`EntityFactory.CreateCamera` / `entity.Destroy()`），但执行后 SceneBrowser TreeView 不更新。

**根因**：实体创建/销毁后没有通知机制，`SceneBrowserController.RefreshTree()` 无人调用。

## 方案

利用已有的 `SceneEventBus` 事件系统，在 SceneWorld 的 CreateEntity/DestroyEntity 中发射事件，Controller 订阅后自动刷新。

---

## 修改文件

### 1. `Runtime/Neverness.Runtime.Scene/Public/SceneWorld.cs`

在 `CreateEntity` 和 `DestroyEntity` 中发射事件：

```csharp
public SceneEntity? CreateEntity(string? displayName = null)
{
    var entity = _scene.CreateEntity(displayName);
    if (entity != null)
        Events.Emit(SceneEvent.OnEntityCreated(entity));
    return new SceneEntity(entity, _scene);
}

public bool DestroyEntity(SceneEntity entity)
{
    ArgumentNullException.ThrowIfNull(entity);
    if (!entity.IsAlive) return false;

    var iEntity = entity.Entity;
    Events.Emit(SceneEvent.OnEntityDestroyed(iEntity));
    iEntity.Destroy();
    return true;
}
```

### 2. `Editor/Neverness.Editor.Core/Controllers/SceneBrowserController.cs`

在构造函数中订阅场景事件：

```csharp
public SceneBrowserController(SceneBrowserViewModel viewModel, ISceneQueryService sceneQueryService)
{
    _viewModel = viewModel;
    _sceneQueryService = sceneQueryService;

    RefreshTree();

    // 订阅场景实体变更事件，自动刷新树
    SubscribeToSceneEvents();
}
```

新增方法：

```csharp
private void SubscribeToSceneEvents()
{
    var world = SceneModule.GetActiveWorld();
    if (world == null) return;

    world.Events.SubscribeAll(evt =>
    {
        if (evt.Type is SceneEventType.EntityCreated
            or SceneEventType.EntityDestroyed)
        {
            // 延迟到下一帧刷新（避免在事件回调中修改 UI 状态）
            ScheduleRefresh();
        }
    });
}

private bool _refreshScheduled;
private void ScheduleRefresh()
{
    if (_refreshScheduled) return;
    _refreshScheduled = true;
    // 由主循环在下一帧调用
    Dispatcher.UIThread.Post(() =>
    {
        _refreshScheduled = false;
        RefreshTree();
    });
}
```

### 3. `Editor/Neverness.Editor.Scene/Private/Panel/SceneBrowserContextMenuContributor.cs`

不需要改。命令本身已经正确执行（CreateCamera / Destroy），事件由 SceneWorld 自动发射。

### 4. `Editor/Neverness.Editor.Scene/Private/PlayMode/PlayModeController.cs`

切换 ActiveWorld 时，Controller 需要重新订阅新世界的事件。在 `SetActiveWorld` 或等效方法中调用 `SubscribeToSceneEvents()`。

---

## 事件流

```
用户右键 → Add Entity Camera
  → EntityFactory.CreateCamera(world)
    → world.CreateEntity(...)
      → Events.Emit(EntityCreated)        ← SceneWorld 发射
        → Controller.ScheduleRefresh()    ← Controller 订阅
          → Dispatcher.UIThread.Post     ← 延迟到下一帧
            → RefreshTree()               ← UI 刷新
```

## 验证

1. 编译 Scene + Core + AvaloniaFrontend
2. 右键空白区域 → Add Entity → Camera
3. TreeView 应自动出现新实体节点
4. 右键实体 → Delete
5. TreeView 应自动移除该节点
