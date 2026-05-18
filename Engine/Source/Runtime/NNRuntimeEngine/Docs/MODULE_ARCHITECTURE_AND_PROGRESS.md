# NNRuntimeEngine — 进程级 Runtime Facade

> 曾用名：**VGEngineRuntime**；CMake 目标 **`NevernessRuntime-Engine`**；C++ 类名仍为 **`VGEngineRuntime`**；命名空间 **`NN::Runtime::engine`**。

## 1. 定位与边界

| 项目 | 说明 |
|------|------|
| **职责** | 提供 **`VGEngineRuntime`** 单例：`Initialize` / `Tick` / `Shutdown`；内置 **TimingSystem**、**RuntimeScheduler**（**P0-1**）、**AsyncSystem**、**SceneSubsystem**、**AssetSubsystem**、**ObjectSubsystem**、**AssetRegistrySubsystem**、**EntitySubsystem**（实现 **IRuntimeSubsystem**，**Update** 组；**`runtimeTick`**、**`NN_ENTITY_SERVICE_ABI_TOKEN`** 对齐 **NNEntityAPI**）。**当前形态**为 **Engine Service 状态机聚合** 向 **Runtime Kernel** 演进（见 [MANAGED 总览 §0.3 P0-1](../../../Managed/MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md)）。 |
| **不负责** | 不链接 **NevernessRuntime-EngineLegacy**、**NevernessRuntime-RHI**、**NevernessRuntime-RmlUI**；**不**承载 **Gameplay** 产品逻辑（变量、剧本 VM、Graph 执行等均在 **Managed**）。与 **NNNativeEngineAPI** 的衔接由 **NNRuntimeEngineServices** 覆写函数表完成。 |
| **CMake 目标** | `NevernessRuntime-Engine`（`STATIC`） |
| **依赖** | `NevernessRuntime-NativeEngineAPI`（PUBLIC：Handle / `NNTransform3` / `NNGuid` 等头）。 |
| **典型消费者** | **NevernessRuntime-EngineServices**；单元测试与托管宿主通过 C 封装 `VGEngineRuntimeHost_*` 驱动 Tick。 |

---

## 2. 构建与选项

无独立 CMake `option`；由根 CMake 的 **`VISIONGAL_USE_ENGINE_RUNTIME_SERVICES`** 决定是否链接 **NevernessRuntime-EngineServices**（进而使用本库状态机）。

---

## 3. 目录结构

```
Engine/Source/Runtime/NNRuntimeEngine/
├── CMakeLists.txt
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md   ← 本文件
├── Include/
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
    └── …
```

---

## 4. 使用说明

### 4.1 包含方式

