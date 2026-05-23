# Neverness.Runtime.Scene — 工业级 Gameplay Runtime Framework 架构方案

> 版本：1.0
> 日期：2026-05-23
> 作者：首席 Gameplay Framework / Scene Runtime 架构师

---

## 第一部分：当前代码分析

### 1.1 当前设计优点

| 优点 | 说明 |
|------|------|
| **ABI 边界清晰** | `SceneNativeBridge` 是纯转发薄层，零逻辑，不持有状态。正确设计。 |
| **Entt ECS 基础扎实** | Native 端 `NNRuntimeScene` 已完成：世代 Handle、ECS Registry、Query、System Scheduler、EventBus、DirtyTracker、Component 反射。 |
| **FNV-1a TypeId 跨进程稳定** | Native 端 `NN_REGISTER_COMPONENT("NameUtf8", ...)` 使用显式字符串 + FNV-1a，跨编译单元/版本稳定。 |
| **ComponentTypeRegistry<T> 零开销** | 静态泛型类缓存 hash，每个 T 只算一次，后续访问零成本。 |
| **Handle-based Entity** | `NNEntity = uint64(Index + Generation)`，正确实现了世代安全。 |
| **VFS 序列化路径** | 通过 VFS 虚拟路径序列化，不直接操作文件系统，解耦良好。 |
| **多场景管理** | `SceneManager` 支持 Load/Unload/Activate 多场景。 |
| **Prefab 组合模式** | `Prefab.WithComponent<T>()` 链式注册，Builder 模式合理。 |
| **ABI 版本控制** | `layoutVersion` 逐字段校验，防止跨版本不匹配。 |

### 1.2 当前架构问题

#### 问题 1：SceneManager 是 God Object

```
SceneManager 职责：
├── 场景生命周期（Load / Unload / Activate）
├── 实体创建/销毁
├── 实体跟踪（List<SceneEntity>）
├── 场景句柄映射（Dictionary<string, ulong>）
├── 系统 Tick 驱动
└── 清理逻辑
```

**风险：** 未来加 Multiplayer、AI Gameplay、序列化等功能时，SceneManager 会膨胀为不可维护的 God Object。

**建议：** 立即拆分。SceneManager 降级为"场景生命周期管理器"，不承担实体管理、Tick 驱动等职责。

#### 问题 2：ComponentTypeRegistry 使用 typeof(T).Name — **致命隐患**

```csharp
// 当前实现
private static ulong ComputeTypeId()
{
    var name = typeof(T).Name;  // "TransformComponent"
    ulong hash = 14695981039346656037UL;
    foreach (var c in name)
    {
        hash ^= (byte)c;
        hash *= 1099511628211UL;
    }
    return hash;
}
```

**问题：**

| 风险 | 后果 |
|------|------|
| 重命名 `TransformComponent` → `Transform` | TypeId 变化，所有已保存场景文件无法加载 |
| 不同命名空间同名类型 | hash 冲突（`Physics.TransformComponent` vs `Rendering.TransformComponent`） |
| 字符编码 | 仅取 `byte` 低 8 位，Unicode 字符可能丢数据 |
| Native 端用显式字符串 `"NNTransformComponent"` | C# 端 `typeof(T).Name` = `"NNTransformComponent"` — 一旦 C# 类改名就断线 |
| 无碰撞检测 | FNV-1a 64-bit 碰撞概率极低，但理论上存在 |

**建议：** **立即重构。** 改为显式 Attribute 标注稳定 ID，不依赖类型名。

#### 问题 3：Prefab 太初级

当前 Prefab 只是一个 `List<Type>` + `Instantiate`。

**缺失：**
- 无资产引用（Prefab 不是从文件加载的资产）
- 无嵌套 Prefab
- 无 Variant / Override
- 无 Prefab 与实例的关联（实例化后无法知道来源）
- 无 Prefab 差异检测（Diff）

**建议：** Phase 4 重新设计。

#### 问题 4：C# 端没有 Query / View 系统

当前所有组件操作都是逐实体调用：

```csharp
foreach (var entity in manager.Entities)
{
    if (entity.HasComponent<TransformComponent>())
    {
        var t = entity.GetComponent<TransformComponent>();
        // ...
    }
}
```

**问题：**
- 每次 `HasComponent<T>()` 都是一次 P/Invoke → 性能灾难
- 无批量遍历能力
- 无法利用 entt 的 archetype 优势

**建议：** 必须实现 Native Query Bridge，支持批量遍历。

#### 问题 5：实体跟踪使用 List<SceneEntity>

```csharp
private readonly List<SceneEntity> _entities = [];
```

- `DestroyEntity` 调用 `_entities.Remove(entity)` = O(n) 线性查找
- 无句柄有效性验证（已销毁实体仍可调用 API）
- 无 Entity→Managed 对象映射

**建议：** 改为 `Dictionary<NNEntityHandle, SceneEntity>` + 世代校验。

#### 问题 6：Native 已有但 C# 未暴露的能力

Native `NNRuntimeScene` 已实现但 C# 完全没有访问：

| Native 能力 | C# 状态 |
|-------------|---------|
| `NNSceneEventBus` (EntityCreated/Destroyed/ParentChanged/ComponentEmplaced) | **未暴露** |
| `NNDirtyTracker` | **未暴露** |
| `NNSceneSystemScheduler` (EarlyUpdate/FixedUpdate/Update/LateUpdate/Render) | **未暴露** |
| `NNEntityQuery<Components...>` (entt view 封装) | **未暴露** |
| `ForEachAliveEntity` (序列化遍历) | **未暴露** |
| `GetChildren` (层级遍历) | **未暴露** |

C# 端只使用了 14 个 ABI 函数中的 CRUD + 序列化子集，浪费了 Native 端的大量已实现能力。

#### 问题 7：SceneEntity 绑定 SceneHandle

```csharp
public sealed class SceneEntity
{
    public NNEntityHandle Handle { get; private set; }
    public ulong SceneHandle { get; private set; }  // 硬编码绑定
}
```

- 实体被锁死在创建时的场景中
- 无法跨场景移动实体
- 无法验证 SceneHandle 是否仍然有效

### 1.3 边界合理性评估

| 边界 | 当前状态 | 评估 |
|------|---------|------|
| Native 持有 ECS 存储，C# 不持有 | 正确 | **保留** |
| C# 驱动场景生命周期 | 正确 | **保留，但职责需拆分** |
| SceneNativeBridge 纯 ABI 转发 | 正确 | **保留** |
| FNV-1a 作为 TypeId | 设计正确，实现有缺陷 | **保留概念，重构实现** |
| Prefab 在 C# 端组装 | 设计方向正确 | **保留，但需要完整重设计** |
| 序列化走 VFS | 正确 | **保留** |

### 1.4 应保留的部分

- `SceneNativeBridge` 作为 ABI 薄层（但需扩展 Query API）
- `ComponentTypeRegistry<T>` 静态泛型缓存模式（但需改 TypeId 来源）
- Handle-based Entity 设计
- `Prefab` 的 Builder 模式概念
- VFS 序列化路径
- 多场景管理概念

---

## 第二部分：推荐最终 Runtime Scene 架构

### 2.1 架构总览

