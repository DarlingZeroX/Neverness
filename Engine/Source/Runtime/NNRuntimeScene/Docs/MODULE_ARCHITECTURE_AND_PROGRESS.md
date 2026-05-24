# NNRuntimeScene — ECS-first 运行时场景

> CMake 目标 **`NevernessRuntime-Scene`**；C++ 命名空间 **`NN::Runtime::Scene`**。

## 1. 定位与边界

| 项目 | 说明 |
|------|------|
| **职责** | **Handle + POD Component + entt::registry** 的新一代 Native Scene：**世代 `NNEntity`**、实体生命周期、`Emplace`/`Query`、**字段级组件反射**、**System 调度**、层级、事件/脏跟踪、**二进制场景快照**。Transform 使用 `float3`/`quaternion`/`matrix`（`HCoreTypes`）与 Legacy 对齐。 |
| **不负责** | Legacy `IGameActor`/`IComponent`、**SceneSubsystem** 替换或数据迁移、**NNNativeEngineAPI** layout 扩展（`NN_AddComponent` 留 Phase 4）、**Prefab/Streaming** 完整实现、JSON 导出、并行 Job。 |
| **CMake 目标** | `NevernessRuntime-Scene`（`SHARED`） |
| **宏** | `NN_RUNTIME_SCENE_API` / `NN_RUNTIME_SCENE_EXPORT` |
| **依赖** | `NevernessCore-Core`（PUBLIC，含 vendored **entt**）；**NNNativeEngineAPI** 头路径（PRIVATE，仅字段对齐约定，不链接 ABI 表）。**不**链接 **NevernessRuntime-Engine**。 |
| **典型消费者** | **`NevernessRuntime-Engine`**（`NNRuntimeSceneTickSubsystem` + `VGEngineRuntime::EcsScene()`）；未来 **VGSceneRuntime** / 工具链。 |

### 1.1 与既有身份体系对照

| 体系 | 身份 | 关系 |
|------|------|------|
| **NNEngineLegacy** | `VGActorID` + `shared_ptr<IGameActor>` + entt | **并存**，不修改 |
| **NNRuntimeEngine** | `NNEntityHandle` 单调 id + `SceneSubsystem` map | **并存**；ECS 路径为 **`EcsScene()`**，不替换 `SceneSubsystem` |
| **本模块** | `NNEntity`（`uint64` Index+Generation）+ entt | **已落地** Phase 1–3 |
| **VisionGal.Managed.Entity** | `EntityHandle`（`uint` Index + `uint` Generation） | **打包规则对齐**；**无**自动桥接 |

---

## 2. 构建与选项

- 在 [`Engine/Source/Runtime/CMakeLists.txt`](../CMakeLists.txt) 中于 **`NNNativeEngineAPI`** 之后、`NNRuntimeEngine` 之前注册 `add_subdirectory("NNRuntimeScene")`。
- **PUBLIC** 包含目录：`Engine/Source/Runtime`、`NNRuntimeScene/Include`（消费者可用 `#include "Scene/NNRuntimeScene.h"`）。
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
├── NNRuntimeSceneExport.h
├── Docs/
├── Include/
│   ├── Scene/           # NNEntity, NNEntityHandleTable, NNWorld, NNRuntimeScene
│   ├── Components/      # Transform / Relationship / Tag / Camera（POD）
│   ├── Query/           # NNEntityQuery
│   ├── Reflection/      # NNComponentRegistry, NN_FIELD, NNComponentFieldType
│   ├── Systems/         # ISceneSystem, Scheduler, Transform/Hierarchy/SceneUpdate/Camera
│   ├── Serialization/   # NNSceneSerializer（二进制 VGSC）
│   ├── Prefab/          # NNPrefab 占位
│   └── Runtime/         # NNSceneEventBus, NNDirtyTracker, NNSceneRuntime 占位
├── Source/
│   ├── Scene/
│   ├── Reflection/
│   ├── Systems/
│   ├── Runtime/
│   └── Serialization/
└── Tests/
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

