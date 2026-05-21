# NNRuntimeEngineServices — Engine Service ABI 适配层（真实 Runtime Bridge）

> 曾用名：**VGEngineRuntimeServices**；CMake 目标 **`NevernessRuntime-EngineServices`**；别名 **`NNEngineRuntimeServices`**（C++ Facade **`NNEngineRuntime`**；Engine Service ABI 为 **`NN*`**）。

## 1. 定位与边界

| 项目 | 说明 |
|------|------|
| **职责** | **`NNNativeEngineApiTable_BuildRuntime`**：先 **`NNNativeEngineApiTable_BuildDefault`**（Stub 模块），再 **`NNBuild*RuntimeApi`** 覆写 Timing / Async / Scene / Asset（部分）/ Object / AssetRegistry / **entity.***。提供 **`NNNativeEngineApi_GetRuntimeTable`** 与 **`NNEngineRuntimeHost_*`**。 |
| **不负责** | 纯 Stub 表（**`NNNativeEngineApi_GetDefaultTable`**，见 Stub 模块）；Render / UI / Audio / Input 仍沿用 Stub 指针。 |
| **CMake 目标** | `NevernessRuntime-EngineServices`（`STATIC`） |
| **依赖** | `NevernessRuntime-NativeEngineAPI`、`NevernessRuntime-NativeEngineAPIStub`、`NevernessRuntime-Engine`（PUBLIC） |
| **典型消费者** | **NNRuntimeManaged**（`NEVERNESS_USE_ENGINE_RUNTIME_SERVICES=ON`） |

---

## 2. 目录结构

```
Engine/Source/Runtime/NNRuntimeEngineServices/
├── CMakeLists.txt
├── Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md
├── Include/NativeEngineRuntimeServices.h
└── Private/
    ├── Registry/NativeEngineRuntimeApiTable.cpp   ← BuildRuntime + GetRuntimeTable + Host_*
    ├── Internal/RuntimeApiBuilders.h
    ├── Timing/TimingRuntimeApi.cpp
    ├── Async/AsyncWaitRuntimeApi.cpp
    ├── Scene/SceneRuntimeApi.cpp
    ├── Asset/AssetRuntimeApi.cpp
    ├── Object/ObjectRuntimeApi.cpp
    ├── AssetRegistry/AssetRegistryRuntimeApi.cpp
    └── Entity/EntityRuntimeApi.cpp
```

---

## 3. 使用说明

```cpp
#include "NativeEngineRuntimeServices.h"
#include "NNRuntimeNativeEngineApiStub.h"  // 若需 BuildDefault / Stub 计数

const NNNativeEngineAPI* api = NNNativeEngineApi_GetRuntimeTable();
```

**`BuildRuntime` 流程**：`BuildDefault` → `NNBuildTimingRuntimeApi` → … → `NNBuildEntityRuntimeApi`（Registry 文件内无 thunk 实现）。

---

## 4. 开发进展

| 日期 | 进展 |
|------|------|
| 2026-05-18 | 依赖 **Stub** 模块；**NativeEngineRuntimeServices.cpp** 拆为多 TU + **`NNBuild*RuntimeApi`**；**`NNEngineRuntimeHost_*`** / CMake 别名 **`NNEngineRuntimeServices`**。 |
| 2026-05-15 | **entity.*** 转发 **EntitySubsystem**；**layout v5**。 |

---

## 5. 相关链接

- [NNRuntimeNativeEngineAPIStub](../NNRuntimeNativeEngineAPIStub/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
- [NNNativeEngineAPI](../NNNativeEngineAPI/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
- [NNRuntimeEngine](../NNRuntimeEngine/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
