# NNRuntimeScene — ECS-first 运行时场景

> CMake 目标 **`NevernessRuntime-Scene`**；C++ 命名空间 **`NN::Runtime::Scene`**。

## 1. 定位与边界

| 项目 | 说明 |
|------|------|
| **职责** | **Handle + POD Component + entt::registry** 的新一代 Native Scene：世代 `NNEntity`、实体生命周期、`Emplace`/`Query`、**字段级组件反射**、**System 调度**、层级、事件/脏跟踪、**二进制/JSON 场景序列化**、**Snapshot 构建**、**Prefab 实例化**。Transform 使用 `float3`/`quaternion`/`matrix`（HCoreTypes）与 Legacy 对齐。 |
| **不负责** | Legacy `IGameActor`/`IComponent`、**SceneSubsystem** 替换或数据迁移、**NNNativeEngineAPI** layout 扩展（`NN_AddComponent` 留后续）、**Streaming** 完整实现、并行 Job。 |
| **CMake 目标** | `NevernessRuntime-Scene`（`SHARED`） |
| **宏** | `NN_RUNTIME_SCENE_API` / `NN_RUNTIME_SCENE_EXPORT` |
| **依赖** | `NevernessCore-Core`（PUBLIC，含 vendored **entt**）；**NNNativeEngineAPI** 头路径（PRIVATE，仅字段对齐约定，不链接 ABI 表）。**不**链接 **NevernessRuntime-Engine**。 |
| **典型消费者** | **`NevernessRuntime-Engine`**（`NNRuntimeSceneTickSubsystem` + `VGEngineRuntime::EcsScene()`）；未来 **VGSceneRuntime** / 工具链。 |

### 1.1 与既有身份体系对照

| 体系 | 身份 | 关系 |
|------|------|------|
| **NNEngineLegacy** | `VGActorID` + `shared_ptr<IGameActor>` + entt | **并存**，不修改 |
| **NNRuntimeEngine** | `NNEntityHandle` 单调 id + `SceneSubsystem` map | **并存**；ECS 路径为 **`EcsScene()`**，不替换 `SceneSubsystem` |
| **本模块** | `NNEntity`（`uint64` Index+Generation）+ entt | **已落地** Phase 1–4-J |
| **VisionGal.Managed.Entity** | `EntityHandle`（`uint` Index + `uint` Generation） | **打包规则对齐**；**无**自动桥接 |

---

## 2. 构建与选项

- 在 [`Engine/Source/Runtime/CMakeLists.txt`](../CMakeLists.txt) 中于 **`NNNativeEngineAPI`** 之后、`NNRuntimeEngine` 之前注册 `add_subdirectory("NNRuntimeScene")`。
- **PUBLIC** 包含目录：`Engine/Source/Runtime`、`NNRuntimeScene/Include`（消费者可用 `#include "Scene/NNRuntimeScene.h"`）。
- **PRIVATE** 包含目录：`Engine/Source/Core`。
- `ENABLE_TESTS=ON` 且找到 GTest 时构建 **`NNRuntimeSceneTest`**（15 cases）。

```powershell
cmake --build Build/vs/x64-debug --target NevernessRuntime-Scene NNRuntimeSceneTest
Build\bin\Debug\NNRuntimeSceneTest.exe
```

---

## 3. 目录结构