- 定义收口到 **`NNNativeEngineAPI/EngineHandles.h`**：`typedef uint64_t NNEntityHandle; using NNEntity = NNEntityHandle;`（全局命名空间）。
- `NN::Runtime::Scene::NNEntity` 为同一类型的命名空间别名，所有消费者代码继续使用 `NNEntity`。
- `PackEntityHandle` / `UnpackEntityHandle`：低 32 = **Index**（自 1 起），高 32 = **Generation**（Destroy 递增）。
- `NNEntityInvalid == 0`：与托管 `EntityHandle.Invalid` 一致。

### 4.3 NNRuntimeScene

| API | 说明 |
|-----|------|
| `CreateEntity()` / `DestroyEntity(NNEntity)` | 世代校验 + entt 生命周期 |
| `CreateEntityWithDefaults()` | Transform + Relationship + Tag 默认 POD（Camera 需手动 Emplace） |
| `Emplace<T>` / `TryGet<T>` / `Has<T>` / `Remove<T>` | entt 组件 CRUD |
| `Query<Components...>()` | `NNEntityQuery::Each` |
| `RegisterSystem` / `TickSystems` | **NNSceneSystemScheduler** |
| `SetParent` / `GetParent` / `GetChildren` | 委托 **NNHierarchySystem** |
| `Events()` / `Dirty()` | **NNSceneEventBus** / **NNDirtyTracker** |
| `ForEachAliveEntity` | 序列化与编辑器遍历 |

构造时默认注册：**HierarchySystem → SceneUpdateSystem → TransformSystem → CameraSystem**。

### 4.4 反射与序列化

- **`NN_REGISTER_COMPONENT`** + **`NN_FIELD`**：字段 `offset`/`size`/`NNComponentFieldType`/`nameUtf8`。
- 内置：**Transform**、**Relationship**、**Tag**、**Camera** 全字段已注册。
- **`NNSceneSerializer`**：魔数 **`VGSC`**（格式版本 2），按注册表字段 `memcpy` 快照；组件 TypeId 为 **FNV-1a name hash**（8 字节，跨进程稳定）；反序列化按 name hash 查表重建实体（Index/Generation 由运行时重新分配）；Relationship 经 **`SetParent`** 恢复层级。

### 4.5 Engine 适配（依赖方向：Engine → Scene）

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
| JSON 导出 | **未做**（与 **VGManagedScene** JSON DTO 边界保留） |

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
| 编译验证：Engine + Scene + Editor.Serialization + Tests 全部通过 | **已完成** |
| 测试验证：10 个 Scene 相关测试全绿 | **已完成** |

### Phase 4-G（2026-05-23）— System 框架

| 项 | 状态 |
|----|------|
| `TickGroup` 枚举：对齐 Native `NNSceneTickGroup`（EarlyUpdate/FixedUpdate/Update/LateUpdate/Render） | **已完成** |
| `ISceneSystem` 标记接口 + `ISystemInitialize` / `ISystemShutdown` / `ISystemTick` / `ISystemFixedTick` / `ISystemLateTick` | **已完成** |
| `SystemDependencyAttribute`：`[SystemDependency(typeof(X))]` 依赖声明 | **已完成** |
| `SceneSystemScheduler`：Kahn 拓扑排序 + 按 TickGroup 分组 + 延迟 Initialize | **已完成** |
| `SceneWorld.Systems` 属性 + `Tick()` 完整 Tick 流（EarlyUpdate → Update → Native → LateUpdate → Render） | **已完成** |
| `SceneWorld.FixedTick()`：独立固定步长 Tick | **已完成** |
| 编译验证：Scene + Editor.Serialization + Tests 全部通过 | **已完成** |
| 测试验证：10 个 Scene 相关测试全绿 | **已完成** |

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
| `SceneNativeBridge.TryGetSceneApi` 改为 internal | **已完成** |
| `Prefab` 旧类重构为便捷包装器（保留 builder API，内部委托 PrefabAsset） | **已完成** |
| 编译验证：Scene + Editor.Serialization + Tests 全部通过 | **已完成** |

### Phase 4-I（2026-05-23）— 事件系统 + 热重载

