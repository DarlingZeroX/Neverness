# Neverness.Runtime.Scene — 工业级 Gameplay Runtime Framework

> 程序集：`Neverness.Runtime.Scene`　|　命名空间：`Neverness.Managed.Scene`　|　目标框架：net10.0

## 1. 定位与边界

| 项目 | 说明 |
|------|------|
| **职责** | C# 侧 Gameplay Runtime Root：**SceneWorld**（世界管理）、**EntityRegistry**（实体映射）、**零 GC Query**（SceneView/SceneQuery）、**System 调度**（Kahn 拓扑排序 + TickGroup）、**Prefab 系统**（PrefabAsset/Instantiator/Override）、**事件总线**（SceneEventBus）、**热重载**（HotReloadSnapshot）。Native ECS 数据不复制到 C#，所有操作经 ABI 转发。 |
| **不负责** | Legacy `IGameActor`/`IComponent`、**NNRuntimeScene** Native 实现（C++ entt::registry）、**SceneSubsystem** 替换或数据迁移、**Editor** UI 层（由 `Neverness.Editor.Framework` 消费）、并行 Job 调度。 |
| **依赖** | `Neverness.Runtime.Engine`（`NNEntityHandle` / `NNSceneResult` / `NNTransformData` / `NNCameraComponentData` / `ComponentIdAttribute`）；`Neverness.Runtime.Interop`（`EngineNativeApiBootstrap`）。**不**引用任何 Editor 模块。 |

### 1.1 与 Native 边界

```
C# SceneWorld                        Native NNRuntimeScene
┌──────────────┐                ┌──────────────────────┐
│ NativeHandle │───────────────│ m_Registry (entt)     │
│              │  C ABI 调用    │ m_EntityTable         │
│ Entities ────│──────────────│ m_ComponentRegistry   │
│ Systems ────│──────────────│ m_SystemScheduler     │
│ Events ─────│──────────────│ m_EventBus            │
│ Queries ────│──────────────│ m_DirtyTracker        │
└──────────────┘                └──────────────────────┘
```

- SceneWorld 不复制 Native 数据，每次读写经 `SceneNativeBridge` ABI 转发
- `ComponentTypeCache<T>.TypeId`（FNV-1a name hash）与 Native `NNComponentRegistry` 跨进程对齐
- Managed System 和 Native System 在各自的 Tick 阶段独立调度

### 1.2 与既有身份体系对照

| 体系 | 身份 | 关系 |
|------|------|------|
| **NNEngineLegacy** | `VGActorID` + `shared_ptr<IGameActor>` + entt | **并存**，不修改 |
| **NNRuntimeEngine** | `NNEntityHandle` 单调 id + `SceneSubsystem` map | **并存**；ECS 路径为 `EcsScene()`，不替换 `SceneSubsystem` |
| **本模块** | `SceneWorld` + `EntityRegistry` + Managed ECS | **已落地** Phase 4-D–4-I |
| **VisionGal.Managed.Entity** | `EntityHandle`（`uint` Index + `uint` Generation） | **打包规则对齐**；**无**自动桥接 |

---

## 2. 目录结构