```
┌─────────────────────────────────────────────────────────┐
│                    Gameplay Layer                         │
│  ┌──────────┐  ┌───────────┐  ┌──────────────────────┐  │
│  │ Systems  │  │ EventBus  │  │  Prefab / Instantiation│ │
│  └────┬─────┘  └─────┬─────┘  └──────────┬───────────┘  │
│       │              │                    │              │
│  ┌────▼──────────────▼────────────────────▼───────────┐  │
│  │                  SceneWorld                         │  │
│  │  ┌─────────┐ ┌────────┐ ┌──────────┐ ┌─────────┐  │  │
│  │  │ Entities│ │ Query  │ │ Systems  │ │ Context │  │  │
│  │  └─────────┘ └────────┘ └──────────┘ └─────────┘  │  │
│  └────────────────────┬───────────────────────────────┘  │
│                       │                                   │
│  ┌────────────────────▼───────────────────────────────┐  │
│  │              SceneNativeBridge (ABI)                │  │
│  └────────────────────┬───────────────────────────────┘  │
│                       │ C ABI                             │
├───────────────────────┼───────────────────────────────────┤
│                  Native Layer                              │
│  ┌────────────────────▼───────────────────────────────┐  │
│  │            NNRuntimeScene (entt)                    │  │
│  │  ┌─────────┐ ┌────────┐ ┌──────────┐ ┌─────────┐  │  │
│  │  │Registry │ │ Query  │ │ Scheduler│ │ EventBus│  │  │
│  │  └─────────┘ └────────┘ └──────────┘ └─────────┘  │  │
│  └────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

### 2.2 核心设计原则

| 原则 | 说明 |
|------|------|
| **Native 权威存储** | entt::registry 是 ECS 数据的唯一权威。C# 不复制。 |
| **C# Gameplay 逻辑** | C# 负责 System 行为、Prefab 组装、生命周期管理、事件响应。 |
| **零 God Object** | 每个类一个职责。SceneWorld 是组合者，不是实现者。 |
| **Registry + Reflection 驱动** | 无 switch(type)，无硬编码。所有类型通过注册表发现。 |
| **零 GC Query** | Query/View 使用 stack-allocated enumerator，无 foreach boxing。 |
| **AOT-safe** | 无 `dynamic`、无 `Reflection.Emit`、无运行时代码生成。 |
| **Hot Reload 友好** | 实体 ID 不绑定程序集实例。系统可热替换。 |
| **Multiplayer 预留** | SceneWorld 可隔离为 Server/Client 世界。 |

### 2.3 设计参考

| 引擎 | 借鉴概念 |
|------|---------|
| **Unity** | Scene/GameObject/Prefab 生命周期、PlayerLoop Tick 分组、Prefab Variant/Override |
| **Unreal** | Gameplay Framework（World/Actor/Component）、TickGroup、GameplayTag |
| **DOTS/ECS** | Entity/Component/System 分离、Query/Archetype、Burst Job |
| **Frostbite** | EBX 场景图、Entity 复合体、Streaming World |
| **O3DE** | AZ::Entity、Component Descriptor、EBus 事件 |

---

## 第三部分：SceneWorld 重点设计

### 3.1 SceneWorld 职责

```csharp
/// <summary>
/// 场景世界——Gameplay Runtime Root。
/// 持有一个 Native 场景的完整 Managed 映射，管理该场景中的所有实体、查询、系统和事件。
/// </summary>
public sealed class SceneWorld : IDisposable
{
    // ── 核心标识 ──
    public ulong NativeHandle { get; }            // Native 场景句柄
    public string Name { get; set; }              // 场景名称
    public NNGuid AssetGuid { get; set; }         // 场景资产 GUID

    // ── 子系统 ──
    public EntityRegistry Entities { get; }       // 实体注册表
    public SceneQueryCache Queries { get; }       // 查询缓存
    public SceneSystemScheduler Systems { get; }  // System 调度器
    public SceneEventBus Events { get; }          // 事件总线
    public SceneContext Context { get; }          // 运行时上下文

    // ── 生命周期 ──
    public static SceneWorld Create(string name);
    public static SceneWorld LoadFromAsset(string vfsPath);
    public void Save(string vfsPath);
    public void Dispose();

    // ── Tick ──
    public void Tick(float deltaTime);
    public void FixedUpdate(float fixedDeltaTime);
    public void LateTick(float deltaTime);
}
```

### 3.2 SceneWorld 生命周期

```
SceneWorld.Create("MyWorld")
  │
  ├─ SceneNativeBridge.CreateScene() → NativeHandle
  ├─ EntityRegistry 初始化
  ├─ SystemScheduler 初始化
  ├─ EventBus 订阅 Native 事件桥接
  └─ Context 初始化
  │
  ▼
[Game Loop]
  │
  ├─ world.Tick(deltaTime)
  │   ├─ Systems.Tick(EarlyUpdate, deltaTime)
  │   ├─ Systems.Tick(FixedUpdate, fixedDt)
  │   ├─ Systems.Tick(Update, deltaTime)      ← Gameplay 逻辑
  │   ├─ Systems.Tick(LateUpdate, deltaTime)
  │   ├─ SceneNativeBridge.TickSystems()       ← 驱动 Native 侧 System
  │   └─ Events.FlushDeferred()                ← 处理延迟事件
  │
  ▼
world.Dispose()
  │
  ├─ Systems.Dispose()
  ├─ Events.Clear()
  ├─ Entities.Clear()
  └─ SceneNativeBridge.DestroyScene(NativeHandle)
```

### 3.3 SceneWorld 与 SceneManager 的关系

```
SceneManager（降级为"世界管理器"）
├── 场景生命周期（Load / Unload / Activate）
├── World 映射（name → SceneWorld）
└── 不再持有实体列表，不再 Tick

SceneWorld（每个场景的完整运行时）
├── 持有一个 Native 场景
├── 管理该场景的所有实体
├── 驱动该场景的 System Tick
└── 管理该场景的事件和查询
```

```csharp
public sealed class SceneManager
{
    private readonly Dictionary<string, SceneWorld> _worlds = new();
    private SceneWorld? _activeWorld;

    public SceneWorld? ActiveWorld => _activeWorld;

    public SceneWorld LoadWorld(string name) { ... }
    public void UnloadWorld(string name) { ... }
    public void ActivateWorld(string name) { ... }

    // 不再持有 _entities，不再 CreateEntity/DestroyEntity
    // 实体操作全部通过 SceneWorld.Entities 进行
}
```

### 3.4 SceneWorld 与 Native Scene 的关系

```
C# SceneWorld                    Native NNRuntimeScene
┌──────────────┐                ┌──────────────────────┐
│ NativeHandle │───────────────│ m_Registry (entt)     │
│              │  C ABI 调用    │ m_EntityTable         │
│ Entities ────│──────────────│ m_ComponentRegistry   │
│ Systems ────│──────────────│ m_SystemScheduler     │
│ Events ─────│──────────────│ m_EventBus            │
│ Queries ────│──────────────│ m_DirtyTracker        │
└──────────────┘                └──────────────────────┘
```

- SceneWorld 不复制 Native 数据
- 每次读取通过 Bridge 调用
- 写入通过 Bridge 调用
- Native System 由 Bridge.TickSystems 驱动
- Managed System 由 SceneWorld.Systems.Tick 驱动

### 3.5 SceneWorld 与 System Scheduler 的关系

```
SceneWorld.Tick(dt)
  │
  ├─ Systems.Tick(EarlyUpdate, dt)    ← Managed System
  ├─ Systems.Tick(FixedUpdate, fdt)   ← Managed System
  ├─ Systems.Tick(Update, dt)         ← Managed System
  │
  ├─ SceneNativeBridge.TickSystems()  ← Native System（由 Native Scheduler 调度）
  │
  ├─ Systems.Tick(LateUpdate, dt)     ← Managed System
  └─ Systems.Tick(Render, dt)         ← Managed System
```

**关键：** Managed System 和 Native System 在各自的 Tick 阶段独立调度，互不阻塞。Native System 由 Native Scheduler 在 `TickSystems` 内部按 Native TickGroup 调度。Managed System 由 C# Scheduler 在 `Systems.Tick` 内按 Managed TickGroup 调度。

### 3.6 SceneWorld 与 Query API 的关系

```csharp
// 通过 SceneWorld 获取缓存查询
var query = world.GetQuery<NNTransformData, NNTagComponent>();

// 零 GC 遍历
foreach (ref var entry in query)
{
    ref var transform = ref entry.Get<NNTransformData>();
    transform.Position.X += 1.0f;
    entry.Set(transform);
}
```

Query 由 `SceneWorld.Queries` 缓存管理，避免重复创建。

### 3.7 完整目录结构

```
Neverness.Runtime.Scene/
├── Neverness.Runtime.Scene.csproj
├── SceneWorld.cs                          # 场景世界根
├── SceneManager.cs                        # 场景管理器（降级为世界管理器）
├── SceneEntity.cs                         # 实体句柄门面
│
├── Entities/
│   ├── EntityRegistry.cs                  # 实体注册表（handle → entity 映射）
│   └── EntityHandleExtensions.cs          # 句柄工具扩展
│
├── Queries/
│   ├── SceneQuery.cs                      # 单组件查询
│   ├── SceneView.cs                       # 多组件视图（零 GC enumerator）
│   ├── SceneQueryCache.cs                 # 查询缓存
│   └── NativeQueryBridge.cs               # Native Query ABI 桥接
│
├── Systems/
│   ├── ISceneSystem.cs                    # Managed System 接口
│   ├── ISystemTick.cs                     # Tick 接口
│   ├── ISystemFixedTick.cs                # FixedTick 接口
│   ├── ISystemLateTick.cs                 # LateTick 接口
│   ├── ISystemInitialize.cs               # Initialize 接口
│   ├── ISystemShutdown.cs                 # Shutdown 接口
│   ├── SceneSystemScheduler.cs            # System 调度器
│   ├── TickGroup.cs                       # Managed TickGroup 枚举
│   └── SystemDependencyAttribute.cs       # 依赖排序特性
│
├── Components/
│   ├── ComponentIdAttribute.cs            # [ComponentId(0x...)] 稳定 ID
│   ├── ComponentIdRegistry.cs             # 组件 ID 注册表
│   ├── ComponentMetadata.cs               # 组件元数据
│   └── ComponentTypeCache.cs              # 泛型类型缓存
│
├── Prefabs/
│   ├── PrefabAsset.cs                     # Prefab 资产
│   ├── PrefabInstance.cs                  # Prefab 实例
│   ├── PrefabOverride.cs                  # Override 描述
│   └── PrefabInstantiator.cs              # 实例化器
│
├── Serialization/
│   ├── SceneAsset.cs                      # 场景资产描述
│   ├── AssetReference.cs                  # 资产引用（GUID + Path）
│   ├── AssetGuid.cs                       # 128-bit GUID
│   ├── NNSceneSerializeBridge.cs          # 序列化 ABI 桥接
│   └── SceneSerializer.cs                 # C# 侧序列化协调
│
├── Reflection/
│   ├── PropertyMetadata.cs                # 属性元数据
│   ├── SerializedAttribute.cs             # [Serialized]
│   ├── RangeAttribute.cs                  # [Range(min, max)]
│   └── HideInInspectorAttribute.cs        # [HideInInspector]
│
└── Events/
    ├── SceneEvent.cs                      # 场景事件定义
    ├── SceneEventBus.cs                   # C# 事件总线
    ├── SceneEventHandler.cs               # 事件处理器委托
    └── NativeEventBridge.cs               # Native EventBus 桥接
