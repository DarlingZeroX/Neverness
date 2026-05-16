# VGEngineRuntimeServices — Engine Service ABI 适配层

## 1. 定位与边界

| 项目 | 说明 |
|------|------|
| **职责** | 实现 **`VGNativeEngineApiTable_BuildRuntime`**：以 **`VGNativeEngineApiTable_BuildDefault`** 为基底，覆写 **Timing**、**AsyncWait**、**Scene**（含层级/变换/命名）、**Asset**（`loadTexture` / `loadAudio`）、**Object**、**AssetRegistry**、**`entity.*`**（**`getServiceAbiToken`** / **`getRuntimeTick`**，转发 **`VGEngineRuntime::Instance().Entity()`** / **EntitySubsystem**）等字段。提供 **`VGNativeEngineApi_GetRuntimeTable`** 与宿主辅助 **`VGEngineRuntimeHost_*`**。 |
| **不负责** | 不取代纯 Stub 测试目标（**`VGNativeEngineApi_GetDefaultTable`** 仍可用于 Stub-only）；**`entity.*`** 在默认表上仍为 Stub；Render / UI / Audio / Input 等仍沿用 Stub 函数指针，直至未来 Adapter 替换。不链接 **VGEngine**。 |
| **CMake 目标** | `VGEngineRuntimeServices`（`STATIC`） |
| **依赖** | `VGNativeEngineAPI`、`VGEngineRuntime`（均为 PUBLIC）。 |
| **典型消费者** | **VGManagedCore**（根 CMake `VISIONGAL_USE_ENGINE_RUNTIME_SERVICES=ON` 时，默认 Native 表挂载 `engineServices`）。 |

---

## 2. 构建与选项

| 选项 | 默认 | 说明 |
|------|------|------|
| `VISIONGAL_USE_ENGINE_RUNTIME_SERVICES` | ON | **`VGNativeApiTable_BuildDefault`**（VGManagedCore）将 **`engineServices`** 设为 **`VGNativeEngineApi_GetRuntimeTable()`**；OFF 时使用 **`VGNativeEngineApi_GetDefaultTable()`** 的 Stub-only 表。 |

---

## 3. 目录结构

```
Engine/Source/Runtime/VGEngineRuntimeServices/
├── CMakeLists.txt
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md   ← 本文件
├── Include/VGEngineRuntimeServices/
│   └── NativeEngineRuntimeServices.h
└── Private/
    └── VGEngineRuntimeServices.cpp
```

---

## 4. 使用说明

### 4.1 包含方式（C API）

```cpp
#include "VGEngineRuntimeServices/NativeEngineRuntimeServices.h"
```

依赖 **`VGNativeEngineAPI/NativeEngineAPI.h`** 所间接包含的类型（由实现翻译单元已包含 `EngineAPIRegistry.h`）。

### 4.2 构建 Runtime 表

```cpp
VGNativeEngineAPI table{};
VGNativeEngineApiTable_BuildRuntime(&table);
```

或直接使用进程静态单例：

```cpp
const VGNativeEngineAPI* api = VGNativeEngineApi_GetRuntimeTable();
```

`GetRuntimeTable` 内部使用 **`std::call_once`** 调用 `BuildRuntime`，指针生命周期为 **进程静态**。

### 4.3 宿主驱动（C）

| 符号 | 说明 |
|------|------|
| `bool VGEngineRuntimeHost_Initialize(void)` | 转发 `VGEngineRuntime::Instance().Initialize()`。 |
| `void VGEngineRuntimeHost_Tick(float deltaTimeSeconds)` | 转发 `Tick`。 |
| `void VGEngineRuntimeHost_Shutdown(void)` | 转发 `Shutdown`。 |

须与 [VGEngineRuntime 文档](../VGEngineRuntime/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md) 中的 **线程/Shutdown 契约** 一致使用。

### 4.4 与 VGManagedCore 的关系

**VGManagedCore** 在链接本目标时，将 **`VGNativeAPI.engineServices`** 指向 **`VGNativeEngineApi_GetRuntimeTable()`**（受 `VISIONGAL_USE_ENGINE_RUNTIME_SERVICES` 控制）。托管镜像见 [MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md](../../../Managed/MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md)。

### 4.5 覆写字段（实现摘要）

实现见 [`VGEngineRuntimeServices.cpp`](../Private/VGEngineRuntimeServices.cpp)：在 `BuildRuntime` 中于默认 Stub 表上赋值：

- `timing.*` → `TimingSystem`
- `asyncWait.*` → `AsyncSystem`
- `scene.*` → `SceneSubsystem`
- `asset.loadTexture` / `loadAudio` → `AssetSubsystem`
- `object.*` → `ObjectSubsystem`
- `assetRegistry.*` → `AssetRegistrySubsystem`
- **`entity.*`** → **`EntitySubsystem`**（**`getServiceAbiToken`** 魔数；**`getRuntimeTick`** 观测 **Tick** 驱动；**layout v5**；见 `VGEngineRuntimeServices.cpp` 文件头注释）

其余子表保持 Stub 指针。

---

## 5. 接口与 API 文档（`NativeEngineRuntimeServices.h`）

| 符号 | 签名 | 说明 |
|------|------|------|
| `VGNativeEngineApiTable_BuildRuntime` | `void(VGNativeEngineAPI* outTable)` | 以 Stub 为基底覆写 Runtime 相关字段；`outTable == nullptr` 时 no-op。 |
| `VGNativeEngineApi_GetRuntimeTable` | `const VGNativeEngineAPI*(void)` | 进程内懒初始化单例表。 |
| `VGEngineRuntimeHost_Initialize` | `bool(void)` | 初始化 Runtime。 |
| `VGEngineRuntimeHost_Tick` | `void(float deltaTimeSeconds)` | 帧推进。 |
| `VGEngineRuntimeHost_Shutdown` | `void(void)` | 关闭。 |

---

## 6. 开发进展

| 日期 | 进展 |
|------|------|
| 2026-05-14 | Phase 4 首包：适配层与 Runtime 建表路径。 |
| 2026-05-15 | **layout v5**：**`BuildRuntime`** 覆写 **`entity.*`** 至 **EntitySubsystem**；**`rt_entity_*`** 转发与源码内 **Stub / Runtime** 分流注释；与 MANAGED **§2.7.1 Kernel 首包** 对齐。（**layout v4** 阶段曾暂不覆写 **`entity`**，已废止。） |
| 2026-05-15 | **P0-1**：**VGEngineRuntimeHost_Tick** 路径未变；**VGEngineRuntime** 内部改为 **Timing → RuntimeFrameContext → RuntimeScheduler**；**VGNativeEngineAPI** **layoutVersion** 仍为 **5**。 |
| 2026-05-15 | **§0 索引**：Managed **§0.3** **P0-1**（**VGRuntimeScheduler**）、**P0-3**（**VGSceneRuntime**）等为 Native Kernel 后续模块；当前本模块仍为 **VGNativeEngineAPI** 服务表覆写与 **EntitySubsystem** 可观测转发，见 [MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md](../../../Managed/MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md) **§0**。 |

---

## 7. 相关链接

- [Runtime 总览](../../RUNTIME_ARCHITECTURE_AND_PROGRESS.md)
- [VGNativeEngineAPI](../VGNativeEngineAPI/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
- [VGEngineRuntime](../VGEngineRuntime/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