```
Neverness.Runtime.Scene/
├── Neverness.Runtime.Scene.csproj
│
├── Public/                                        # namespace Neverness.Runtime.Scene
│   ├── SceneWorld.cs                              # 场景世界根（Gameplay Runtime Root）
│   ├── SceneManager.cs                            # 场景管理器（降级为世界管理器）
│   ├── SceneEntity.cs                             # 实体句柄门面
│   ├── TickGroup.cs                               # Tick 分组枚举（对齐 Native NNSceneTickGroup）
│   ├── HotReloadSnapshot.cs                       # 热重载快照（HotReloadSnapshot + GlobalHotReloadSnapshot）
│   ├── Prefab.cs                                  # Prefab 便捷包装器（保留 builder API）
│   │
│   ├── Entities/
│   │   ├── EntityRegistry.cs                      # 实体注册表（handle↔entity 双向映射）
│   │   └── EntityFactory.cs                       # 实体工厂（CreateCamera 等快捷创建）
│   │
│   ├── Queries/
│   │   ├── SceneView.cs                           # ref struct 单组件零 GC 视图
│   │   ├── SceneView2.cs                          # ref struct 双组件零 GC 视图
│   │   ├── SceneQuery.cs                          # 缓存式查询对象
│   │   └── SceneQueryCache.cs                     # 查询对象池
│   │
│   ├── Systems/
│   │   ├── ISceneSystem.cs                        # System 标记接口
│   │   ├── ISystemInitialize.cs                   # void Initialize(SceneWorld)
│   │   ├── ISystemShutdown.cs                     # void Shutdown(SceneWorld)
│   │   ├── ISystemTick.cs                         # TickGroup + void Tick(SceneWorld, float)
│   │   ├── ISystemFixedTick.cs                    # TickGroup + void FixedTick(SceneWorld, float)
│   │   ├── ISystemLateTick.cs                     # TickGroup + void LateTick(SceneWorld, float)
│   │   ├── SystemDependencyAttribute.cs           # [SystemDependency(typeof(X))] 依赖声明
│   │   └── SceneSystemScheduler.cs                # Kahn 拓扑排序 + TickGroup 分组
│   │
│   ├── Prefabs/
│   │   ├── PrefabAsset.cs                         # Prefab 资产（Guid/Name/Entities + FromEntity BFS）
│   │   ├── PrefabEntityData.cs                    # 实体数据容器（LocalIndex/ParentIndex/Components）
│   │   ├── PrefabInstance.cs                      # Prefab 实例（Source/RootEntity/InstanceMap/Overrides）
│   │   ├── PrefabOverride.cs                      # 覆盖类型枚举 + 覆盖记录
│   │   └── PrefabInstantiator.cs                  # 实例化器（Instantiate/ApplyOverrides/RevertToPrefab）
│   │
│   └── Events/
│       ├── SceneEvent.cs                          # SceneEventType 枚举 + SceneEvent 结构体
│       ├── SceneEventBus.cs                       # C# 事件总线（Subscribe/Emit/FlushDeferred）
│       └── NativeEventBridge.cs                   # Native EventBus → C# 桥接（stub）
│
├── Private/                                       # namespace Neverness.Runtime.Scene.Internal
│   ├── SceneNativeBridge.cs                       # NNSceneApi ABI 薄封装（不对外暴露）
│   ├── ComponentTypeCache.cs                      # 泛型组件类型缓存（[ComponentId] → TypeId）
│   └── NativeQueryBridge.cs                       # Native 批量查询 ABI 封装
│
├── Docs/
└── obj/ / Build/
```

---

## 3. 核心 API

### 3.1 SceneWorld — 场景世界

```csharp
public sealed class SceneWorld : IDisposable
{
    ulong NativeHandle { get; }            // Native 场景句柄
    string Name { get; set; }              // 场景名称
    NNGuid AssetGuid { get; set; }         // 场景资产 GUID

    EntityRegistry Entities { get; }       // 实体注册表
    SceneQueryCache Queries { get; }       // 查询缓存
    SceneSystemScheduler Systems { get; }  // System 调度器
    SceneEventBus Events { get; }          // 事件总线

    // 工厂
    static SceneWorld? Create(string name);
    static SceneWorld? LoadFromAsset(string name, string vfsPath);

    // 实体操作（委托 EntityRegistry）
    SceneEntity? CreateEntity(string? displayName = null);
    NNSceneResult DestroyEntity(SceneEntity entity);

    // Tick 流
    void Tick(float deltaTime);            // EarlyUpdate → Update → Native → LateUpdate → Render → FlushDeferred
    void FixedTick(float fixedDeltaTime);  // 固定步长 Tick

    // 查询
    SceneQuery<T> GetQuery<T>() where T : unmanaged;
    SceneQuery<T1, T2> GetQuery<T1, T2>() where T1 : unmanaged where T2 : unmanaged;

    // 序列化
    NNSceneResult Save(string vfsPath);

    // 热重载
    HotReloadSnapshot SaveSnapshot();
    static SceneWorld RestoreFromSnapshot(HotReloadSnapshot snapshot);
    void RebuildAfterReload();

    void Dispose();
}
```