```

---

## 第四部分：工业级 ECS Query 系统

### 4.1 设计目标

| 目标 | 实现 |
|------|------|
| **零 GC** | Stack-allocated enumerator，ref struct |
| **AOT-safe** | 无 Reflection.Emit，纯泛型特化 |
| **高性能** | 批量获取组件指针，减少 P/Invoke 次数 |
| **无 foreach boxing** | `ref struct` enumerator 不实现 `IEnumerator<T>` |
| **可缓存** | Query 对象可复用，不重复创建 |

### 4.2 核心类型

#### SceneView<T> — 单组件批量遍历

```csharp
/// <summary>
/// 单组件视图——通过 Native ABI 批量获取匹配实体的组件数据。
/// ref struct 保证零 GC、零 boxing。
/// </summary>
public ref struct SceneView<T> where T : unmanaged
{
    private readonly ulong _sceneHandle;
    private readonly Span<ulong> _entityHandles;
    private readonly Span<byte> _componentBuffer;
    private int _index;

    internal SceneView(ulong sceneHandle, Span<ulong> handles, Span<byte> buffer)
    {
        _sceneHandle = sceneHandle;
        _entityHandles = handles;
        _componentBuffer = buffer;
        _index = -1;
    }

    public readonly int Length => _entityHandles.Length;

    public SceneViewEnumerator<T> GetEnumerator() => new(this);

    public ref struct SceneViewEnumerator<T>
    {
        private readonly SceneView<T> _view;
        private int _index;

        internal SceneViewEnumerator(SceneView<T> view) { _view = view; _index = -1; }

        public bool MoveNext() => ++_index < _view._entityHandles.Length;

        public readonly SceneViewEntry<T> Current
        {
            get => new(_view._entityHandles[_index], _view.GetComponentRef(_index));
        }
    }
}

/// <summary>
/// 视图条目——提供 Entity 句柄和组件 ref 访问。
/// </summary>
public readonly ref struct SceneViewEntry<T> where T : unmanaged
{
    public readonly NNEntityHandle Entity;
    private readonly ref T _component;

    public ref T Component => ref _component;

    internal SceneViewEntry(NNEntityHandle entity, ref T component)
    {
        Entity = entity;
        _component = ref component;
    }
}
```

#### SceneView<T1, T2> — 双组件批量遍历

```csharp
public ref struct SceneView<T1, T2>
    where T1 : unmanaged
    where T2 : unmanaged
{
    private readonly ulong _sceneHandle;
    private readonly Span<ulong> _entityHandles;
    private readonly Span<byte> _buffer1;
    private readonly Span<byte> _buffer2;

    public SceneViewEnumerator GetEnumerator() => new(this);

    public ref struct SceneViewEnumerator
    {
        // ...
        public readonly SceneViewEntry<T1, T2> Current { get; }
    }
}

public readonly ref struct SceneViewEntry<T1, T2>
    where T1 : unmanaged
    where T2 : unmanaged
{
    public readonly NNEntityHandle Entity;
    public ref T1 Component1 => ref _c1;
    public ref T2 Component2 => ref _c2;
}
```

#### SceneQuery<T> — 缓存式查询

```csharp
/// <summary>
/// 缓存式查询——持有 Native query token，支持增量更新。
/// </summary>
public sealed class SceneQuery<T> where T : unmanaged
{
    private readonly ulong _sceneHandle;
    private ulong _nativeQueryToken;  // Native query 缓存句柄

    /// <summary>执行查询并获取当前匹配视图。</summary>
    public SceneView<T> Execute()
    {
        // 经 NativeQueryBridge 获取匹配实体句柄和组件数据缓冲区
        var (handles, buffer) = NativeQueryBridge.QuerySingle<T>(_sceneHandle, _nativeQueryToken);
        return new SceneView<T>(_sceneHandle, handles, buffer);
    }

    /// <summary>获取匹配数量（不获取数据）。</summary>
    public int Count => NativeQueryBridge.GetQueryCount(_sceneHandle, _nativeQueryToken);
}
```

#### SceneQueryCache — 查询对象池

```csharp
/// <summary>
/// 查询缓存——避免重复创建 Query 对象。
/// </summary>
public sealed class SceneQueryCache
{
    private readonly ulong _sceneHandle;
    private readonly Dictionary<ulong, object> _cache = new();

    public SceneQuery<T> GetQuery<T>() where T : unmanaged
    {
        var key = ComponentTypeCache<T>.TypeId;
        if (!_cache.TryGetValue(key, out var query))
        {
            query = new SceneQuery<T>(_sceneHandle);
            _cache[key] = query;
        }
        return (SceneQuery<T>)query;
    }

    public SceneQuery<T1, T2> GetQuery<T1, T2>()
        where T1 : unmanaged
        where T2 : unmanaged { ... }
}
```

### 4.3 Native Query Bridge ABI

需要在 NNSceneApi 中新增以下函数指针（layoutVersion 6）：

```c
// 查询单组件匹配实体
typedef NNSceneResult (NN_ENGINE_ABI_STDCALL* NNSceneQueryEntitiesFn)(
    NNSceneHandle scene,
    uint64_t componentTypeId,
    NNEntityHandle* outEntities,    // 输出缓冲区
    uint32_t maxCount,              // 缓冲区容量
    uint32_t* outCount);            // 实际匹配数

// 批量获取组件数据（减少 P/Invoke 次数）
typedef NNSceneResult (NN_ENGINE_ABI_STDCALL* NNSceneQueryComponentsFn)(
    NNSceneHandle scene,
    uint64_t componentTypeId,
    NNEntityHandle* entities,       // 实体数组
    uint32_t entityCount,           // 实体数量
    void* outData,                  // 输出缓冲区（entityCount * componentSize）
    uint32_t componentSize);

// 查询双组件交集数量
typedef NNSceneResult (NN_ENGINE_ABI_STDCALL* NNSceneQueryCount2Fn)(
    NNSceneHandle scene,
    uint64_t typeId1,
    uint64_t typeId2,
    uint32_t* outCount);
```

### 4.4 使用示例

```csharp
// 获取缓存的查询
var query = world.Queries.GetQuery<NNTransformData>();

// 零 GC 遍历
foreach (ref var entry in query.Execute())
{
    ref var transform = ref entry.Component;
    transform.Position.X += 10.0f;
    // 直接修改 Native 内存（组件数据在 Span<byte> 中已映射）
}

// 双组件查询
var query2 = world.Queries.GetQuery<NNTransformData, NNTagComponent>();
foreach (ref var entry in query2.Execute())
{
    ref var transform = ref entry.Component1;
    ref var tag = ref entry.Component2;
    // ...
}
```

---

## 第五部分：工业级 Component Metadata

### 5.1 当前问题总结

| 问题 | 风险等级 |
|------|---------|
| `typeof(T).Name` 重命名 → TypeId 变化 | **致命** |
| 不同命名空间同名类型 → hash 冲突 | **高** |
| Native 显式字符串 vs C# 类名不一致 | **高** |
| 无碰撞检测 | **中** |
| 不支持 SourceGenerator | **中** |

### 5.2 新设计：显式 ComponentId Attribute

```csharp
/// <summary>
/// 组件稳定标识特性——显式声明组件的跨版本 TypeId。
/// 不依赖类型名，rename/namespace 不影响 ID。
/// </summary>
[AttributeUsage(AttributeTargets.Struct, Inherited = false, AllowMultiple = false)]
public sealed class ComponentIdAttribute : Attribute
{
    /// <summary>稳定的 64-bit TypeId（须与 Native NN_REGISTER_COMPONENT 的 FNV-1a 显式字符串一致）。</summary>
    public ulong TypeId { get; }