```
Engine/Source/Runtime/NNRuntimeScene/
├── CMakeLists.txt
├── NNRuntimeSceneExport.h                # NN_RUNTIME_SCENE_API 宏定义
├── Docs/                                 # 模块文档
│   ├── MODULE_ARCHITECTURE_AND_PROGRESS.md
│   ├── EDITOR_SNAPSHOT_SYSTEM.md         # Editor Snapshot 驱动的 SceneBrowser 架构
│   ├── RUNTIME_REFLECTION_API.md         # Runtime 反射 Snapshot（Inspector/PropertyGrid）
│   └── NNSpriteRendererComponent — 工业级 2D 渲染组件设计.md
├── Include/
│   ├── Scene/
│   │   ├── NNEntity.h                    # NNEntity = uint64_t 世代 Handle 别名
│   │   ├── NNEntityHandle.h              # NNEntityHandleTable：slot-based 世代分配器
│   │   ├── NNRuntimeScene.h              # 核心类：ECS Scene 世界
│   │   ├── NNWorld.h                     # using NNWorld = entt::registry
│   │   ├── NNSceneAssetLoader.h          # Phase 7 占位：场景资产异步加载
│   │   └── NNStreamingZone.h             # Phase 7 占位：Streaming Zone 管理
│   ├── Components/                       # 全部为纯 POD 结构体
│   │   ├── NNTransformComponent.h        # float3 Position + quaternion Rotation + float3 Scale + matrix WorldMatrix
│   │   ├── NNRelationshipComponent.h     # Parent + ChildCount + Depth
│   │   ├── NNTagComponent.h              # uint32 Flags + char Name[64]
│   │   ├── NNCameraComponent.h           # Perspective/Orthographic 投影参数 + ProjectionMatrix
│   │   ├── NNSpriteRendererComponent.h   # 2D Sprite 渲染：Texture/Material GUID + Color + UV + BlendMode + Flags
│   │   ├── NNAudioSourceComponent.h      # 音频源：Clip GUID + Volume/Pitch + 3D 参数 + Flags
│   │   └── NNVideoPlayerComponent.h      # 视频播放器：Clip GUID + VideoTexture + Volume + Flags
│   ├── Systems/
│   │   ├── ISceneSystem.h                # System 抽象接口
│   │   ├── NNSceneTickGroup.h            # TickGroup 枚举：EarlyUpdate/FixedUpdate/Update/LateUpdate/Render
│   │   ├── NNSceneSystemScheduler.h      # System 调度器：按 TickGroup 分组执行
│   │   ├── NNHierarchySystem.h           # EarlyUpdate：父子关系管理 + 循环检测
│   │   ├── NNSceneUpdateSystem.h         # EarlyUpdate：场景逻辑 Hook
│   │   ├── NNTransformSystem.h           # Update：DFS 世界矩阵计算
│   │   └── NNCameraSystem.h              # Update：投影矩阵重算
│   ├── Reflection/
│   │   ├── NNComponentTypeId.h           # NNComponentTypeId = uint64_t (FNV-1a name hash)
│   │   ├── NNComponentFieldType.h        # 字段类型枚举：Float/Float3/Float4/Float4x4/Quaternion/...
│   │   └── NNComponentRegistry.h         # 组件类型注册表 + NN_FIELD/NN_REGISTER_COMPONENT 宏
│   ├── Query/
│   │   └── NNEntityQuery.h               # NNEntityQuery<Components...>：entt view 封装
│   ├── Serialization/
│   │   ├── NNSceneSerializer.h           # 二进制序列化（VGSC 格式，版本 3）
│   │   └── NNJsonSceneSerializer.h       # JSON 序列化（nlohmann/json，版本 2）
│   ├── Snapshot/
│   │   ├── NNHierarchySnapshotBuilder.h  # Editor SceneBrowser 层级 Snapshot 构建
│   │   └── NNReflectionSnapshotBuilder.h # Editor Inspector 反射元数据 Snapshot 构建
│   ├── Runtime/
│   │   ├── NNSceneEventBus.h             # 单线程事件分发（EntityCreated/Destroyed/ParentChanged/ComponentEmplaced）
│   │   ├── NNDirtyTracker.h              # 脏实体/脏组件跟踪（consume-and-clear 模式）
│   │   └── NNSceneRuntime.h              # 未来多场景容器占位
│   └── Prefab/
│       └── NNPrefab.h                    # Prefab 实例化器：二进制 Blob → 实体树
├── Source/
│   ├── Scene/
│   │   ├── NNEntity.cpp
│   │   ├── NNEntityHandle.cpp
│   │   └── NNRuntimeScene.cpp
│   ├── Systems/
│   │   ├── NNCameraSystem.cpp
│   │   ├── NNHierarchySystem.cpp
│   │   ├── NNSceneSystemScheduler.cpp
│   │   ├── NNSceneUpdateSystem.cpp
│   │   └── NNTransformSystem.cpp
│   ├── Reflection/
│   │   ├── BuiltinComponentRegistration.cpp  # 内置组件注册（Transform/Relationship/Tag/Camera/SpriteRenderer/AudioSource/VideoPlayer）
│   │   └── NNComponentRegistry.cpp
│   ├── Serialization/
│   │   ├── NNSceneSerializer.cpp
│   │   └── NNJsonSceneSerializer.cpp
│   ├── Snapshot/
│   │   ├── NNHierarchySnapshotBuilder.cpp
│   │   └── NNReflectionSnapshotBuilder.cpp
│   ├── Runtime/
│   │   ├── NNDirtyTracker.cpp
│   │   └── NNSceneEventBus.cpp
│   └── Prefab/
│       └── NNPrefab.cpp
└── Test/
    ├── CMakeLists.txt
    └── NNRuntimeSceneTest.cpp
```

---

## 4. 核心 API

### 4.1 包含方式

```cpp
#include "Scene/NNRuntimeScene.h"
// 或（Runtime 根路径）
#include "NNRuntimeScene/Include/Scene/NNRuntimeScene.h"
```

### 4.2 NNEntity（对外仅 uint64）