### 3.2 EntityRegistry — 实体注册表

| 方法 | 说明 |
|------|------|
| `Create(displayName?)` | 创建实体并注册 |
| `Destroy(entity)` | 销毁实体并解除注册 |
| `Register(entity)` | 注册已有实体（如 Prefab 实例化后的实体） |
| `Unregister(entity)` | 解除注册（不销毁 Native 实体） |
| `Find(handle)` | 按句柄查找（O(1)，返回 null） |
| `Get(handle)` | 按句柄获取（O(1)，找不到抛异常） |
| `TryGet(handle, out entity)` | 尝试获取 |
| `Contains(entity)` | 是否包含 |
| `SyncFromHandles(handles)` | 从句柄列表重建映射（热重载用） |
| `ExportHandleValues()` | 导出所有句柄值（快照保存用） |

### 3.3 Query 系统 — 零 GC 批量遍历

```csharp
// 获取缓存查询
var query = world.GetQuery<NNTransformData>();

// 零 GC 遍历（ref struct enumerator）
foreach (ref var entry in query.Execute())
{
    ref var transform = ref entry.Component;
    transform.Position.X += 10.0f;
}

// 双组件查询
var query2 = world.GetQuery<NNTransformData, NNTagComponent>();
foreach (ref var entry in query2.Execute())
{
    ref var t = ref entry.Component1;
    ref var tag = ref entry.Component2;
}
```

核心类型：
- `SceneView<T>` / `SceneView<T1,T2>`：ref struct，零 GC，`MemoryMarshal.AsRef<T>` 直接引用 Span 数据
- `SceneQuery<T>`：缓存式查询，管理 handle/component 数据缓冲区，懒增长
- `NativeQueryBridge`：批量查询 ABI 封装（QueryEntities / QueryComponents / QueryCount2）

### 3.4 System 框架 — Kahn 拓扑排序 + TickGroup

```csharp
// 定义 System
[SystemDependency(typeof(MovementSystem))]
public class MySystem : ISystemTick
{
    public TickGroup TickGroup => TickGroup.Update;
    public void Tick(SceneWorld world, float dt) { ... }
}

// 注册 + Tick
world.Systems.Register(new MySystem());
world.Tick(deltaTime);  // 内部按 TickGroup 顺序调度
```

Tick 流：`Managed EarlyUpdate → Managed Update → Native TickSystems → Managed LateUpdate → Managed Render → Events.FlushDeferred`

接口体系：`ISceneSystem`（标记）→ `ISystemInitialize` / `ISystemShutdown` / `ISystemTick` / `ISystemFixedTick` / `ISystemLateTick`

### 3.5 Prefab 系统 — 资产 + 实例 + 覆盖

```csharp
// 从运行时实体构建 Prefab
var asset = PrefabAsset.FromEntity(world, rootEntityHandle);

// 实例化
var instance = PrefabInstantiator.Instantiate(asset, world);

// 检测差异
var overrides = PrefabInstantiator.DetectDifferences(world, instance);

// 应用/还原覆盖
PrefabInstantiator.ApplyOverrides(instance, world);
PrefabInstantiator.RevertToPrefab(instance, world);
```

覆盖类型：`PropertyModified` / `ComponentAdded` / `ComponentRemoved` / `ChildAdded` / `ChildRemoved`

便捷 API（旧 `Prefab` 类保留）：`new Prefab("Name").WithComponent<T>().Instantiate(world)`

