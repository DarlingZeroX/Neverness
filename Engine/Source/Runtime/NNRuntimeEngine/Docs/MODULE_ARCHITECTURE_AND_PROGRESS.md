# VGEngineRuntime — 进程级 Runtime Facade

## 1. 定位与边界

| 项目 | 说明 |
|------|------|
| **职责** | 提供 **`VGEngineRuntime`** 单例：`Initialize` / `Tick` / `Shutdown`；内置 **TimingSystem**、**RuntimeScheduler**（**VGRuntimeScheduler** / **P0-1**）、**AsyncSystem**、**SceneSubsystem**、**AssetSubsystem**、**ObjectSubsystem**、**AssetRegistrySubsystem**、**EntitySubsystem**（实现 **IRuntimeSubsystem**，**Update** 组；**`runtimeTick`**、**`VG_ENTITY_SERVICE_ABI_TOKEN`** 对齐 **VGEntityAPI**）。**当前形态**为 **Engine Service 状态机聚合** 向 **Runtime Kernel** 演进（见 [MANAGED 总览 §0.3 P0-1](../../../Managed/MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md)）。 |
| **不负责** | 不链接 **VGEngine**、**VGRHI**、**VGUI**；**不**承载 **Gameplay** 产品逻辑（变量、剧本 VM、Graph 执行等均在 **Managed**）。与 **VGNativeEngineAPI** 的衔接由 **VGEngineRuntimeServices** 覆写函数表完成。 |
| **CMake 目标** | `VGEngineRuntime`（`STATIC`） |
| **依赖** | `VGNativeEngineAPI`（PUBLIC：Handle / `VGTransform3` / `VGGuid` 等头）。 |
| **典型消费者** | **VGEngineRuntimeServices**；单元测试与托管宿主通过 C 封装 `VGEngineRuntimeHost_*` 驱动 Tick。 |

---

## 2. 构建与选项

无独立 CMake `option`；由根 CMake 的 **`VISIONGAL_USE_ENGINE_RUNTIME_SERVICES`** 决定是否链接 **VGEngineRuntimeServices**（进而使用本库状态机）。

---

## 3. 目录结构

```
Engine/Source/Runtime/VGEngineRuntime/
├── CMakeLists.txt
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md   ← 本文件
├── Include/VGEngineRuntime/
│   ├── VGEngineRuntime.h
│   ├── TimingSystem.h
│   ├── AsyncSystem.h
│   ├── SceneSubsystem.h
│   ├── AssetSubsystem.h
│   ├── ObjectSubsystem.h
│   ├── AssetRegistrySubsystem.h
│   ├── EntitySubsystem.h
│   └── RuntimeScheduler/
│       ├── RuntimeTickGroup.h
│       ├── RuntimeFrameContext.h
│       ├── RuntimeSubsystem.h
│       ├── RuntimeSubsystemCollection.h
│       ├── RuntimeScheduler.h
│       ├── RuntimePhase.h
│       ├── RuntimePipeline.h
│       └── RuntimePipelineBuilder.h
└── Private/
    ├── VGEngineRuntime.cpp
    ├── TimingSystem.cpp
    ├── AsyncSystem.cpp
    ├── SceneSubsystem.cpp
    ├── AssetSubsystem.cpp
    ├── ObjectSubsystem.cpp
    ├── AssetRegistrySubsystem.cpp
    ├── EntitySubsystem.cpp
    ├── RuntimeSubsystemCollection.cpp
    └── RuntimeScheduler.cpp
```

---

## 4. 使用说明

### 4.1 包含方式

```cpp
#include "VGEngineRuntime/VGEngineRuntime.h"
```

子系统头文件可按需包含（例如仅测试 Async 时包含 `AsyncSystem.h`）。

### 4.2 线程与生命周期契约