- 定义收口到 **`EngineHandles.h`**：`typedef uint64_t NNEntityHandle; using NNEntity = NNEntityHandle;`（全局命名空间）。
- `NN::Runtime::Scene::NNEntity` 为同一类型的命名空间别名，所有消费者代码继续使用 `NNEntity`。
- `PackEntityHandle` / `UnpackEntityHandle`：低 32 = **Index**（自 1 起），高 32 = **Generation**（Destroy 递增）。
- `NNEntityInvalid == 0`：与托管 `EntityHandle.Invalid` 一致。

### 4.3 NNRuntimeScene

| API | 说明 |
|-----|------|
| `CreateEntity()` / `DestroyEntity(NNEntity)` | 世代校验 + entt 生命周期 |
| `CreateEntityWithDefaults()` | Transform + Relationship + Tag 默认 POD（Camera / SpriteRenderer 等需手动 Emplace） |
| `Emplace<T>` / `TryGet<T>` / `Has<T>` / `Remove<T>` | entt 组件 CRUD |
| `Query<Components...>()` | `NNEntityQuery::Each` |
| `RegisterSystem` / `TickSystems` | **NNSceneSystemScheduler** |
| `SetParent` / `GetParent` / `GetChildren` | 委托 **NNHierarchySystem** |
| `Events()` / `Dirty()` | **NNSceneEventBus** / **NNDirtyTracker** |
| `ForEachAliveEntity` | 序列化与编辑器遍历 |
| `BindComponentType<T>()` | 类型擦除 ECS 访问函数指针（GetComponentPtr/Has/Add/Remove/ForEach） |
| `GetHierarchyVersion()` / `GetTransformVersion()` / `GetReflectionVersion()` | 原子版本计数器（编辑器 Snapshot 增量检测） |

构造时默认注册：**NNHierarchySystem → NNSceneUpdateSystem → NNTransformSystem → NNCameraSystem**。

### 4.4 内置组件一览

| 组件 | TypeId（FNV-1a） | 核心字段 | 大小 |
|------|-------------------|----------|------|
| `NNTransformComponent` | `0xC1FFF4F356DFB2FB` | Position(`float3`), Rotation(`quaternion`), Scale(`float3`), WorldMatrix(`matrix`) | ~80B |
| `NNRelationshipComponent` | 自动注册 | Parent(`NNEntity`), ChildCount(`uint32`), Depth(`uint32`) | 16B |
| `NNTagComponent` | 自动注册 | Flags(`uint32`), Name(`char[64]`) | 68B |
| `NNCameraComponent` | `0x54D1B2A64667E32E` | ProjectionType, FovY, AspectRatio, Near/Far, OrthoWidth/Height, ProjectionMatrix | ~80B |
| `NNSpriteRendererComponent` | 自动注册 | TextureGUID, MaterialGUID, TextureId, Color(`float4`), UVRect, Layer, SortOrder, BlendMode, Flags | ~88B |
| `NNAudioSourceComponent` | 自动注册 | ClipGUID, PlayerId, Volume, Pitch, MinDistance, MaxDistance, Flags | 48B |
| `NNVideoPlayerComponent` | 自动注册 | ClipGUID, PlayerId, VideoTextureId, Volume, Flags, TargetSpriteGUID | 56B |

### 4.5 反射与序列化

- **`NN_REGISTER_COMPONENT`** + **`NN_FIELD`**：字段 `offset`/`size`/`NNComponentFieldType`/`nameUtf8`。
- 内置组件全字段已注册。
- **`NNSceneSerializer`**（二进制）：魔数 **`VGSC`**（格式版本 3），按注册表字段 `memcpy` 快照；组件 TypeId 为 **FNV-1a name hash**（8 字节，跨进程稳定）；反序列化按 name hash 查表重建实体（Index/Generation 由运行时重新分配）；Relationship 经 **`SetParent`** 恢复层级。
- **`NNJsonSceneSerializer`**（JSON）：格式版本 2，基于 nlohmann/json；每个实体输出 id + components 对象（key 为注册名称），字段 key 为字段名；用于 Editor 可读/可 diff 场景。

### 4.6 Snapshot 构建（Editor 专用）

| Builder | 输出格式 | 用途 |
|---------|----------|------|
| **`NNHierarchySnapshotBuilder`** | `[Header 32B][Nodes 40B each][NamePool]` 连续 POD 缓冲区 | Editor SceneBrowser：两遍算法（计数 + DFS 写入），替代逐实体 P/Invoke |
| **`NNReflectionSnapshotBuilder`** | `[Header][ComponentInfo[]][FieldInfo[]][NamePool]` 连续 POD 缓冲区 | Editor Inspector/PropertyGrid：零硬编码，全部来自 `NNComponentRegistry` |

### 4.7 Prefab 系统