    /// <summary>可选的显式名称（用于序列化和 Native 对齐）。</summary>
    public string? Name { get; }

    public ComponentIdAttribute(ulong typeId, string? name = null)
    {
        TypeId = typeId;
        Name = name;
    }
}
```

### 5.3 使用方式

```csharp
// FNV-1a("NNTransformComponent") = 0x...
// Native 端：NN_REGISTER_COMPONENT(NNTransformComponent, "NNTransformComponent", ...)
// C# 端：显式声明相同 ID
[ComponentId(0xA1B2C3D4E5F60001, Name = "NNTransformComponent")]
public struct NNTransformData
{
    public NNVec3 Position;
    public NNQuat Rotation;
    public NNVec3 Scale;
}
```

### 5.4 ComponentIdRegistry

```csharp
/// <summary>
/// 组件 ID 注表——启动时扫描所有程序集中 [ComponentId] 特性并注册。
/// </summary>
public static class ComponentIdRegistry
{
    private static readonly Dictionary<ulong, ComponentMetadata> s_idToMeta = new();
    private static readonly Dictionary<Type, ComponentMetadata> s_typeToMeta = new();
    private static bool s_scanned;

    public static ComponentMetadata Get<T>() where T : unmanaged
    {
        EnsureScanned();
        return s_typeToMeta[typeof(T)];
    }

    public static ComponentMetadata? FindById(ulong typeId)
    {
        EnsureScanned();
        s_idToMeta.TryGetValue(typeId, out var meta);
        return meta;
    }

    public static void ScanAssemblies(params Assembly[] assemblies) { ... }
    public static void ScanAllLoaded() { ... }
}
```

### 5.5 ComponentMetadata

```csharp
/// <summary>
/// 组件元数据——注册表中的完整描述。
/// </summary>
public sealed class ComponentMetadata
{
    /// <summary>稳定的 64-bit TypeId。</summary>
    public ulong TypeId { get; init; }

    /// <summary>CLR 类型（typeof(T)）。</summary>
    public Type ClrType { get; init; } = null!;

    /// <summary>显示名称（用于序列化、Inspector）。</summary>
    public string DisplayName { get; init; } = string.Empty;

    /// <summary>Native 对齐名称（须与 NN_REGISTER_COMPONENT 的 NameUtf8 一致）。</summary>
    public string NativeName { get; init; } = string.Empty;

    /// <summary>字节数（Marshal.SizeOf<T>()）。</summary>
    public int SizeBytes { get; init; }

    /// <summary>字段描述列表（供 Inspector 和序列化使用）。</summary>
    public IReadOnlyList<ComponentFieldMetadata> Fields { get; init; } = [];
}
```

### 5.6 ComponentTypeCache<T>（替代 ComponentTypeRegistry<T>）

```csharp
/// <summary>
/// 泛型组件类型缓存——从 [ComponentId] 特性读取 TypeId，零开销静态缓存。
/// </summary>
internal static class ComponentTypeCache<T> where T : unmanaged
{
    public static readonly ulong TypeId = ResolveTypeId();
    public static readonly int SizeBytes = Marshal.SizeOf<T>();

    private static ulong ResolveTypeId()
    {
        var attr = typeof(T).GetCustomAttribute<ComponentIdAttribute>();
        if (attr != null)
            return attr.TypeId;

        // 回退：FNV-1a of explicit name（仅用于无 [ComponentId] 的临时组件）
        var name = typeof(T).Name;
        return Fnv1a64(name);
    }

    private static ulong Fnv1a64(string name)
    {
        ulong hash = 14695981039346656037UL;
        foreach (var c in name)
        {
            hash ^= (byte)c;
            hash *= 1099511628211UL;
        }
        return hash;
    }
}
```

### 5.7 迁移路径

```
Phase 1（立即）：
  - 新增 ComponentIdAttribute
  - 新增 ComponentTypeCache<T>（同时保留 ComponentTypeRegistry<T> 的 Fallback）
  - 所有已知组件添加 [ComponentId]

Phase 2：
  - 废弃 ComponentTypeRegistry<T>
  - SceneNativeBridge 全部使用 ComponentTypeCache<T>
  - 扫描警告：未标注 [ComponentId] 的组件发出编译警告

Phase 3：
  - SourceGenerator 自动生成 [ComponentId] 和 ComponentMetadata
  - 无 Attribute 时编译错误
```

---

## 第六部分：Managed Gameplay System Framework

### 6.1 System 接口体系

```csharp
/// <summary>System 基础标记接口。</summary>
public interface ISceneSystem
{
    /// <summary>系统名称（用于调试和日志）。</summary>
    string Name => GetType().Name;
}

/// <summary>可初始化系统。</summary>
public interface ISystemInitialize : ISceneSystem
{
    void Initialize(SceneWorld world);
}

/// <summary>可关闭系统。</summary>
public interface ISystemShutdown : ISceneSystem
{
    void Shutdown(SceneWorld world);
}

/// <summary>每帧 Tick 系统。</summary>
public interface ISystemTick : ISceneSystem
{
    TickGroup TickGroup => TickGroup.Update;
    void Tick(SceneWorld world, float deltaTime);
}

/// <summary>固定步长 Tick 系统。</summary>
public interface ISystemFixedTick : ISceneSystem
{
    TickGroup TickGroup => TickGroup.FixedUpdate;
    void FixedTick(SceneWorld world, float fixedDeltaTime);
}

/// <summary>每帧 Late Tick 系统。</summary>
public interface ISystemLateTick : ISceneSystem
{
    TickGroup TickGroup => TickGroup.LateUpdate;
    void LateTick(SceneWorld world, float deltaTime);
}
```

### 6.2 TickGroup

```csharp
/// <summary>
/// Managed System Tick 分组——与 Native NNSceneTickGroup 对齐。
/// </summary>
public enum TickGroup : byte
{
    EarlyUpdate = 0,
    PreUpdate = 1,
    Update = 2,
    PostUpdate = 3,
    FixedUpdate = 4,
    LateUpdate = 5,
    Render = 6,
}
```

### 6.3 SystemDependencyAttribute

```csharp
/// <summary>
/// System 依赖排序特性——声明当前 System 必须在哪些 System 之后执行。
/// </summary>
[AttributeUsage(AttributeTargets.Class, Inherited = false, AllowMultiple = true)]
public sealed class SystemDependencyAttribute : Attribute
{
    public Type DependsOn { get; }

    public SystemDependencyAttribute(Type dependsOn)
    {
        DependsOn = dependsOn;
    }
}
```

### 6.4 SceneSystemScheduler

```csharp
/// <summary>
/// Managed System 调度器——注册、排序、按 TickGroup 分组执行。
/// </summary>
public sealed class SceneSystemScheduler : IDisposable
{
    private readonly SortedDictionary<int, List<ISceneSystem>> _systemsByGroup = new();
    private readonly List<ISceneSystem> _allSystems = [];
    private readonly SceneWorld _world;
    private bool _initialized;

    public SceneSystemScheduler(SceneWorld world) { _world = world; }

    public void Register(ISceneSystem system)
    {
        _allSystems.Add(system);
        _initialized = false; // 触发重新排序
    }

    public void Unregister(ISceneSystem system) { ... }

    /// <summary>按拓扑排序和 TickGroup 分组执行。</summary>
    public void Tick(TickGroup group, float deltaTime)
    {
        EnsureInitialized();
        if (!_systemsByGroup.TryGetValue((int)group, out var systems))
            return;

        foreach (var system in systems)
        {
            if (system is ISystemTick tick)
                tick.Tick(_world, deltaTime);
        }
    }

    public void FixedTick(TickGroup group, float fixedDeltaTime)
    {
        EnsureInitialized();
        if (!_systemsByGroup.TryGetValue((int)group, out var systems))
            return;

        foreach (var system in systems)
        {
            if (system is ISystemFixedTick tick)
                tick.FixedTick(_world, fixedDeltaTime);
        }
    }

    private void EnsureInitialized()
    {
        if (_initialized) return;
        _initialized = true;
        TopologicalSort();
    }

    private void TopologicalSort() { ... }

    public void Dispose()
    {
        foreach (var system in _allSystems)
        {
            if (system is ISystemShutdown shutdown)
                shutdown.Shutdown(_world);
        }
    }
}
```

### 6.5 使用示例

```csharp
// 定义 System
[Dependency(typeof(MovementSystem))]
public class MovementSystem : ISystemTick
{
    public TickGroup TickGroup => TickGroup.Update;

    public void Tick(SceneWorld world, float deltaTime)
    {
        var query = world.Queries.GetQuery<NNTransformData>();
        foreach (ref var entry in query.Execute())
        {
            ref var t = ref entry.Component;
            t.Position.X += deltaTime * 10.0f;
        }
    }
}