### 3.6 EntityFactory — Unity 风格实体工厂

```csharp
// 创建 Camera 实体（自动挂载 Transform + Camera 组件）
var camera = EntityFactory.CreateCamera(world);
var camera2 = EntityFactory.CreateCamera(world, "Main Camera",
    position: new NNVec3 { X = 0, Y = 5, Z = -10 },
    fovY: 75.0f);

// 后续可扩展
// var light = EntityFactory.CreateDirectionalLight(world);
// var sprite = EntityFactory.CreateSprite(world, textureId);
```

`EntityFactory.CreateCamera` 内部流程：
1. `world.Entities.Create(displayName)` — 创建并注册实体
2. `entity.AddComponent<NNTransformData>()` + `entity.SetComponent(...)` — Identity 变换
3. `entity.AddComponent<NNCameraComponentData>()` + `entity.SetComponent(...)` — 透视投影默认参数

所有组件操作均经 `SceneEntity` 接口，不直接引用 `SceneNativeBridge`。

### 3.6 事件总线 — 同步 + 延迟

```csharp
// 订阅
world.Events.Subscribe(SceneEventType.EntityCreated, evt => { ... });
world.Events.SubscribeAll(evt => { ... });

// 立即分发（同步）
world.Events.Emit(SceneEvent.OnEntityCreated(handle));

// 延迟分发（入队，Tick 末尾统一处理）
world.Events.EmitDeferred(evt);
world.Events.FlushDeferred();  // 在 SceneWorld.Tick() 末尾自动调用
```

递归 Emit 自动降级为 deferred，防止栈溢出。

### 3.7 热重载 — Snapshot + Rebuild

```csharp
// 保存快照（程序集卸载前）
var snapshot = manager.SaveAllSnapshots();

// [程序集重载]

// 恢复快照（新程序集加载后）
manager.RestoreAllSnapshots(snapshot);
manager.RebuildAllAfterReload();  // 关闭旧 System，允许重新注册
```

Native 场景数据（entt::registry）不受 C# 程序集重载影响，快照仅保存 Managed 侧映射。

---

## 4. 构建

```powershell
# 构建 Scene 模块
dotnet build Engine/Source/Managed/Runtime/Neverness.Runtime.Scene/Neverness.Runtime.Scene.csproj

# 构建依赖项目（Editor.Serialization）
dotnet build Engine/Source/Managed/Editor/Neverness.Editor.Serialization/Neverness.Editor.Serialization.csproj

# 运行测试
dotnet test Engine/Source/Managed/Runtime/Tests/NevernessRuntimeManaged-Foundation.Tests.csproj
```

---

## 5. 开发进展