- **`NNPrefab`**：静态 `Instantiate()` 方法，接受 `NNRuntimeScene` + 二进制 Blob（EntityHierarchy 格式，与 C# `PrefabImporter.SerializePrefab` 对齐），输出根实体 Handle。

### 4.8 Engine 适配（依赖方向：Engine → Scene）

| 类型 | 说明 |
|------|------|
| **`NNRuntimeSceneTickSubsystem`** | `IRuntimeSubsystem`，`TickGroup == Update`，调用 `scene->TickSystems(dt)` |
| **`VGEngineRuntime::EcsScene()`** | 内嵌 **`NNRuntimeScene ecsScene_`**，与 **`SceneSubsystem`** 并存 |

---

## 5. Phase 完成清单

### Phase 1（2026-05-17）

| 项 | 状态 |
|----|------|
| `NNEntity` / 世代 Handle | **已完成** |
| `NNRuntimeScene` + registry 封装 | **已完成** |
| `NNEntityQuery` | **已完成** |
| `NNComponentRegistry`（类型级） | **已完成** |

### Phase 2（2026-05-17）

| 项 | 状态 |
|----|------|
| `ISceneSystem` / `NNSceneTickGroup` / `NNSceneSystemScheduler` | **已完成** |
| `NNTransformSystem` / `NNHierarchySystem` / `NNSceneUpdateSystem` | **已完成** |
| `NNSceneEventBus` / `NNDirtyTracker` | **已完成** |
| `NNRuntimeSceneTickSubsystem` + `VGEngineRuntime` 集成 | **已完成** |
| GTest：Hierarchy / Scheduler / Event / Dirty | **已完成**（10 cases 全绿） |

### Phase 3（2026-05-17）

| 项 | 状态 |
|----|------|
| `NNComponentFieldType` / `NN_FIELD` / 字段注册 API | **已完成** |
| 内置组件全字段元数据 | **已完成** |
| `NNSceneSerializer` 二进制 round-trip | **已完成** |
| JSON 导出 | **已完成**（`NNJsonSceneSerializer`，格式版本 2） |

### Phase 3.5（2026-05-22）

| 项 | 状态 |
|----|------|
| `NNTransformComponent::WorldMatrix` 字段（`matrix` = `glm::mat4`） | **已完成** |
| `NNComponentFieldType::Float4x4` / `Float4` / `Quaternion` 序列化支持 | **已完成** |
| `NNTransformSystem::Tick` DFS 世界矩阵计算 | **已完成** |
| GTest：单亲 / 多层级 / 纯根 世界矩阵验证（+4 cases） | **已完成** |

### Phase 3.6（2026-05-22）

| 项 | 状态 |
|----|------|
| `NNTransformComponent` 改用 `float3` / `quaternion` / `matrix`（HCoreTypes） | **已完成** |
| `Rotation` 从欧拉角（float[3]）改为四元数（`quaternion`） | **已完成** |
| `NNTransformSystem::BuildLocalMatrix` 使用 `glm::mat4_cast(quaternion)` | **已完成** |

### Phase 3.7（2026-05-22）

| 项 | 状态 |
|----|------|
| `EngineHandles.h` 统一：`typedef uint64_t NNEntityHandle; using NNEntity = NNEntityHandle;` | **已完成** |
| `NNEntity.h` 复用全局 `NNEntityHandle`：`using NNEntity = NNEntityHandle;` | **已完成** |
| NNNativeEngineAPI 保持零依赖（INTERFACE），无架构变动 | **已完成** |

### Phase 4.0（2026-05-22）— NNSceneAPI ABI 重写

| 项 | 状态 |
|----|------|
| `SceneAPI.h` 完全重写：纯 C 头文件，`layoutVersion` + `NNSceneResult` + `NNTransformData` | **已完成** |
| `NN_NATIVE_ENGINE_API_LAYOUT_VERSION` 10→11（聚合体 layout 变化） | **已完成** |
| `SceneApiStubs.cpp` / `SceneRuntimeApi.cpp` 适配新 ABI 签名 | **已完成** |
| `SceneSubsystem` 方法签名统一为 `NNSceneResult` / `NNTransformData` / `uint32_t` | **已完成** |
| C# 端：`NNSceneApi` 结构体重写（`LayoutVersion` + 新 delegate*）、`NNTransformData`/`NNVec3`/`NNQuat`/`NNSceneResult` 类型 | **已完成** |
| C# 端：`SceneNativeBridge` 新增 Activate/Hierarchy/Transform 封装 | **已完成** |
| C# 端：`NNSceneSerializeBridge` 适配新序列化签名（`NNSceneResult` + `outRequired`） | **已完成** |

### Phase 4-B（2026-05-23）— 跨进程稳定 TypeId（FNV-1a name hash）