// 注册
world.Systems.Register(new MovementSystem());

// Tick
world.Tick(deltaTime);  // 内部按 TickGroup 顺序调度
```

---

## 第七部分：Prefab Runtime Framework

### 7.1 架构分层

```
PrefabAsset（资产层）
├── EntityData[]       # 原始实体数据（组件快照）
├── Children[]         # 嵌套子实体
└── AssetGuid          # 资产 GUID

PrefabInstance（实例层）
├── SourceAsset        # → PrefabAsset 引用
├── RootEntity         # 根实体句柄
├── InstanceMap        # prefab entity index → runtime entity handle
└── Overrides[]        # 属性覆盖列表
```

### 7.2 PrefabAsset

```csharp
/// <summary>
/// Prefab 资产——描述可实例化的实体子图。
/// 可从文件加载（经 VFS）或运行时构建。
/// </summary>
public sealed class PrefabAsset
{
    /// <summary>资产 GUID。</summary>
    public NNGuid Guid { get; init; }

    /// <summary>Prefab 名称。</summary>
    public string Name { get; init; } = string.Empty;

    /// <summary>原始实体描述列表。</summary>
    public List<PrefabEntityData> Entities { get; } = [];

    /// <summary>从 VFS 路径加载 Prefab。</summary>
    public static PrefabAsset? Load(string vfsPath) { ... }

    /// <summary>保存到 VFS 路径。</summary>
    public void Save(string vfsPath) { ... }

    /// <summary>从 SceneWorld 中选择的实体构建 Prefab。</summary>
    public static PrefabAsset FromEntities(SceneWorld world, IReadOnlyList<NNEntityHandle> entities) { ... }
}

/// <summary>
/// Prefab 中单个实体的描述。
/// </summary>
public sealed class PrefabEntityData
{
    /// <summary>在 Prefab 内的局部索引（0 = 根实体）。</summary>
    public int LocalIndex { get; init; }

    /// <summary>父实体的局部索引（-1 = 无父）。</summary>
    public int ParentIndex { get; init; } = -1;

    /// <summary>显示名称。</summary>
    public string DisplayName { get; init; } = "Entity";

    /// <summary>组件数据列表（TypeId → 二进制 blob）。</summary>
    public Dictionary<ulong, byte[]> Components { get; } = new();
}
```

### 7.3 PrefabInstance

```csharp
/// <summary>
/// Prefab 实例——运行时实体子图与源 Prefab 的关联。
/// </summary>
public sealed class PrefabInstance
{
    /// <summary>源 Prefab 引用。</summary>
    public PrefabAsset Source { get; }

    /// <summary>根实体句柄。</summary>
    public NNEntityHandle RootEntity { get; internal set; }

    /// <summary>Prefab 局部索引 → 运行时实体句柄 映射。</summary>
    public IReadOnlyDictionary<int, NNEntityHandle> InstanceMap => _instanceMap;

    /// <summary>属性覆盖列表。</summary>
    public IReadOnlyList<PrefabOverride> Overrides => _overrides;

    // ... 内部字段 ...
}
```

### 7.4 PrefabOverride

```csharp
/// <summary>
/// Prefab 覆盖类型。
/// </summary>
public enum PrefabOverrideType : byte
{
    /// <summary>组件属性值被修改。</summary>
    PropertyModified,
    /// <summary>添加了新组件。</summary>
    ComponentAdded,
    /// <summary>移除了组件。</summary>
    ComponentRemoved,
    /// <summary>添加了子实体。</summary>
    ChildAdded,
    /// <summary>移除了子实体。</summary>
    ChildRemoved,
}

/// <summary>
/// 单条 Prefab 覆盖。
/// </summary>
public sealed class PrefabOverride
{
    /// <summary>覆盖类型。</summary>
    public PrefabOverrideType Type { get; init; }

    /// <summary>受影响的实体局部索引。</summary>
    public int EntityLocalIndex { get; init; }

    /// <summary>受影响的组件 TypeId（PropertyModified / ComponentAdded / ComponentRemoved）。</summary>
    public ulong ComponentTypeId { get; init; }

    /// <summary>属性路径（PropertyModified）。</summary>
    public string? PropertyPath { get; init; }

    /// <summary>新值（PropertyModified）。</summary>
    public byte[]? NewValue { get; init; }
}
```

### 7.5 PrefabInstantiator

```csharp
/// <summary>
/// Prefab 实例化器——将 PrefabAsset 实例化到 SceneWorld 中。
/// </summary>
public static class PrefabInstantiator
{
    /// <summary>将 Prefab 实例化到场景中。</summary>
    public static PrefabInstance Instantiate(SceneWorld world, PrefabAsset prefab)
    {
        var instance = new PrefabInstance(prefab);
        var map = new Dictionary<int, NNEntityHandle>();

        // 按局部索引顺序创建实体
        for (int i = 0; i < prefab.Entities.Count; i++)
        {
            var data = prefab.Entities[i];
            var handle = world.Entities.Create(data.DisplayName);

            // 写入组件
            foreach (var (typeId, blob) in data.Components)
            {
                NativeQueryBridge.SetComponentData(world.NativeHandle, handle, typeId, blob);
            }

            map[i] = handle;
        }

        // 建立父子关系
        for (int i = 0; i < prefab.Entities.Count; i++)
        {
            var data = prefab.Entities[i];
            if (data.ParentIndex >= 0)
            {
                world.Entities.SetParent(map[i], map[data.ParentIndex]);
            }
        }

        instance.SetInstanceMap(map);
        instance.RootEntity = map[0];
        return instance;
    }

    /// <summary>将 Prefab 实例的修改应用回 PrefabAsset。</summary>
    public static void ApplyOverrides(PrefabInstance instance) { ... }

    /// <summary>将 PrefabAsset 的修改重新应用到实例（Revert to Prefab）。</summary>
    public static void RevertToPrefab(PrefabInstance instance) { ... }

    /// <summary>检测实例与源 Prefab 的差异。</summary>
    public static IReadOnlyList<PrefabOverride> DetectDifferences(PrefabInstance instance) { ... }
}
```

### 7.6 兼容旧 Prefab

旧 `Prefab` 类可保留为便捷 API，内部委托给 `PrefabAsset` + `PrefabInstantiator`：

```csharp
public sealed class Prefab
{
    private readonly PrefabAsset _asset;

    public Prefab(string name, string templateDisplayName = "Entity")
    {
        _asset = new PrefabAsset { Name = name };
        _asset.Entities.Add(new PrefabEntityData
        {
            LocalIndex = 0,
            DisplayName = templateDisplayName,
        });
    }

    public Prefab WithComponent<T>() where T : unmanaged
    {
        // 注册组件到根实体
        _asset.Entities[0].Components[ComponentTypeCache<T>.TypeId] = Array.Empty<byte>();
        return this;
    }

    public SceneEntity? Instantiate(SceneWorld world)
    {
        var instance = PrefabInstantiator.Instantiate(world, _asset);
        return world.Entities.Get(instance.RootEntity);
    }
}
```

---

## 第八部分：Serialization Graph

### 8.1 SceneAsset

```csharp
/// <summary>
/// 场景资产描述——对应 .scene 文件。
/// </summary>
public sealed class SceneAsset
{
    /// <summary>格式版本。</summary>
    public int FormatVersion { get; set; } = CurrentVersion;
    public const int CurrentVersion = 3;

    /// <summary>场景 GUID。</summary>
    public NNGuid Guid { get; set; }

    /// <summary>场景名称。</summary>
    public string Name { get; set; } = string.Empty;

    /// <strong>实体图（有序，根实体在前）。</summary>
    public List<SceneEntityEntry> Entities { get; } = [];

    /// <summary>资产引用列表（子场景引用、Prefab 引用等）。</summary>
    public List<AssetReference> AssetReferences { get; } = [];
}
```

### 8.2 AssetReference

```csharp
/// <summary>
/// 资产引用——通过 GUID + 可选路径 引用其他资产。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct AssetReference
{
    /// <summary>目标资产 GUID。</summary>
    public NNGuid Guid;

    /// <summary>可选的 VFS 路径缓存（GUID 解析失败时的回退）。</summary>
    public string CachedPath;

    public bool IsEmpty => Guid.High == 0 && Guid.Low == 0;
}
```

### 8.3 EntityGraph 序列化模式

```
SceneAsset
├── FormatVersion = 3
├── Guid
├── Name
├── Entities[]
│   ├── LocalIndex: uint
│   ├── ParentIndex: int (-1 = root)
│   ├── DisplayName: string
│   └── Components[]
│       ├── TypeId: ulong (FNV-1a stable)
│       └── Data: byte[] (field-level serialization)
└── AssetReferences[]
    ├── Guid
    └── CachedPath