| 项 | 状态 |
|----|------|
| `SceneEventType` 枚举：EntityCreated / EntityDestroyed / ParentChanged / ComponentEmplaced | **已完成** |
| `SceneEvent` 结构体：Type / Entity / OtherEntity / ComponentTypeId + 工厂方法 | **已完成** |
| `SceneEventBus`：Subscribe / Unsubscribe / SubscribeAll / Emit / EmitDeferred / FlushDeferred / Clear | **已完成** |
| `NativeEventBridge`：持有 SceneEventBus 引用 + Native 回调委托 + EnableAutoBridge + InjectEvent（待 ABI 扩展后启用自动桥接） | **已完成** |
| `HotReloadSnapshot`：Name / NativeHandle / AssetGuid / EntityHandles / CapturedAt | **已完成** |
| `GlobalHotReloadSnapshot`：Worlds 字典 + ActiveWorldName + CapturedAt | **已完成** |
| `SceneWorld`：新增 Events 属性 + AssetGuid + SaveSnapshot / RestoreFromSnapshot / RebuildAfterReload | **已完成** |
| `SceneWorld.Tick`：末尾新增 Events.FlushDeferred() | **已完成** |
| `SceneWorld.Dispose`：新增 NativeEventBridge 释放 + Events.Clear() | **已完成** |
| `SceneSystemScheduler.Rebuild()`：关闭所有 System 并清空注册（热重载用） | **已完成** |
| `EntityRegistry`：新增 Get / TryGet / SyncFromHandles / ExportHandleValues | **已完成** |
| `SceneManager`：新增 SaveAllSnapshots / RestoreAllSnapshots / RebuildAllAfterReload | **已完成** |
| 编译验证：Scene + Editor.Serialization + Tests 全部通过 | **已完成** |
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
| 编译验证：C# Engine + Scene 全部通过 | **已完成** |

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
| 2026-05-23 | **Phase 4-C** 合入：C# SceneManager 架构重构 — SceneAPI.h layoutVersion 3→4、NNSceneHandle uint32→uint64、SceneSubsystem 改为管理 NNRuntimeScene 实例、C# 端 SceneManager 负责场景生命周期/多场景管理、SceneEntity 改用泛型组件操作、Prefab 改为 CreateEntity+AddComponent 模式、NNSceneSerializeBridge 适配新 ABI。 |
| 2026-05-23 | **Phase 4-D** 合入：C# 端 ComponentId 重构 — `ComponentIdAttribute`（Engine 程序集）+ `ComponentTypeCache<T>` 替代 `ComponentTypeRegistry<T>`、`NNTransformData` 标注 `[ComponentId]`、`SceneNativeBridge` 全部切换到 `ComponentTypeCache<T>`。 |
| 2026-05-23 | **Phase 4-E** 合入：SceneWorld 提取 — `EntityRegistry`（handle↔entity 映射）、`SceneQueryCache`（Stub）、`SceneWorld`（Gameplay Runtime Root：NativeHandle + Entities + Queries + Tick）、`SceneManager` 降级为世界管理器（`Dictionary<string, SceneWorld>` + 委托模式，旧 API 全部向后兼容）、10 个 Scene 测试全绿。 |
| 2026-05-23 | **Phase 4-F** 合入：Query 系统 — Native 端新增 `queryEntities` / `queryComponents` / `queryCount2` ABI 函数（layoutVersion 5→6）、C# 端 `NativeQueryBridge` 批量查询封装、`SceneView<T>` / `SceneView<T1,T2>` ref struct 零 GC 视图（`MemoryMarshal.AsRef` 直接引用 Span 数据）、`SceneQuery<T>` / `SceneQuery<T1,T2>` 缓存式查询对象、`SceneQueryCache.GetQuery<T>` 实现、`SceneWorld.GetQuery` 便捷方法。 |
| 2026-05-23 | **Phase 4-G** 合入：Managed System 框架 — `TickGroup` 枚举（对齐 Native NNSceneTickGroup）、`ISceneSystem` + 5 个子接口（Initialize/Shutdown/Tick/FixedTick/LateTick）、`SystemDependencyAttribute` 依赖声明、`SceneSystemScheduler`（Kahn 拓扑排序 + 按 TickGroup 分组 + 延迟 Initialize + 循环依赖检测）、`SceneWorld.Tick()` 完整 Tick 流（EarlyUpdate → Update → Native TickSystems → LateUpdate → Render）、`SceneWorld.FixedTick()` 独立固定步长、`SceneManager.TickActiveScene` 返回值改为 void。 |
| 2026-05-23 | **Phase 4-H** 合入：Prefab 系统 — `PrefabAsset`（Guid/Name/Entities + `FromEntity()` BFS 构建）、`PrefabEntityData`（LocalIndex/ParentIndex/Components）、`PrefabOverride`（5 种覆盖类型）、`PrefabInstance`（Source/RootEntity/InstanceMap/Overrides）、`PrefabInstantiator`（Instantiate 完整流程 + ApplyOverrides + RevertToPrefab + DetectDifferences）、`SceneNativeBridge` 新增 `SetComponentData` / 原始 TypeId `AddComponent`/`RemoveComponent` 重载、旧 `Prefab` 类重构为便捷包装器（保留 builder API）。 |
| 2026-05-23 | **Phase 4-I** 合入：事件系统 + 热重载 — `SceneEventType` 枚举（对齐 Native NNSceneEventType）、`SceneEvent` 结构体（Type/Entity/OtherEntity/ComponentTypeId + 工厂方法）、`SceneEventBus`（Subscribe/Unsubscribe/Emit/EmitDeferred/FlushDeferred，递归 Emit 自动降级为 deferred）、`NativeEventBridge`（Native 回调桥接 stub，待 ABI layoutVersion 7 后启用自动桥接）、`HotReloadSnapshot` + `GlobalHotReloadSnapshot`（Managed 侧状态快照）、`SceneWorld` 新增 Events 属性 + SaveSnapshot / RestoreFromSnapshot / RebuildAfterReload、`SceneSystemScheduler.Rebuild()`（热重载用 Shutdown+清空）、`EntityRegistry` 新增 Get/TryGet/SyncFromHandles/ExportHandleValues、`SceneManager` 新增 SaveAllSnapshots/RestoreAllSnapshots/RebuildAllAfterReload、`SceneWorld.Tick` 末尾 FlushDeferred、`SceneWorld.Dispose` 完整清理 Events+NativeEventBridge。 |
| 2026-05-23 | **Phase 4-J** 合入：NNCameraComponent — `NNCameraComponent` POD 结构体（`NNProjectionType` + 透视/正交参数 + `ProjectionMatrix`）、`NNCameraSystem`（`glm::perspectiveRH_NO` / `glm::orthoRH_NO` 投影矩阵计算）、`BuiltinComponentRegistration` 注册 Camera 组件（FNV-1a TypeId `0x54D1B2A64667E32E`）、`NNRuntimeScene` 注册 CameraSystem、C# 端 `NNProjectionType` / `NNMat4` / `NNCameraComponentData` blittable 结构体（`[ComponentId(0x54D1B2A64667E32E, Name = "Camera")]`）。无 ABI 变更。 |