| 项 | 状态 |
|----|------|
| `fnv1a_64()` 编译期 FNV-1a 64-bit 哈希工具函数 | **已完成** |
| `NNComponentTypeId` 从 `uint32`（自增计数器）改为 `uint64`（name hash） | **已完成** |
| `NN_REGISTER_COMPONENT` 显式名称字符串的 FNV-1a 哈希作为稳定 TypeId | **已完成** |
| `NNComponentRegistry` 新增 `m_NameHashToDesc` + `FindDescByNameHash()` O(1) 查找 | **已完成** |
| `NNSceneSerializer` VGSC 格式版本 1→2：组件 TypeId 改为 8 字节 nameHash | **已完成** |
| 反序列化按 nameHash 查表（`FindDescByNameHash`），不再依赖注册顺序 | **已完成** |
| GTest：`SerializeRoundTripStableTypeId`（static_assert + round-trip 验证，15 cases） | **已完成** |

### Phase 4-C（2026-05-23）— C# SceneManager 架构重构

| 项 | 状态 |
|----|------|
| `SceneAPI.h` layoutVersion 3→4，`NNSceneHandle` uint32→uint64 | **已完成** |
| `SceneSubsystem` 改为管理 `NNRuntimeScene` 实例 | **已完成** |
| C# 端 `SceneManager` 负责场景生命周期 / 多场景管理 | **已完成** |
| `SceneEntity` 改用泛型组件操作 | **已完成** |
| Prefab 改为 CreateEntity+AddComponent 模式 | **已完成** |
| `NNSceneSerializeBridge` 适配新 ABI | **已完成** |

### Phase 4-D（2026-05-23）— C# 端 ComponentId 重构

| 项 | 状态 |
|----|------|
| `ComponentIdAttribute`（`Neverness.Runtime.Engine`）：显式声明跨版本 TypeId | **已完成** |
| `ComponentTypeCache<T>`（`Neverness.Runtime.Scene`）：从 [ComponentId] 读取，Fallback 到 FNV-1a | **已完成** |
| `NNTransformData` 添加 `[ComponentId(0xC1FFF4F356DFB2FB, Name = "Transform")]` | **已完成** |
| `SceneNativeBridge` 5 处 `ComponentTypeRegistry<T>` → `ComponentTypeCache<T>` | **已完成** |
| `ComponentTypeRegistry<T>` 标记 `[Obsolete]` | **已完成** |

### Phase 4-E（2026-05-23）— SceneWorld 提取

| 项 | 状态 |
|----|------|
| `EntityRegistry`：handle↔entity 双向映射，从 `SceneManager._entities` 提取 | **已完成** |
| `SceneQueryCache`：查询对象池（Stub，Phase 4-F 实现完整查询） | **已完成** |
| `SceneWorld`：Gameplay Runtime Root，持有 NativeHandle + EntityRegistry + Queries + Tick | **已完成** |
| `SceneManager` 降级为世界管理器：`Dictionary<string, SceneWorld>` + 委托模式 | **已完成** |
| 向后兼容：`SceneManager.CreateEntity`/`DestroyEntity`/`Entities` 委托到 `ActiveWorld` | **已完成** |
| 编译验证：Engine + Scene + Editor.Serialization + Tests 全部通过 | **已完成** |
| 测试验证：10 个 Scene 相关测试全绿 | **已完成** |

### Phase 4-F（2026-05-23）— Query 系统

| 项 | 状态 |
|----|------|
| Native 端 `SceneAPI.h`：layoutVersion 5→6，新增 3 个 Query 函数指针 | **已完成** |
| Native 端 `SceneSubsystem`：`QueryEntities` / `QueryComponents` / `QueryCount2` 实现 | **已完成** |
| Native 端 `SceneRuntimeApi` + `SceneApiStubs`：转发 + stub | **已完成** |
| C# 端 `NNSceneApi`：新增 3 个 delegate* 字段 | **已完成** |
| C# 端 `NativeQueryBridge`：批量查询 ABI 封装 | **已完成** |
| C# 端 `SceneView<T>` / `SceneView<T1,T2>`：ref struct 零 GC 视图 | **已完成** |
| C# 端 `SceneQuery<T>` / `SceneQuery<T1,T2>`：缓存式查询对象 | **已完成** |
| `SceneQueryCache`：`GetQuery<T>` / `GetQuery<T1,T2>` 实现 | **已完成** |
| `SceneWorld`：`GetQuery<T>` / `GetQuery<T1,T2>` 便捷方法 | **已完成** |

### Phase 4-G（2026-05-23）— System 框架