```cpp
#include "NNRuntimeEngine/Include/VGEngineRuntime.h"
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

### 4.4 与 NNRuntimeEngineServices 的关系

**NevernessRuntime-EngineServices** 将 **NNNativeEngineAPI** 表中 Timing、Async、Scene、Asset（纹理/音频项）、Object、AssetRegistry、**Entity** 等指针指向本单例子系统的 thunk；Render/UI/Audio/Input 等仍可能保持 Stub。详见 [NNRuntimeEngineServices 文档](../NNRuntimeEngineServices/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)。

---

## 5. 接口与 API 文档（C++）

命名空间：`NN::Runtime::engine`。

### 5.1 `VGEngineRuntime`（[`VGEngineRuntime.h`](../Include/VGEngineRuntime.h)）

| 成员 | 说明 |
|------|------|
| `static VGEngineRuntime& Instance() noexcept` | 进程级单例。 |
| `bool Initialize() noexcept` | 初始化各子系统状态。 |
| `void Tick(float deltaTimeSeconds) noexcept` | **先** **`timing_.Tick`**；再构造 **`RuntimeFrameContext`**；**最后** **`scheduler_.Tick`** 驱动 **IRuntimeSubsystem**（**EntitySubsystem** 于 **Update** 组递增 **`runtimeTick`**）。 |
| `void Shutdown() noexcept` | 关闭；**先** join **Async**，再 **`scheduler_.ShutdownRegistered`**。 |
| `Timing()` / `Async()` / `Scene()` / `Asset()` / `Object()` / `AssetRegistry()` / `Entity()` | 子系统引用。 |
| `RuntimeScheduler& Scheduler() noexcept` | **P0-1**：统一 Tick 管线调度器。 |
| `bool IsInitialized() const noexcept` | 是否已完成 `Initialize`。 |

### 5.2 `TimingSystem`（[`TimingSystem.h`](../Include/TimingSystem.h)）

| 成员 | 说明 |
|------|------|
| `void Reset() noexcept` | 重置累积状态。 |
| `void Tick(float deltaTimeSeconds) noexcept` | 累加 `totalTime`、`frameIndex`，记录 `lastDelta`。 |
| `GetDeltaTime()` | 上一帧传入的 Δt。 |
| `GetTotalTime()` | 累积时间。 |
| `GetFrameIndex()` | 每次 `Tick` 结束后递增。 |

### 5.3 `AsyncSystem`（[`AsyncSystem.h`](../Include/AsyncSystem.h)）

| 成员 | 说明 |
|------|------|
| `std::uint64_t CreateWait()` | 分配等待槽并返回 handle（非零）。 |
| `int TryComplete(std::uint64_t handle) noexcept` | 未完成返回 `0`；完成后返回 `1`（直到 `ReleaseWait`）。 |
| `void ReleaseWait(std::uint64_t handle) noexcept` | 释放。 |
| `void Shutdown() noexcept` | join 所有未完成等待；与 `CreateWait` 互斥阶段。 |

### 5.4 `SceneSubsystem`（[`SceneSubsystem.h`](../Include/SceneSubsystem.h)）

提供与 **NNNativeEngineAPI::NNSceneAPI** 对齐的 C++ 方法。实现为进程内内存模型，**未接 EngineLegacy**。

### 5.5 `AssetSubsystem`（[`AssetSubsystem.h`](../Include/AssetSubsystem.h)）

| 方法 | 说明 |
|------|------|
| `LoadTexture` / `LoadAudio` | 未接 **NevernessRuntime-Asset** 前返回 `0` handle（空壳）。 |

### 5.6 `ObjectSubsystem` / `AssetRegistrySubsystem` / `EntitySubsystem`

见各头文件；**EntitySubsystem** 实现 **IRuntimeSubsystem**，供 **`entity.*`** ABI 转发。

### 5.7 `RuntimeScheduler` 与 **RuntimeScheduler/** 头文件（**P0-1**）

| 符号 | 说明 |
|------|------|
| **`IRuntimeSubsystem`** | 子系统接口：**Initialize** / **Shutdown** / **Tick(RuntimeFrameContext)** / **TickGroup**。 |
| **`RuntimeScheduler`** | **RegisterSubsystem**、**InitializeRegistered** / **ShutdownRegistered**、**Tick**。 |
| **`RuntimeTickGroup`** | **EarlyUpdate** / **FixedUpdate** / **Update** / **LateUpdate** / **Render**。 |

---

## 6. 开发进展

| 日期 | 进展 |
|------|------|
| 2026-05-17 | 文档与 **NN/Neverness** 命名对齐（无行为变更）。 |
| 2026-05-14 | Phase 4 首包：静态库与子系统骨架；供 **EngineServices** 转发 ABI。 |
| 2026-05-15 | **§2.7.1 Kernel 首包**：**EntitySubsystem**；**layout v5**；**P0-1 RuntimeScheduler** 首包。 |

---

## 7. 相关链接

- [Runtime 总览](../../RUNTIME_ARCHITECTURE_AND_PROGRESS.md)
- [NNNativeEngineAPI](../NNNativeEngineAPI/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
- [NNRuntimeEngineServices](../NNRuntimeEngineServices/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