- **`Tick`** 须在与 **`Initialize` / `Shutdown` 相同的控制线程** 调用（与未来 game loop 对齐）。
- **`Shutdown`** 会 **join** 尚未 `releaseWait` 的后台线程；**禁止**在 Async 回调内调用 **`Shutdown`**。
- **`AsyncSystem`**：`CreateWait` 与 `Shutdown` 互斥；详见类注释。

### 4.3 推荐驱动顺序

1. `VGEngineRuntime::Instance().Initialize()`（或 C 侧 `VGEngineRuntimeHost_Initialize`，见 Services 文档）：内部 **`timing_.Reset`** → **`scheduler_.RegisterSubsystem(&entity_)`**（幂等）→ **`scheduler_.InitializeRegistered`**。
2. 每帧：`Tick(deltaTimeSeconds)`：**先** **`timing_.Tick`**，再构造 **`RuntimeFrameContext`**，**最后** **`scheduler_.Tick`**（其中 **EntitySubsystem** 于 **Update** 阶段推进 **`runtimeTick`**）。
3. 进程退出前：`Shutdown()`：**先** **`async_.Shutdown`**，再 **`scheduler_.ShutdownRegistered`**（**EntitySubsystem::Shutdown** 内 **`Reset`**）。

### 4.4 与 VGEngineRuntimeServices 的关系

**VGEngineRuntimeServices** 将 **VGNativeEngineAPI** 表中 Timing、Async、Scene、Asset（纹理/音频项）、Object、AssetRegistry、**Entity** 等指针指向本单例子系统的 thunk；Render/UI/Audio/Input 等仍可能保持 Stub。详见 [VGEngineRuntimeServices 文档](../VGEngineRuntimeServices/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)。

---

## 5. 接口与 API 文档（C++）

命名空间：`visiongal::engine`。

### 5.1 `VGEngineRuntime`（[`VGEngineRuntime.h`](../Include/VGEngineRuntime/VGEngineRuntime.h)）

| 成员 | 说明 |
|------|------|
| `static VGEngineRuntime& Instance() noexcept` | 进程级单例。 |
| `bool Initialize() noexcept` | 初始化各子系统状态。 |
| `void Tick(float deltaTimeSeconds) noexcept` | **先** **`timing_.Tick`**；再构造 **`RuntimeFrameContext`**；**最后** **`scheduler_.Tick`** 驱动 **IRuntimeSubsystem**（**EntitySubsystem** 于 **Update** 组递增 **`runtimeTick`**）。Async 轮询语义仍由表侧使用方式决定。 |
| `void Shutdown() noexcept` | 关闭；**先** join **Async**，再 **`scheduler_.ShutdownRegistered`**。 |
| `Timing()` / `Async()` / `Scene()` / `Asset()` / `Object()` / `AssetRegistry()` / `Entity()` | 子系统引用。 |
| `RuntimeScheduler& Scheduler() noexcept` | **P0-1**：统一 Tick 管线调度器。 |
| `bool IsInitialized() const noexcept` | 是否已完成 `Initialize`。 |

### 5.2 `TimingSystem`（[`TimingSystem.h`](../Include/VGEngineRuntime/TimingSystem.h)）

| 成员 | 说明 |
|------|------|
| `void Reset() noexcept` | 重置累积状态。 |
| `void Tick(float deltaTimeSeconds) noexcept` | 累加 `totalTime`、`frameIndex`，记录 `lastDelta`。 |
| `GetDeltaTime()` | 上一帧传入的 Δt。 |
| `GetTotalTime()` | 累积时间。 |
| `GetFrameIndex()` | 每次 `Tick` 结束后递增（首次 `Tick` 后为 `1`，见头文件注释）。 |

### 5.3 `AsyncSystem`（[`AsyncSystem.h`](../Include/VGEngineRuntime/AsyncSystem.h)）

