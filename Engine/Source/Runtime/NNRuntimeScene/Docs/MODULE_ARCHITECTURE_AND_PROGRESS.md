# NNRuntimeScene — ECS-first 运行时场景

> CMake 目标 **`NevernessRuntime-Scene`**；C++ 命名空间 **`NN::Runtime::Scene`**。

## 1. 定位与边界

| 项目 | 说明 |
|------|------|
| **职责** | **Handle + POD Component + entt::registry** 的新一代 Native Scene：**世代 `NNEntity`**、实体生命周期、`Emplace`/`Query`、**字段级组件反射**、**System 调度**、层级、事件/脏跟踪、**二进制场景快照**。 |
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
- `ENABLE_TESTS=ON` 且找到 GTest 时构建 **`NNRuntimeSceneTest`**（10 cases）。

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
│   ├── Components/      # Transform / Relationship / Tag（POD）
│   ├── Query/           # NNEntityQuery
│   ├── Reflection/      # NNComponentRegistry, NN_FIELD, NNComponentFieldType
│   ├── Systems/         # ISceneSystem, Scheduler, Transform/Hierarchy/SceneUpdate
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

- `PackEntityHandle` / `UnpackEntityHandle`：低 32 = **Index**（自 1 起），高 32 = **Generation**（Destroy 递增）。
- `NNEntityInvalid == 0`：与托管 `EntityHandle.Invalid` 一致。

### 4.3 NNRuntimeScene

| API | 说明 |
|-----|------|
| `CreateEntity()` / `DestroyEntity(NNEntity)` | 世代校验 + entt 生命周期 |
| `CreateEntityWithDefaults()` | Transform + Relationship + Tag 默认 POD |
| `Emplace<T>` / `TryGet<T>` / `Has<T>` / `Remove<T>` | entt 组件 CRUD |
| `Query<Components...>()` | `NNEntityQuery::Each` |
| `RegisterSystem` / `TickSystems` | **NNSceneSystemScheduler** |
| `SetParent` / `GetParent` / `GetChildren` | 委托 **NNHierarchySystem** |
| `Events()` / `Dirty()` | **NNSceneEventBus** / **NNDirtyTracker** |
| `ForEachAliveEntity` | 序列化与编辑器遍历 |

构造时默认注册：**HierarchySystem → SceneUpdateSystem → TransformSystem**。

### 4.4 反射与序列化

- **`NN_REGISTER_COMPONENT`** + **`NN_FIELD`**：字段 `offset`/`size`/`NNComponentFieldType`/`nameUtf8`。
- 内置：**Transform**、**Relationship**、**Tag** 全字段已注册。
- **`NNSceneSerializer`**：魔数 **`VGSC`**，按注册表字段 `memcpy` 快照；反序列化重建实体（Index/Generation 由运行时重新分配）；Relationship 经 **`SetParent`** 恢复层级。

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

---

## 6. 开发进展

| 日期 | 进展 |
|------|------|
| 2026-05-17 | **Phase 1** 合入：`NevernessRuntime-Scene` SHARED 库、GTest 4 cases。 |
| 2026-05-17 | **Phase 2–3** 合入：System 层、层级、事件/脏、字段反射、**VGSC** 序列化；**Engine** 私有链接 Scene 并挂 **Update** Tick；**Legacy / SceneSubsystem / ABI layout** 未改。 |

---

## 7. 未完成项（Phase 4+）

- **`NNPrefab`** / **`NNSceneRuntime`**（Streaming / 多场景 / Activation）。
- **`SceneSubsystem` → `NNRuntimeScene`** 数据桥接或迁移工具。
- C 平面 API：`NN_AddComponent(SceneHandle, EntityHandle, ComponentTypeId)` 与 **layoutVersion** 评审。
- **NNTransformSystem** 世界矩阵计算（当前为 Update 组占位 pass）。
- 跨进程稳定 **TypeId**（FNV name hash 可选轨）；JSON 序列化。
- Legacy 场景资产迁移、C# 自动组件绑定。

---

## 8. 未来规划

与 [RUNTIME_ARCHITECTURE_AND_PROGRESS.md](../RUNTIME_ARCHITECTURE_AND_PROGRESS.md) §5 及 [MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md](../../../Managed/MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md) **§0.3 P0-2/P0-3** 对齐：本模块为 Native **VGEntitySystem** / **VGSceneRuntime** 的实体存储后端；托管 **EntityWorld** 仍为双世界 gameplay ECS，不承诺自动镜像。
