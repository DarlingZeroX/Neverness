# NNRuntimeManaged — Native ↔ Managed ABI 基础层（Phase 2/3）

## 1. 定位

| 项目 | 说明 |
|------|------|
| **职责** | 定义并实现 **VisionGal Native ↔ Managed 共享 ABI**：**`NNNativeAPI`** 函数表、默认 Native 实现（如 **`LogInfo`**）、默认 API 表单例、建表服务（**`NNNativeApiTable_BuildDefault`**）。**Phase 3**：表尾挂载 **`engineServices`** → **`NNNativeEngineAPI`**（见 **NNNativeEngineAPI**）。**不包含** CoreCLR 启动、hostfxr、程序集加载（见 **NNRuntimeManagedHost**）。 |
| **不负责** | Gameplay、对白、Editor、Hot Reload、Roslyn、反射式 Invoke、扩展元数据。 |
| **CMake 目标** | **`NevernessRuntime-Managed`**（**`SHARED`**）；别名 **`NNRuntimeManaged`** |
| **依赖** | **`NevernessRuntime-NativeEngineAPI`**、**`NevernessRuntime-NativeEngineAPIStub`**（**`PUBLIC`**）；可选 **`NevernessRuntime-EngineServices`**（`NEVERNESS_USE_ENGINE_RUNTIME_SERVICES=ON`）。**不**链接 `nethost`。 |

**托管镜像（C#）** 位于 [`Engine/Source/Managed/Runtime/Core/`](../../../Managed/Runtime/Core/)（程序集 **`NevernessRuntimeManaged-Core`**，命名空间 **`Neverness.Managed.Core`**），不在本目录。

---

## 2. 目录结构

```
Engine/Source/Runtime/NNRuntimeManaged/
├── CMakeLists.txt
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md   ← 本文件
├── Include/                    ← PUBLIC 头文件根（#include "NativeAPI.h"）
│   ├── NNRuntimeManagedConfig.h
│   ├── NativeAPI.h
│   ├── ManagedHandle.h
│   ├── ManagedABI.h
│   ├── ManagedRuntimeServices.h
│   └── ManagedExports.h
└── Private/
    ├── NativeAPI.cpp
    ├── ManagedRuntimeServices.cpp
    └── ManagedExports.cpp
```

---

## 3. 公开 C ABI 摘要

| 符号 | 说明 |
|------|------|
| `NN_NATIVE_API_VERSION` | 与托管 `NNNativeApiConstants.ApiVersion` 对齐（**当前为 2**）。 |
| `NNNativeAPI` | `apiVersion`、`reserved0`、`logInfo`、**`engineServices`**（`const NNNativeEngineAPI*`，可为 nullptr；默认表由建表函数填写）。 |
| `NNNativeApi_DefaultLogInfo` | 默认 `logInfo`：写 stderr + 内部诊断计数。 |
| `NNNativeApi_GetLogInfoCallCount` | 诊断：累计 `DefaultLogInfo` 调用次数。 |
| `NNNativeApiTable_BuildDefault` | 填充默认表并挂载 **`NNNativeEngineApi_GetDefaultTable()`** 或 Runtime 表。 |
| `NNNativeApi_RegisterLogInfoOverride` | Phase 2 占位（未改变默认表）。 |
| `NNNativeApi_GetDefaultTable` | 返回进程内只读单例表指针（从 **NNRuntimeManaged.dll** 导出）。 |

托管镜像与安装逻辑见 **`Neverness.Managed.Core`**；引擎子表镜像见 **`Neverness.Managed.Engine`**。

---

## 4. 与 NNRuntimeManagedHost 的边界

- **NNRuntimeManagedHost**（可选）：加载 CoreCLR、解析 **`[UnmanagedCallersOnly]`**，将 **`NNNativeApi_GetDefaultTable()`** 传给托管 **`Entry.BootstrapNativeApi`**；运行目录需 **NevernessRuntime-ManagedHost.dll** + **NevernessRuntime-Managed.dll**。
- **NNRuntimeManaged**：提供表与 Native 侧函数实现；**不**包含 hostfxr。

---

## 5. Phase 路线图（本模块）

| Phase | 内容 |
|-------|------|
| **2** | **`NNNativeAPI`** v1、`LogInfo` 闭环、托管 **`NativeApiBootstrap`**。 |
| **3（当前）** | **`NN_NATIVE_API_VERSION` = 2**、**`engineServices`** 指针、与 **NNNativeEngineAPI** 建表串接。 |
| **4+** | 扩充表字段（版本递增）、**`RegisterLogInfoOverride`** 真正接线、与 Gameplay 合约对齐之共享 struct（迁入 **`ManagedABI.h`** 需评审）。 |

---

## 6. 开发进展

| 日期 | 进展 |
|------|------|
| **2026-05-14** | **Phase 2 落地**：`NNNativeAPI`、默认 Log、单例表、**NevernessRuntimeManaged-Core**、Runtime **`BootstrapNativeApi`**、扩展 **VGManagedHostTest**。 |
| **2026-05-14** | **Phase 3**：递增 **`NN_NATIVE_API_VERSION`**、挂载 **NNNativeEngineAPI**、托管 **Neverness.Managed.Engine**、Stub 跨边界测试。 |
| **2026-05-18** | 自 **VGManagedCore**（`Managed/` STATIC）迁至 **`Runtime/NNRuntimeManaged`**（**SHARED**）；**`Include/`** 为头文件根；C# 仍位于 **`Managed/Runtime/Core`**。 |
| **2026-05-18** | C ABI 统一 **`NN*`** 前缀（曾用名 **`VGNativeAPI`** / **`VGNativeApi_*`**）；托管镜像 **`NNNativeApi`** / **`NNNativeApiConstants`**。 |