---

## 7. 未完成项（Phase 4+）

- ~~**`NNPrefab`**~~ **已完成（Phase 4-H）**；**`NNSceneRuntime`**（Streaming / 多场景 / Activation）待后续。
- ~~C# 事件系统 + 热重载~~ **已完成（Phase 4-I）**；Native 事件 ABI 桥接待 layoutVersion 7 扩展。
- **`SceneSubsystem` → `NNRuntimeScene`** 数据桥接或迁移工具。
- C 平面 API：`NN_AddComponent(SceneHandle, EntityHandle, ComponentTypeId)` 与 **layoutVersion** 评审。
- ~~跨进程稳定 **TypeId**（FNV name hash 可选轨）~~ **已完成（Phase 4-B）**；JSON 序列化。
- Legacy 场景资产迁移、C# 自动组件绑定。
- `NNTransformSystem` 脏标记优化（仅重算脏实体子树）。

---

## 8. 未来规划

与 [RUNTIME_ARCHITECTURE_AND_PROGRESS.md](../RUNTIME_ARCHITECTURE_AND_PROGRESS.md) §5 及 [MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md](../../../Managed/MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md) **§0.3 P0-2/P0-3** 对齐：本模块为 Native **VGEntitySystem** / **VGSceneRuntime** 的实体存储后端；托管 **EntityWorld** 仍为双世界 gameplay ECS，不承诺自动镜像。