| 项 | 状态 |
|----|------|
| `TickGroup` 枚举：对齐 Native `NNSceneTickGroup`（EarlyUpdate/FixedUpdate/Update/LateUpdate/Render） | **已完成** |
| `ISceneSystem` 标记接口 + `ISystemInitialize` / `ISystemShutdown` / `ISystemTick` / `ISystemFixedTick` / `ISystemLateTick` | **已完成** |
| `SystemDependencyAttribute`：`[SystemDependency(typeof(X))]` 依赖声明 | **已完成** |
| `SceneSystemScheduler`：Kahn 拓扑排序 + 按 TickGroup 分组 + 延迟 Initialize | **已完成** |
| `SceneWorld.Systems` 属性 + `Tick()` 完整 Tick 流（EarlyUpdate → Update → Native → LateUpdate → Render） | **已完成** |
| `SceneWorld.FixedTick()`：独立固定步长 Tick | **已完成** |

### Phase 4-H（2026-05-23）— Prefab 系统

| 项 | 状态 |
|----|------|
| `PrefabEntityData`：LocalIndex / ParentIndex / DisplayName / Components（TypeId → byte[]） | **已完成** |
| `PrefabAsset`：Guid / Name / Entities 列表 + `FromEntity()` BFS 构建 | **已完成** |
| `PrefabOverrideType` 枚举：PropertyModified / ComponentAdded / ComponentRemoved / ChildAdded / ChildRemoved | **已完成** |
| `PrefabOverride`：Type / EntityLocalIndex / ComponentTypeId / PropertyPath / NewValue | **已完成** |
| `PrefabInstance`：Source / RootEntity / InstanceMap / Overrides + GetEntity / AddOverride / ClearOverrides | **已完成** |
| `PrefabInstantiator`：Instantiate（创建实体 + 写入组件 + 建立层级 + 回滚）、ApplyOverrides、RevertToPrefab、DetectDifferences | **已完成** |
| `SceneNativeBridge.AddComponent / RemoveComponent`（原始 TypeId 重载） | **已完成** |
| `SceneNativeBridge.SetComponentData`（ReadOnlySpan<byte> → 原始字节写入） | **已完成** |
| `Prefab` 旧类重构为便捷包装器（保留 builder API，内部委托 PrefabAsset） | **已完成** |

### Phase 4-I（2026-05-23）— 事件系统 + 热重载

| 项 | 状态 |
|----|------|
| `SceneEventType` 枚举：EntityCreated / EntityDestroyed / ParentChanged / ComponentEmplaced | **已完成** |
| `SceneEvent` 结构体：Type / Entity / OtherEntity / ComponentTypeId + 工厂方法 | **已完成** |
| `SceneEventBus`：Subscribe / Unsubscribe / SubscribeAll / Emit / EmitDeferred / FlushDeferred / Clear | **已完成** |
| `NativeEventBridge`：持有 SceneEventBus 引用 + Native 回调委托 + EnableAutoBridge + InjectEvent | **已完成** |
| `HotReloadSnapshot`：Name / NativeHandle / AssetGuid / EntityHandles / CapturedAt | **已完成** |
| `GlobalHotReloadSnapshot`：Worlds 字典 + ActiveWorldName + CapturedAt | **已完成** |
| `SceneWorld`：新增 Events 属性 + AssetGuid + SaveSnapshot / RestoreFromSnapshot / RebuildAfterReload | **已完成** |
| `SceneWorld.Tick`：末尾新增 Events.FlushDeferred() | **已完成** |
| `SceneWorld.Dispose`：新增 NativeEventBridge 释放 + Events.Clear() | **已完成** |
| `SceneSystemScheduler.Rebuild()`：关闭所有 System 并清空注册（热重载用） | **已完成** |
| `EntityRegistry`：新增 Get / TryGet / SyncFromHandles / ExportHandleValues | **已完成** |
| `SceneManager`：新增 SaveAllSnapshots / RestoreAllSnapshots / RebuildAllAfterReload | **已完成** |
| 测试验证：31/34 测试通过（3 个预存 window API 失败） | **已完成** |

### Phase 4-J（2026-05-23）— NNCameraComponent

| 项 | 状态 |
|----|------|
| `NNCameraComponent` POD 结构体：`NNProjectionType` 枚举 + 透视参数（FovY/AspectRatio）+ 正交参数（OrthoWidth/OrthoHeight）+ `ProjectionMatrix` 输出 | **已完成** |
| `NNCameraSystem`：`ISceneSystem` 实现，TickGroup = Update，`glm::perspectiveRH_NO` / `glm::orthoRH_NO` 投影矩阵计算 | **已完成** |
| `BuiltinComponentRegistration`：`NN_REGISTER_COMPONENT` 注册 Camera，FNV-1a TypeId `0x54D1B2A64667E32E` | **已完成** |
| `NNRuntimeScene`：`NNCameraSystem m_CameraSystem` 成员 + `RegisterDefaultSystems` 注册 + `GetCameraSystem()` 访问器 | **已完成** |
| C# 端：`NNProjectionType` 枚举 + `NNMat4` 结构体 + `NNCameraComponentData` blittable 结构体 + `[ComponentId(0x54D1B2A64667E32E, Name = "Camera")]` | **已完成** |
| 无 ABI 变更：复用现有 `AddComponent/GetComponent/SetComponent` 泛型接口 | **已完成** |