| 成员 | 说明 |
|------|------|
| `std::uint64_t CreateWait()` | 分配等待槽并返回 handle（非零）。 |
| `int TryComplete(std::uint64_t handle) noexcept` | 未完成返回 `0`；完成后返回 `1`（直到 `ReleaseWait`）。 |
| `void ReleaseWait(std::uint64_t handle) noexcept` | 释放。 |
| `void Shutdown() noexcept` | join 所有未完成等待；与 `CreateWait` 互斥阶段。 |

### 5.4 `SceneSubsystem`（[`SceneSubsystem.h`](../Include/VGEngineRuntime/SceneSubsystem.h)）

提供与 **VGNativeEngineAPI::VGSceneAPI** 对齐的 C++ 方法：`LoadScene`、`Spawn`、`Destroy`、`Find`、`Activate`、`UnloadScene`、`GetActiveSceneName`、`SetParent`、`GetParent`、`GetChildCount`、`GetChildAt`、`GetTransform`、`SetTransform`、`SetEntityName`、`GetEntityName`。实现为进程内内存模型，**未接 VGEngine**。

### 5.5 `AssetSubsystem`（[`AssetSubsystem.h`](../Include/VGEngineRuntime/AssetSubsystem.h)）

| 方法 | 说明 |
|------|------|
| `LoadTexture` / `LoadAudio` | 未接 **VGAsset** 前返回 `0` handle（空壳）。 |

### 5.6 `ObjectSubsystem`（[`ObjectSubsystem.h`](../Include/VGEngineRuntime/ObjectSubsystem.h)）

托管 `VGObject` 桥接：创建/销毁、 retain/release、引用计数、存活查询、`GetTypeName` 写入缓冲。

### 5.7 `AssetRegistrySubsystem`（[`AssetRegistrySubsystem.h`](../Include/VGEngineRuntime/AssetRegistrySubsystem.h)）

GUID ↔ 虚拟路径登记、依赖查询、`ImportAsset` 生成合成 GUID（实现细节见源码）。

### 5.8 `EntitySubsystem`（[`EntitySubsystem.h`](../Include/VGEngineRuntime/EntitySubsystem.h)）

| 成员 | 说明 |
|------|------|
| **`IRuntimeSubsystem`** | **TickGroup** → **`Update`**；**Tick** → **`OnTick`**；**Shutdown** → **`Reset`**。 |
| `OnTick(float)` | 仍可由测试或遗留路径直接调用；生产路径经 **RuntimeScheduler**。 |
| `Reset()` | 清零 **`runtimeTick`**（**Shutdown** 经调度器触发）。 |
| `GetServiceAbiToken()` | 返回 **`VG_ENTITY_SERVICE_ABI_TOKEN`**，与 **Stub** / **VGEntityAPI** 魔数一致。 |
| `GetRuntimeTick()` | 供 **`getRuntimeTick`** ABI 观测；**非**托管 **EntityWorld** 镜像。 |

### 5.9 `RuntimeScheduler` 与 **RuntimeScheduler/** 头文件（**P0-1**）

| 符号 | 说明 |
|------|------|
| **`IRuntimeSubsystem`** | 子系统接口：**Initialize** / **Shutdown** / **Tick(RuntimeFrameContext)** / **TickGroup**。 |
| **`RuntimeScheduler`** | **RegisterSubsystem**、**InitializeRegistered** / **ShutdownRegistered**、**Tick**；含 **FixedUpdate** 累加与 **`kMaxFixedStepsPerFrame`**；**LateUpdate** 后 **`FlushMainThreadDelegates`** 占位。 |
| **`RuntimeTickGroup`** | **EarlyUpdate** / **FixedUpdate** / **Update** / **LateUpdate** / **Render**。 |
| **`RuntimeFrameContext`** | 只读帧上下文（Δt、累计时间、**frameIndex**、固定步长）。 |
| **`RuntimePhase` / `RuntimePipeline` / `RuntimePipelineBuilder`** | 占位，预留可配置 **PlayerLoop** 扩展。 |

---

## 6. 开发进展

