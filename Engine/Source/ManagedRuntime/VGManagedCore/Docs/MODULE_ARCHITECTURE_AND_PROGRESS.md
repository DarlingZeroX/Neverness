# VGManagedCore — Managed Runtime 基础层（Phase 2：Native/Managed ABI）

## 1. 定位

| 项目 | 说明 |
|------|------|
| **职责** | 定义并实现 **VisionGal Native ↔ Managed 共享 ABI**：`VGNativeAPI` 函数表、默认 Native 实现（如 `LogInfo`）、句柄/枚举等 POD 占位、默认 API 表单例、构建服务（`VGNativeApiTable_BuildDefault`）。**不包含** CoreCLR 启动、hostfxr、程序集加载（见 **VGManagedHost**）。 |
| **不负责** | Gameplay、Dialogue、Editor、Hot Reload、Roslyn、反射式 Invoke、扩展元数据。 |
| **CMake 目标** | **`VGManagedCore`**（**`STATIC`**） |
| **依赖** | 仅 C++ 标准库；**不**链接 `nethost`。 |

---

## 2. 目录结构

```
Engine/Source/ManagedRuntime/VGManagedCore/
├── CMakeLists.txt
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md   ← 本文件
├── Include/VGManagedCore/
│   ├── VGManagedCoreConfig.h
│   ├── NativeAPI.h            ← VGNativeAPI、VG_NATIVE_API_VERSION、默认 LogInfo
│   ├── ManagedHandle.h        ← VGEntityHandle 等 64-bit 不透明句柄占位
│   ├── ManagedABI.h           ← VGLogChannel 等小枚举 / 填充占位
│   ├── ManagedRuntimeServices.h
│   └── ManagedExports.h       ← VGNativeApi_GetDefaultTable
├── Private/
│   ├── NativeAPI.cpp
│   ├── ManagedRuntimeServices.cpp
│   └── ManagedExports.cpp
└── Managed/VisionGal.Managed.Core/
    ├── VisionGal.Managed.Core.csproj
    ├── VGNativeApiConstants.cs
    ├── VGNativeApi.cs
    └── NativeApiBootstrap.cs
```

---

## 3. 公开 C ABI 摘要

| 符号 | 说明 |
|------|------|
| `VG_NATIVE_API_VERSION` | 与托管 `VGNativeApiConstants.ApiVersion` 对齐。 |
| `VGNativeAPI` | `apiVersion`、`reserved0`、`logInfo`（`VGNativeLogInfoFn`）。 |
| `VGNativeApi_DefaultLogInfo` | 默认 `logInfo`：写 stderr + 内部诊断计数。 |
| `VGNativeApi_GetLogInfoCallCount` | 诊断：累计 `DefaultLogInfo` 调用次数。 |
| `VGNativeApiTable_BuildDefault` | 填充默认表。 |
| `VGNativeApi_RegisterLogInfoOverride` | Phase 2 占位（未改变默认表）。 |
| `VGNativeApi_GetDefaultTable` | 返回进程内只读单例表指针。 |

托管镜像与安装逻辑见 **`VisionGal.Managed.Core`**（随本模块源码树维护，由 **`VisionGal.Managed.Runtime`** 引用）。

---

## 4. 与 VGManagedHost 的边界

- **VGManagedHost**：加载 CoreCLR、解析 `[UnmanagedCallersOnly]`，将 `VGNativeApi_GetDefaultTable()` 的指针传给托管 `Entry.BootstrapNativeApi`。
- **VGManagedCore**：提供表与 Native 侧函数实现；**不**包含 hostfxr。

---

## 5. Phase 路线图（本模块）

| Phase | 内容 |
|-------|------|
| **2（当前）** | `VGNativeAPI` v1、`LogInfo` 闭环、托管 `NativeApiBootstrap`、GTest `BootstrapNativeApiCallsNativeLogInfo`。 |
| **3+** | 扩展表字段（版本递增）、可选 `RegisterLogInfoOverride` 真正接线、与 Gameplay 合约对齐的共享 struct（迁入 `ManagedABI.h` 需谨慎评审）。 |

---

## 6. 开发进展

| 日期 | 进展 |
|------|------|
| **2026-05-14** | **Phase 2 落地**：新增 **VGManagedCore** 静态库、`VGNativeAPI`、默认 Log、单例表、**VisionGal.Managed.Core**、Runtime `BootstrapNativeApi`、扩展 **VGManagedHostTest**。 |