| 日期 | 进展 |
|------|------|
| 2026-05-15 | 初始：Phase 5.3 JSON 再水合（Object 路径） |
| 2026-05-19 | 删除 `Neverness.Runtime.Entity`；Scene 改为 Native ABI 薄门面 |
| 2026-05-23 | **Phase 4-D** 合入：ComponentId 重构 — `ComponentIdAttribute`（Engine 程序集）+ `ComponentTypeCache<T>` 替代 `ComponentTypeRegistry<T>`、`NNTransformData` 标注 `[ComponentId]`、`SceneNativeBridge` 全部切换到 `ComponentTypeCache<T>`。 |
| 2026-05-23 | **Phase 4-E** 合入：SceneWorld 提取 — `EntityRegistry`（handle↔entity 映射）、`SceneQueryCache`、`SceneWorld`（Gameplay Runtime Root）、`SceneManager` 降级为世界管理器、10 个 Scene 测试全绿。 |
| 2026-05-23 | **Phase 4-F** 合入：Query 系统 — Native 端新增 queryEntities/queryComponents/queryCount2 ABI（layoutVersion 5→6）、`NativeQueryBridge`、`SceneView<T>`/`SceneView<T1,T2>` ref struct 零 GC 视图、`SceneQuery<T>`/`SceneQueryCache`。 |
| 2026-05-23 | **Phase 4-G** 合入：System 框架 — `TickGroup` 枚举、`ISceneSystem` + 5 子接口、`SystemDependencyAttribute`、`SceneSystemScheduler`（Kahn 拓扑排序 + TickGroup 分组 + 延迟 Initialize + Rebuild）、`SceneWorld.Tick()` 完整 Tick 流。 |
| 2026-05-23 | **Phase 4-H** 合入：Prefab 系统 — `PrefabAsset`/`PrefabEntityData`/`PrefabInstance`/`PrefabOverride`/`PrefabInstantiator`（全部 5 种覆盖类型 Apply + Revert）、`SceneNativeBridge` 新增 SetComponentData + 原始 TypeId AddComponent/RemoveComponent、旧 `Prefab` 类重构为便捷包装器。 |
| 2026-05-23 | **Phase 4-I** 合入：事件系统 + 热重载 — `SceneEventBus`/`SceneEvent`/`NativeEventBridge`（stub）、`HotReloadSnapshot`/`GlobalHotReloadSnapshot`、`SceneWorld.SaveSnapshot`/`RestoreFromSnapshot`/`RebuildAfterReload`、`SceneSystemScheduler.Rebuild()`、`EntityRegistry.Get`/`TryGet`/`SyncFromHandles`/`ExportHandleValues`、`SceneManager.SaveAllSnapshots`/`RestoreAllSnapshots`/`RebuildAllAfterReload`。 |
| 2026-05-23 | **Phase 4-J** 合入：NNCameraComponent — C# 端 `NNProjectionType` 枚举 + `NNMat4` 列主序 4x4 矩阵 + `NNCameraComponentData` blittable 结构体（`[ComponentId(0x54D1B2A64667E32E, Name = "Camera")]`），无 ABI 变更，复用现有泛型组件 API。 |
| 2026-05-23 | **EntityFactory** 合入：Unity 风格实体工厂 — `EntityFactory.CreateCamera()` 快捷创建 Camera 实体（自动挂载 Transform + Camera 组件 + 合理默认值），后续可扩展 CreateDirectionalLight / CreateSprite 等。 |
| 2026-05-23 | **目录重构**：从 Entities/Queries/Systems/Prefabs/Events 改为 Public/ + Private/ 两目录。Public（`Neverness.Runtime.Scene`）26 个文件；Private（`Neverness.Runtime.Scene.Internal`）2 个文件（ComponentTypeCache + NativeQueryBridge）。 |

---

## 6. 未完成项

- **Native 事件 ABI 桥接**：`NativeEventBridge` 当前为 stub，待 `NNSceneApi` layoutVersion 7 新增 `SubscribeEvents`/`EmitEvent` 函数指针后启用自动桥接。
- **性能基准测试**：Query 系统的零 GC 验证基准（Phase 4-F 收尾项）。
- **`SceneSubsystem` → `NNRuntimeScene`** 数据桥接或迁移工具。
- **Legacy 场景资产迁移**、C# 自动组件绑定。
- **`NNTransformSystem` 脏标记优化**（仅重算脏实体子树）。
- **Prefab Variant** 系统（继承链 + 多层覆盖）。

---

## 7. 设计约束

| 约束 | 原因 |
|------|------|
| **不允许 switch(type)** | 所有类型通过注册表 + 接口发现 |
| **不允许 God Object** | 每个类一个职责，SceneWorld 是组合者不是实现者 |
| **不允许 foreach boxing** | Query 使用 ref struct enumerator |
| **不允许 GC alloc in hot path** | Query/View/Enumerator 零 GC |
| **不允许 implicit Native state copy** | C# 不复制 ECS 数据 |
| **Runtime 不依赖 Editor** | 编译时无 Editor 引用 |
| **AOT-safe** | 无 dynamic / Reflection.Emit / 运行时代码生成 |