| 日期 | 进展 |
|------|------|
| 2026-05-14 | Phase 4 首包：静态库与子系统骨架；供 **VGEngineRuntimeServices** 转发 ABI。 |
| 2026-05-15 | 文档扩展：简体中文、子系统 API 表、与 Services 的契约说明。 |
| 2026-05-16 | 文档交叉：与 [Runtime 总览](../../RUNTIME_ARCHITECTURE_AND_PROGRESS.md) **§5.2** 总体状态同步；托管 **Phase 6 slice 5** 不扩展本子系统 C++ 行为，后续 **VGEntity** 相关由 **VGNativeEngineAPI** 路线驱动（见 MANAGED **§2.7**）。 |
| 2026-05-17 | 文档交叉：MANAGED **§2** Phase 6 表区分「託管 slice 2–5 已落地」与 **Native** 子项；与 [Runtime 总览](../../RUNTIME_ARCHITECTURE_AND_PROGRESS.md) **§5.1** 验证里程碑对齐。 |
| 2026-05-18 | 文档交叉：MANAGED **§5.1** Native 推进草案与 [Runtime 总览](../../RUNTIME_ARCHITECTURE_AND_PROGRESS.md) **§6**；本库仍不扩展 C++ 行为直至 ABI 评审。 |
| 2026-05-19 | **Kernel 边界**：**SceneSubsystem** 等仍仅服务 Native ABI 转发；与托管 **EntityWorld**（**HasComponent** / **GetComponent**）无自动桥接（见 MANAGED **§2.5.2**）。 |
| 2026-05-20 | **Kernel 边界（续）**：托管 **EntityWorld.GetComponentCount** 为纯 C# 调试向 API；本子系统 C++ 无对应转发（见 MANAGED **§2.5.3**）。 |
| 2026-05-21 | **Kernel 边界（续）**：托管 **EntityWorld** **Type** 键 **HasComponent**／**TryGetComponent** 仍为纯 C#；与 **SceneSubsystem** 无桥接（见 MANAGED **§2.5.4**）。 |
| 2026-05-15 | **Kernel 边界（续）**：托管 **EntityWorld** **Type** 键 **GetComponent**／**RemoveComponent** 仍为纯 C#；**Kernel** 无新转发（见 MANAGED **§2.5.5**）。 |
| 2026-05-15 | **§2.7.1 Kernel 首包**：新增 **EntitySubsystem**（**`runtimeTick`**、魔数）；**`VGEngineRuntime::Tick` / `Shutdown`** 接线；**`entity.*`** 由 **VGEngineRuntimeServices** 覆写（**layout v5**）；与 MANAGED **§2.7.1**／[VGNativeEngineAPI MODULE](../VGNativeEngineAPI/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md) 交叉。 |
| 2026-05-15 | **P0 注释与契约收口**：**`VGEngineRuntime.cpp` / `EntitySubsystem.cpp`** 增补简体中文 `@file` 与 **Tick / Shutdown** 顺序说明；与 MANAGED **§2.7.1** 文档互证。 |
| 2026-05-15 | **§0 主線對齊**：定位表補充「當前聚合 vs **Kernel** 目標」與 **VGRuntimeScheduler** 索引；與 MANAGED **§0.3** **P0-1**／**P0-2** 互鏈。 |
| 2026-05-15 | **P0-1 首包**：新增 **Include/.../RuntimeScheduler/**（**RuntimeScheduler**、**IRuntimeSubsystem**、**RuntimeTickGroup**、**RuntimeFrameContext**、Collection、**FixedUpdate** 上限、**FlushMainThreadDelegates** 占位）；**EntitySubsystem** 实现接口并挂 **Update**；**VGEngineRuntime** 集成 **`Scheduler()`**；**CMake** 增源；**ABI** 未变。 |

- [Runtime 总览](../../RUNTIME_ARCHITECTURE_AND_PROGRESS.md)
- [VGNativeEngineAPI](../VGNativeEngineAPI/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
- [VGEngineRuntimeServices](../VGEngineRuntimeServices/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