```

序列化走 Native `NNSceneSerializer::Serialize/Deserialize`（VGSC 二进制格式）。
C# 侧的 `SceneAsset` 用于 Editor 层的 JSON 序列化、UndoRedo、Diff。

### 8.4 Runtime Scene Save

```csharp
public static class SceneSerializer
{
    /// <summary>通过 Native ABI 序列化到 VFS 路径。</summary>
    public static NNSceneResult Save(SceneWorld world, string vfsPath)
    {
        return NNSceneSerializeBridge.SerializeScene(world.NativeHandle, vfsPath);
    }

    /// <summary>从 VFS 路径加载，创建新 SceneWorld。</summary>
    public static SceneWorld? Load(SceneManager manager, string name, string vfsPath)
    {
        var (result, handle) = NNSceneSerializeBridge.DeserializeScene(vfsPath);
        if (result != NNSceneResult.Ok)
            return null;

        var world = new SceneWorld(handle, name);
        // 从 Native 同步实体到 Managed
        SyncEntitiesFromNative(world);
        return world;
    }

    /// <summary>从 Native 场景同步实体到 Managed EntityRegistry。</summary>
    private static void SyncEntitiesFromNative(SceneWorld world) { ... }
}
```

---

## 第九部分：Runtime Reflection

### 9.1 与 Editor.Reflection 的关系

```
Neverness.Editor.Reflection    ← Editor-only，UI 驱动
Neverness.Runtime.Scene.Reflection  ← Runtime，序列化和 Inspector 数据驱动
```

Runtime Reflection 不依赖 Editor。Editor.Reflection 可以依赖 Runtime.Reflection。

### 9.2 Attribute 定义

```csharp
/// <summary>标记属性为可序列化（Editor 和 Runtime 均可读写）。</summary>
[AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
public sealed class SerializedAttribute : Attribute { }

/// <summary>标记属性不在 Inspector 中显示。</summary>
[AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
public sealed class HideInInspectorAttribute : Attribute { }

/// <summary>指定 Inspector 滑动条范围。</summary>
[AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
public sealed class RangeAttribute : Attribute
{
    public float Min { get; }
    public float Max { get; }
    public RangeAttribute(float min, float max) { Min = min; Max = max; }
}

/// <summary>指定 Inspector 中的显示名称和提示。</summary>
[AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
public sealed class DisplayNameAttribute : Attribute
{
    public string Name { get; }
    public string? Tooltip { get; }
    public DisplayNameAttribute(string name, string? tooltip = null) { Name = name; Tooltip = tooltip; }
}

/// <summary>指定组件分组（Inspector 折叠面板）。</summary>
[AttributeUsage(AttributeTargets.Struct)]
public sealed class ComponentGroupAttribute : Attribute
{
    public string Group { get; }
    public ComponentGroupAttribute(string group) { Group = group; }
}
```

### 9.3 PropertyMetadata

```csharp
/// <summary>
/// 属性元数据——描述组件中的单个字段。
/// </summary>
public sealed class PropertyMetadata
{
    /// <summary>字段名称。</summary>
    public string Name { get; init; } = string.Empty;

    /// <summary>CLR 类型。</summary>
    public Type PropertyType { get; init; } = null!;

    /// <summary>在组件结构体中的字节偏移。</summary>
    public int Offset { get; init; }

    /// <summary>字节大小。</summary>
    public int Size { get; init; }

    /// <summary>是否可序列化。</summary>
    public bool IsSerialized { get; init; }

    /// <summary>是否隐藏。</summary>
    public bool IsHidden { get; init; }

    /// <summary>范围限制（如有）。</summary>
    public (float Min, float Max)? Range { get; init; }

    /// <summary>显示名称。</summary>
    public string DisplayName { get; init; } = string.Empty;
}
```

### 9.4 ComponentMetadata 扩展

在已有的 `ComponentMetadata` 上扩展：

```csharp
public sealed class ComponentMetadata
{
    // ... 已有字段 ...

    /// <summary>组件分组。</summary>
    public string Group { get; init; } = string.Empty;

    /// <summary>获取指定字段的元数据。</summary>
    public PropertyMetadata? FindField(string name) { ... }

    /// <summary>通过偏移量读取字段值。</summary>
    public object? ReadField(ReadOnlySpan<byte> componentData, string fieldName) { ... }

    /// <summary>通过偏移量写入字段值。</summary>
    public void WriteField(Span<byte> componentData, string fieldName, object value) { ... }
}
```

---

## 第十部分：Hot Reload 架构

### 10.1 挑战

| 挑战 | 说明 |
|------|------|
| **程序集重载** | C# Gameplay DLL 热重载后，所有静态字段（`ComponentTypeCache<T>`）被重置 |
| **实体句柄** | `NNEntityHandle = ulong` 是 Native 句柄，不依赖 C# 实例，重载后仍有效 |
| **System 实例** | 旧 System 实例被 GC，新程序集创建新实例 |
| **事件订阅** | 旧事件处理器被 GC，需重新订阅 |
| **场景数据** | Native 场景数据不受影响，但 Managed 侧的映射/缓存需要重建 |

### 10.2 架构策略

```
┌──────────────────────────────────────────────┐
│              C# Assembly Reload               │
│                                               │
│  1. 暂停 Tick                                 │
│  2. 保存所有 SceneWorld 的 Native 状态快照     │
│  3. 卸载旧 Assembly                           │
│  4. 加载新 Assembly                           │
│  5. 重建 ComponentTypeCache（扫描新程序集）    │
│  6. 重建 SceneWorld Managed 映射              │
│  7. 重新注册 System                           │
│  8. 重新订阅事件                               │
│  9. 恢复 Tick                                 │
│                                               │
│  Native 数据全程不受影响                       │
└──────────────────────────────────────────────┘
```

### 10.3 关键设计

#### 实体句柄不绑定程序集

```
NNEntityHandle = ulong（Native 句柄）
├── 不包含 C# 对象引用
├── 不包含程序集实例
├── 程序集重载后仍有效
└── SceneWorld.Entities.Get(handle) 在重载后重建映射
```

#### System 热重载

```csharp
/// <summary>
/// System 注册表——支持热重载后重新发现和注册。
/// </summary>
public static class SystemRegistry
{
    /// <summary>扫描程序集中所有 ISceneSystem 实现并注册到指定世界。</summary>
    public static void RegisterAll(SceneWorld world, Assembly assembly) { ... }

    /// <summary>热重载后重新注册所有系统。</summary>
    public static void Rebuild(SceneWorld world)
    {
        // 卸载旧系统
        world.Systems.Dispose();

        // 重新扫描注册
        foreach (var assembly in AppDomain.CurrentDomain.GetAssemblies())
        {
            if (assembly.GetName().Name?.StartsWith("Neverness.Gameplay.") == true)
                RegisterAll(world, assembly);
        }
    }
}
```

#### 事件订阅重建

```csharp
/// <summary>
/// SceneWorld 内部在 Hot Reload 后自动重建 Native 事件桥接。
/// </summary>
public sealed class SceneWorld
{
    private NativeEventBridge? _nativeBridge;

    internal void RebuildAfterReload()
    {
        // 重新订阅 Native EventBus
        _nativeBridge?.Dispose();
        _nativeBridge = new NativeEventBridge(NativeHandle, Events);

        // 重新注册 System
        SystemRegistry.Rebuild(this);
    }
}
```

### 10.4 SceneWorld 状态保存/恢复

```csharp
public sealed class SceneWorld
{
    /// <summary>保存热重载状态快照（Native 状态自动保留，此方法保存 Managed 侧状态）。</summary>
    public HotReloadSnapshot SaveSnapshot()
    {
        return new HotReloadSnapshot
        {
            NativeHandle = NativeHandle,
            Name = Name,
            AssetGuid = AssetGuid,
            // Native 实体数据不需保存（Native 不受重载影响）
        };
    }

    /// <summary>从状态快照恢复。</summary>
    public static SceneWorld RestoreFromSnapshot(HotReloadSnapshot snapshot)
    {
        var world = new SceneWorld(snapshot.NativeHandle, snapshot.Name);
        world.AssetGuid = snapshot.AssetGuid;
        // 同步 Native 实体到 Managed
        SyncEntitiesFromNative(world);
        // 重建系统
        world.RebuildAfterReload();
        return world;
    }
}
```

---

## 第十一部分：最终工业级模块结构

```
Engine/Source/Managed/Runtime/Neverness.Runtime.Scene/
├── Neverness.Runtime.Scene.csproj
│
├── SceneWorld.cs                                  # 场景世界根（Gameplay Runtime Root）
├── SceneManager.cs                                # 场景管理器（降级为世界管理器）
├── SceneEntity.cs                                 # 实体句柄门面
│
├── Entities/
│   ├── EntityRegistry.cs                          # 实体注册表（handle ↔ entity 映射）
│   ├── EntityHandleExtensions.cs                  # 句柄工具（IsValid / Equals 等）
│   └── EntityQueryBridge.cs                       # Native Query ABI 桥接（新增 ABI 函数）
│
├── Queries/
│   ├── SceneView.cs                               # ref struct 单组件视图
│   ├── SceneView2.cs                              # ref struct 双组件视图
│   ├── SceneView3.cs                              # ref struct 三组件视图
│   ├── SceneQuery.cs                              # 缓存式查询对象
│   ├── SceneQueryCache.cs                         # 查询对象池
│   └── NativeQueryBridge.cs                       # Native 批量查询 ABI 封装
│
├── Systems/
│   ├── ISceneSystem.cs                            # System 标记接口
│   ├── ISystemInitialize.cs                       # Initialize 接口
│   ├── ISystemShutdown.cs                         # Shutdown 接口
│   ├── ISystemTick.cs                             # Tick 接口
│   ├── ISystemFixedTick.cs                        # FixedTick 接口
│   ├── ISystemLateTick.cs                         # LateTick 接口
│   ├── TickGroup.cs                               # Tick 分组枚举
│   ├── SystemDependencyAttribute.cs               # 依赖排序特性
│   └── SceneSystemScheduler.cs                    # System 注册/排序/调度
│
├── Components/
│   ├── ComponentIdAttribute.cs                    # [ComponentId(ulong)] 稳定标识
│   ├── ComponentIdRegistry.cs                     # 组件 ID 注册表
│   ├── ComponentMetadata.cs                       # 组件元数据
│   ├── ComponentFieldMetadata.cs                  # 字段元数据
│   └── ComponentTypeCache.cs                      # 泛型静态缓存（替代旧 Registry）
│
├── Prefabs/
│   ├── PrefabAsset.cs                             # Prefab 资产
│   ├── PrefabEntityData.cs                        # Prefab 实体描述
│   ├── PrefabInstance.cs                          # Prefab 实例
│   ├── PrefabOverride.cs                          # Override 描述
│   └── PrefabInstantiator.cs                      # 实例化器
│
├── Serialization/
│   ├── SceneAsset.cs                              # 场景资产描述（Editor JSON 兼容）
│   ├── SceneEntityEntry.cs                        # 场景实体条目
│   ├── AssetReference.cs                          # 资产引用（GUID + Path）
│   ├── AssetGuid.cs                               # GUID 工具
│   ├── NNSceneSerializeBridge.cs                  # 序列化 ABI 桥接
│   └── SceneSerializer.cs                         # C# 侧序列化协调
│
├── Reflection/
│   ├── PropertyMetadata.cs                        # 属性元数据
│   ├── SerializedAttribute.cs                     # [Serialized]
│   ├── RangeAttribute.cs                          # [Range(min, max)]
│   ├── HideInInspectorAttribute.cs                # [HideInInspector]
│   └── DisplayNameAttribute.cs                    # [DisplayName("...")]
│
└── Events/
    ├── SceneEvent.cs                              # 场景事件类型定义
    ├── SceneEventBus.cs                           # C# 事件总线
    ├── SceneEventHandler.cs                       # 事件处理器委托
    └── NativeEventBridge.cs                       # Native EventBus ↔ C# 桥接
```

---

## 第十二部分：工业级代码结构

### 12.1 核心接口清单

| 接口 | 职责 | 模块 |
|------|------|------|
| `ISceneSystem` | System 标记接口 | Systems |
| `ISystemInitialize` | 初始化钩子 | Systems |
| `ISystemShutdown` | 关闭钩子 | Systems |
| `ISystemTick` | 每帧 Tick | Systems |
| `ISystemFixedTick` | 固定步长 Tick | Systems |
| `ISystemLateTick` | Late Tick | Systems |
| `IDisposable` | 资源释放 | SceneWorld, EventBus |

### 12.2 核心类清单

| 类 | 职责 | 生命周期 |
|----|------|---------|
| `SceneWorld` | 场景世界根 | Create → Tick* → Dispose |
| `SceneManager` | 世界管理器 | 全局单例 |
| `SceneEntity` | 实体句柄门面 | 由 EntityRegistry 创建 |
| `EntityRegistry` | 实体注册表 | 由 SceneWorld 持有 |
| `SceneQuery<T>` | 缓存查询 | 由 QueryCache 管理 |
| `SceneView<T>` | 零 GC 视图 | 临时 ref struct |
| `SceneSystemScheduler` | System 调度 | 由 SceneWorld 持有 |
| `ComponentIdRegistry` | 组件 ID 映射 | 全局静态 |
| `ComponentTypeCache<T>` | 泛型缓存 | 全局静态泛型 |
| `PrefabAsset` | Prefab 资产 | 加载/创建 → 实例化 |
| `PrefabInstance` | Prefab 实例 | 由 Instantiator 创建 |
| `SceneEventBus` | C# 事件总线 | 由 SceneWorld 持有 |
| `SceneQueryCache` | 查询缓存 | 由 SceneWorld 持有 |

### 12.3 生命周期

```
┌─────────────────────────────────────────────────────────────┐
│                     Application Start                        │
│                                                              │
│  ComponentIdRegistry.ScanAllLoaded()                         │
│  SceneManager = new SceneManager()                           │
│                                                              │
├─────────────────────────────────────────────────────────────┤
│                     Scene Load                               │
│                                                              │
│  world = manager.LoadWorld("MainWorld")                      │
│    ├─ SceneNativeBridge.CreateScene() → NativeHandle         │
│    ├─ new EntityRegistry(NativeHandle)                       │
│    ├─ new SceneQueryCache(NativeHandle)                      │
│    ├─ new SceneSystemScheduler(world)                        │
│    ├─ new SceneEventBus()                                    │
│    ├─ NativeEventBridge 连接                                 │
│    └─ SystemRegistry.RegisterAll(world)                      │
│                                                              │
├─────────────────────────────────────────────────────────────┤
│                     Game Loop                                │
│                                                              │
│  while (running)                                             │
│  {                                                           │
│    world.Tick(deltaTime)                                     │
│      ├─ Systems.Tick(EarlyUpdate)                            │
│      ├─ Systems.Tick(FixedUpdate, fixedDt)                   │
│      ├─ Systems.Tick(Update)                                 │
│      ├─ SceneNativeBridge.TickSystems()                      │
│      ├─ Systems.Tick(LateUpdate)                             │
│      ├─ Systems.Tick(Render)                                 │
│      └─ Events.FlushDeferred()                               │
│  }                                                           │
│                                                              │
├─────────────────────────────────────────────────────────────┤
│                     Hot Reload                                │
│                                                              │
│  snapshot = world.SaveSnapshot()                             │
│  [Assembly Reload]                                           │
│  ComponentIdRegistry.ScanAllLoaded()                         │
│  world = SceneWorld.RestoreFromSnapshot(snapshot)            │
│    ├─ SyncEntitiesFromNative()                               │
│    ├─ NativeEventBridge.Rebuild()                            │
│    └─ SystemRegistry.Rebuild(world)                          │
│                                                              │
├─────────────────────────────────────────────────────────────┤
│                     Scene Unload                              │
│                                                              │
│  manager.UnloadWorld("MainWorld")                            │
│    ├─ world.Systems.Dispose()                                │
│    ├─ world.Events.Clear()                                   │
│    ├─ world.Queries.Clear()                                  │
│    ├─ world.Entities.Clear()                                 │
│    └─ SceneNativeBridge.DestroyScene(NativeHandle)           │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

### 12.4 数据流

```
Managed System Tick(dt)
  │
  ├─ world.Queries.GetQuery<T>()
  │    └─ NativeQueryBridge.QueryEntities(scene, typeId)
  │         └─ NNSceneApi.QueryEntities(scene, typeId, buffer, max, out count)
  │              └─ [Native] entt::view → entity handles + component data
  │
  ├─ foreach (ref var entry in view)
  │    ├─ ref entry.Component  ← Span<byte> 中的直接引用
  │    └─ entry.Component.Position.X = ...  ← 直接修改（可选写回）
  │
  └─ 写回（如需）
       └─ NativeQueryBridge.SetComponentData(scene, handle, typeId, data)
            └─ NNSceneApi.SetComponent(scene, handle, typeId, data, size)
                 └─ [Native] memcpy → entt component storage
```

### 12.5 Runtime 数据边界

```
┌──── C# Side ────────────────────────┐  ┌──── Native Side ────────────────┐
│                                      │  │                                  │
│  SceneWorld.NativeHandle (ulong) ────┼──│→ NNRuntimeScene                 │
│  EntityRegistry._handleMap ──────────┼──│→ NNEntityHandleTable            │
│  ComponentTypeCache<T>.TypeId ───────┼──│→ NNComponentRegistry            │
│  SceneSystemScheduler ───────────────┼──│→ NNSceneSystemScheduler         │
│  SceneEventBus ──────────────────────┼──│→ NNSceneEventBus                │
│  SceneQueryCache ────────────────────┼──│→ entt::view                     │
│                                      │  │                                  │
│  ┌─ 不持有 ──────────────────────┐   │  │  ┌─ 持有 ──────────────────┐    │
│  │  entt::registry               │   │  │  │  entt::registry         │    │
│  │  组件原始数据                  │   │  │  │  组件原始数据            │    │
│  │  Entity generational data     │   │  │  │  Entity generation      │    │
│  └───────────────────────────────┘   │  │  └─────────────────────────┘    │
│                                      │  │                                  │
│  ┌─ 持有 ────────────────────────┐   │  │                                  │
│  │  实体映射缓存                  │   │  │                                  │
│  │  Query 缓存                    │   │  │                                  │
│  │  System 实例                   │   │  │                                  │
│  │  事件订阅                      │   │  │                                  │
│  │  Prefab 实例映射               │   │  │                                  │
│  └───────────────────────────────┘   │  │                                  │
└──────────────────────────────────────┘  └──────────────────────────────────┘
```

### 12.6 Native ↔ Managed 边界

**现有 ABI（layoutVersion = 5）：**

| 函数 | 方向 | 说明 |
|------|------|------|
| CreateScene | M→N | 创建场景 |
| DestroyScene | M→N | 销毁场景 |
| TickSystems | M→N | 驱动 Native System |
| CreateEntity | M→N | 创建实体 |
| DestroyEntity | M→N | 销毁实体 |
| AddComponent | M→N | 添加组件 |
| RemoveComponent | M→N | 移除组件 |
| HasComponent | M→N | 查询组件 |
| GetComponent | M→N | 读取组件 |
| SetComponent | M→N | 写入组件 |
| SetParent | M→N | 设置父子 |
| GetParent | M→N | 获取父实体 |
| SerializeScene | M→N | 序列化 |
| DeserializeScene | M→N | 反序列化 |

**新增 ABI（layoutVersion = 6，下一阶段）：**

| 函数 | 方向 | 说明 |
|------|------|------|
| QueryEntities | M→N | 批量查询匹配实体句柄 |
| QueryComponents | M→N | 批量获取组件数据 |
| QueryCount | M→N | 查询匹配数量 |
| GetChildren | M→N | 获取子实体列表 |
| EmitEvent | M→N | 从 C# 发送事件到 Native EventBus |
| SubscribeEvents | M→N | 订阅 Native 事件（回调注册） |
| GetDirtyEntities | M→N | 获取脏实体列表 |

### 12.7 设计约束

| 约束 | 原因 |
|------|------|
| **不允许 switch(type)** | 所有类型通过注册表 + 接口发现 |
| **不允许 God Object** | 每个类一个职责 |
| **不允许 foreach boxing** | Query 使用 ref struct enumerator |
| **不允许 GC alloc in hot path** | Query/View/Enumerator 零 GC |
| **不允许 implicit Native state copy** | C# 不复制 ECS 数据 |
| **不允许 hardcoded menu/type** | 全部数据驱动 |
| **Runtime 不依赖 Editor** | 编译时无 Editor 引用 |

---

## 附录 A：迁移路线图

> **全部 Phase 已实施完成（2026-05-23）。**

### Phase 4-D：ComponentId 重构 ✅

1. ✅ 新增 `ComponentIdAttribute`（`Neverness.Runtime.Engine`）
2. ✅ 新增 `ComponentTypeCache<T>`（保留 `ComponentTypeRegistry<T>` 标记 `[Obsolete]`）
3. ✅ `NNTransformData` 添加 `[ComponentId(0xC1FFF4F356DFB2FB, Name = "Transform")]`
4. ✅ `SceneNativeBridge` 全部切换到 `ComponentTypeCache<T>`

### Phase 4-E：SceneWorld 提取 ✅

1. ✅ `SceneWorld`：Gameplay Runtime Root（NativeHandle + Entities + Queries + Systems + Events）
2. ✅ `EntityRegistry`：handle↔entity 双向映射 + Get/TryGet/SyncFromHandles
3. ✅ `SceneQueryCache`：GetQuery\<T\> / GetQuery\<T1,T2\> 缓存
4. ✅ `SceneManager` 降级为世界管理器：`Dictionary<string, SceneWorld>` + 委托模式
5. ✅ 编译验证 + 10 个 Scene 测试全绿

### Phase 4-F：Query 系统 ✅

1. ✅ Native 端新增 `queryEntities` / `queryComponents` / `queryCount2`（layoutVersion 5→6）
2. ✅ `SceneView<T>` / `SceneView<T1,T2>`：ref struct 零 GC 视图
3. ✅ `SceneQuery<T>` / `SceneQuery<T1,T2>`：缓存式查询对象
4. ✅ `NativeQueryBridge`：批量查询 ABI 封装
5. ⏳ 性能基准测试（待后续专项）

### Phase 4-G：System 框架 ✅

1. ✅ `ISceneSystem` + `ISystemInitialize` / `ISystemShutdown` / `ISystemTick` / `ISystemFixedTick` / `ISystemLateTick`
2. ✅ `SceneSystemScheduler`：Kahn 拓扑排序 + 按 TickGroup 分组 + 延迟 Initialize + Rebuild
3. ✅ `SystemDependencyAttribute`：`[SystemDependency(typeof(X))]` 依赖声明
4. ✅ `SceneWorld.Tick()` 完整 Tick 流（EarlyUpdate → Update → Native TickSystems → LateUpdate → Render → FlushDeferred）

### Phase 4-H：Prefab 重设计 ✅

1. ✅ `PrefabAsset`（Guid/Name/Entities + `FromEntity()` BFS 构建）/ `PrefabEntityData`（LocalIndex/ParentIndex/Components）
2. ✅ `PrefabInstance`（Source/RootEntity/InstanceMap/Overrides）/ `PrefabOverride`（5 种覆盖类型枚举）
3. ✅ `PrefabInstantiator`：Instantiate（创建实体 + 写入组件 + 建立层级 + 回滚）、ApplyOverrides、RevertToPrefab、DetectDifferences
4. ✅ 嵌套 Prefab 支持：ChildAdded（创建子实体 + 挂载 + 写入组件）、ChildRemoved（销毁子实体）
5. ✅ Override / Revert 机制：全部 5 种覆盖类型均支持 Apply 和 Revert

### Phase 4-I：事件 + Hot Reload ✅

1. ✅ `SceneEventBus`：Subscribe/Unsubscribe/SubscribeAll/Emit/EmitDeferred/FlushDeferred/Clear，递归 Emit 自动降级为 deferred
2. ✅ `NativeEventBridge`：持有 SceneEventBus + Native 回调委托 + EnableAutoBridge + InjectEvent（当前为 stub，待 NNSceneApi ABI layoutVersion 7 新增事件订阅函数后启用自动桥接）
3. ✅ `HotReloadSnapshot` + `GlobalHotReloadSnapshot`：`SceneWorld.SaveSnapshot` / `RestoreFromSnapshot` + `SceneManager.SaveAllSnapshots` / `RestoreAllSnapshots` / `RebuildAllAfterReload`
4. ✅ `SceneSystemScheduler.Rebuild()`：关闭所有 System 并清空注册（热重载后新程序集重新注册）

---

## 附录 B：csproj 依赖声明

```xml
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <AssemblyName>Neverness.Runtime.Scene</AssemblyName>
    <TargetFramework>net10.0</TargetFramework>
    <Nullable>enable</Nullable>
    <ImplicitUsings>enable</ImplicitUsings>
    <AllowUnsafeBlocks>true</AllowUnsafeBlocks>
  </PropertyGroup>
  <ItemGroup>
    <ProjectReference Include="..\Neverness.Runtime.Engine\Neverness.Runtime.Engine.csproj" />
    <ProjectReference Include="..\Neverness.Runtime.Interop\Neverness.Runtime.Interop.csproj" />
  </ItemGroup>
  <!-- 不引用任何 Editor 模块 -->
</Project>
```

---

## 附录 C：与现有代码的向后兼容

| 现有代码 | 新架构中的位置 | 兼容策略 |
|----------|---------------|---------|
| `SceneManager` | 降级为世界管理器 | 保留类名，内部委托给 `SceneWorld` |
| `SceneEntity` | 实体门面 | 保留类名，增加 `World` 引用 |
| `Prefab` | 便捷 API 包装 | 保留类名，内部委托 `PrefabAsset` |
| `SceneNativeBridge` | ABI 层 | 保留不变，扩展 Query 函数 |
| `ComponentTypeRegistry<T>` | 废弃 | 迁移到 `ComponentTypeCache<T>` |
| `NNSceneSerializeBridge` | 迁移到 Serialization/ | 保留不变 |

---

*文档结束。*