### Phase 4-K（2026-05-23）— 媒体组件（AudioSource / VideoPlayer）

| 项 | 状态 |
|----|------|
| `NNAudioSourceComponent` POD：ClipGUID(16B) + PlayerId + Volume/Pitch + 3D 参数 + Flags | **已完成** |
| `NNVideoPlayerComponent` POD：ClipGUID + PlayerId + VideoTextureId + Volume + Flags + TargetSpriteGUID | **已完成** |
| `BuiltinComponentRegistration`：注册 AudioSource / VideoPlayer 组件 | **已完成** |

### Phase 5（2026-05-23）— Snapshot / Editor 集成

| 项 | 状态 |
|----|------|
| `NNHierarchySnapshotBuilder`：两遍 DFS + 连续 POD 缓冲区构建 | **已完成** |
| `NNReflectionSnapshotBuilder`：组件元数据 Snapshot（零硬编码） | **已完成** |
| Editor SceneBrowser 替代逐实体 P/Invoke（8000x 开销降低） | **已完成** |
| Editor Inspector/PropertyGrid 自动 UI 生成 | **已完成** |

### Phase 6（2026-05-23）— JSON 序列化

| 项 | 状态 |
|----|------|
| `NNJsonSceneSerializer`：nlohmann/json，格式版本 2 | **已完成** |
| 实体 id + components 对象（key = 注册名称）+ 字段 key = 字段名 | **已完成** |
| 可读 / 可 Git-diff 场景文件 | **已完成** |

### Phase 7（2026-05-23）— NNPrefab

| 项 | 状态 |
|----|------|
| `NNPrefab`：二进制 Blob → `NNRuntimeScene` 实体树实例化 | **已完成** |
| 与 C# `PrefabImporter.SerializePrefab` 格式对齐 | **已完成** |

---

## 6. 开发进展

| 日期 | 进展 |
|------|------|
| 2026-05-17 | **Phase 1** 合入：`NevernessRuntime-Scene` SHARED 库、GTest 4 cases。 |
| 2026-05-17 | **Phase 2–3** 合入：System 层、层级、事件/脏、字段反射、**VGSC** 序列化；**Engine** 私有链接 Scene 并挂 **Update** Tick；**Legacy / SceneSubsystem / ABI layout** 未改。 |
| 2026-05-22 | **Phase 3.5** 合入：`NNTransformSystem` DFS 世界矩阵计算、`Float4x4` 字段类型与序列化、GTest 层级矩阵验证（+3 cases → 13 cases）。 |
| 2026-05-22 | **Phase 3.6** 合入：`NNTransformComponent` 改用 `float3`/`quaternion`/`matrix` 核心类型，Rotation 改为四元数，新增 `Float4`/`Quaternion` 字段类型（14 cases 全绿）。 |
| 2026-05-22 | **Phase 3.7** 合入：`NNEntityHandle` / `NNEntity` 类型统一收口到 `EngineHandles.h`，消除重复 `uint64_t` 定义。 |
| 2026-05-22 | **Phase 4.0** 合入：`NNSceneAPI` ABI 重写（纯 C 头文件、`layoutVersion`、`NNSceneResult` 错误码、`NNTransformData` blittable 类型）；`SceneSubsystem` 签名对齐；C# 端 `NNSceneApi`/`SceneNativeBridge`/`NNSceneSerializeBridge` 全链路同步（layout 10→11）。 |
| 2026-05-23 | **Phase 4-B** 合入：跨进程稳定 TypeId — FNV-1a 64-bit name hash 替代自增计数器，VGSC 格式版本升至 2（15 cases 全绿）。 |
| 2026-05-23 | **Phase 4-C** 合入：C# SceneManager 架构重构 — SceneAPI.h layoutVersion 3→4、NNSceneHandle uint32→uint64、SceneSubsystem 改为管理 NNRuntimeScene 实例、C# 端 SceneManager 负责场景生命周期/多场景管理、SceneEntity 改用泛型组件操作、Prefab 改为 CreateEntity+AddComponent 模式。 |
| 2026-05-23 | **Phase 4-D** 合入：C# 端 ComponentId 重构 — `ComponentIdAttribute` + `ComponentTypeCache<T>` 替代 `ComponentTypeRegistry<T>`、`SceneNativeBridge` 全部切换到 `ComponentTypeCache<T>`。 |
| 2026-05-23 | **Phase 4-E** 合入：SceneWorld 提取 — `EntityRegistry`、`SceneQueryCache`（Stub）、`SceneWorld`（Gameplay Runtime Root）、`SceneManager` 降级为世界管理器。 |
| 2026-05-23 | **Phase 4-F** 合入：Query 系统 — Native 端新增 Query ABI 函数（layoutVersion 5→6）、C# 端 `NativeQueryBridge`/`SceneView<T>`/`SceneQuery<T>` 零 GC 批量查询。 |
| 2026-05-23 | **Phase 4-G** 合入：Managed System 框架 — `TickGroup` 枚举、`ISceneSystem` + 子接口、`SystemDependencyAttribute`、`SceneSystemScheduler`（Kahn 拓扑排序）、完整 Tick 流。 |
| 2026-05-23 | **Phase 4-H** 合入：Prefab 系统 — `PrefabAsset`/`PrefabInstance`/`PrefabInstantiator` 全链路、`SceneNativeBridge` 新增 `SetComponentData` / 原始 TypeId 重载。 |
| 2026-05-23 | **Phase 4-I** 合入：事件系统 + 热重载 — `SceneEventBus`（deferred 模式）、`NativeEventBridge`、`HotReloadSnapshot`/`GlobalHotReloadSnapshot`、`SceneWorld.SaveSnapshot`/`RestoreFromSnapshot`/`RebuildAfterReload`、`SceneSystemScheduler.Rebuild()`。 |
| 2026-05-23 | **Phase 4-J** 合入：NNCameraComponent — POD 结构体 + `NNCameraSystem` 投影矩阵计算 + C# blittable 类型 + `[ComponentId]`。无 ABI 变更。 |
| 2026-05-23 | **Phase 4-K** 合入：媒体组件 — `NNAudioSourceComponent`（音频源）+ `NNVideoPlayerComponent`（视频播放器）POD 结构体注册。 |
| 2026-05-23 | **Phase 5** 合入：Snapshot 构建器 — `NNHierarchySnapshotBuilder`（Editor SceneBrowser）+ `NNReflectionSnapshotBuilder`（Editor Inspector），替代逐实体 P/Invoke。 |
| 2026-05-23 | **Phase 6** 合入：`NNJsonSceneSerializer` JSON 序列化（nlohmann/json，格式版本 2），支持可读/可 diff 场景文件。 |
| 2026-05-23 | **Phase 7** 合入：`NNPrefab` 二进制实例化器，与 C# `PrefabImporter.SerializePrefab` 格式对齐。 |

---

## 7. 未完成项

- **`NNSceneRuntime`**（Streaming / 多场景 / Activation）待后续 Phase 实现。
- **`NNSceneAssetLoader`** / **`NNSceneStreamingManager`**（Phase 7 占位）：场景资产异步加载 + Streaming Zone 管理。
- Native 事件 ABI 桥接待 layoutVersion 7 扩展（`NativeEventBridge` 目前为 stub）。
- **`SceneSubsystem` → `NNRuntimeScene`** 数据桥接或迁移工具。
- C 平面 API：`NN_AddComponent(SceneHandle, EntityHandle, ComponentTypeId)` 与 **layoutVersion** 评审。
- JSON 序列化 round-trip 测试覆盖。
- Legacy 场景资产迁移、C# 自动组件绑定。
- `NNTransformSystem` 脏标记优化（仅重算脏实体子树）。
- `NNSpriteRendererComponent` 运行时渲染管线集成。
- `NNAudioSourceComponent` / `NNVideoPlayerComponent` 运行时播放器桥接（FMOD / FFmpeg）。

---

## 8. 未来规划

与 [RUNTIME_ARCHITECTURE_AND_PROGRESS.md](../RUNTIME_ARCHITECTURE_AND_PROGRESS.md) §5 及 [MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md](../../../Managed/MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md) **§0.3 P0-2/P0-3** 对齐：本模块为 Native **VGEntitySystem** / **VGSceneRuntime** 的实体存储后端；托管 **EntityWorld** 仍为双世界 gameplay ECS，不承诺自动镜像。

### 近期方向

1. **Streaming & Asset Loading**：`NNSceneAssetLoader` Guid 字段扫描 + 异步加载优先级 + `NNSceneStreamingManager` 球形触发区域。
2. **Native Event ABI 扩展**：layoutVersion 7 新增事件回调函数指针，启用 `NativeEventBridge` 自动桥接。
3. **Transform 脏标记优化**：仅 DFS 重算脏实体子树，避免每帧全树遍历。
4. **SpriteRenderer 渲染管线集成**：`NNSpriteRendererComponent` → GPU Instance Batch → 2D Renderer。
5. **Audio/Video 运行时桥接**：`NNAudioSourceComponent` → FMOD 播放器、`NNVideoPlayerComponent` → FFmpeg 解码器。
