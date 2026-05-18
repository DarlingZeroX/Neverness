# NevernessRuntimeManaged-Core — 托管 ABI 镜像（Phase 2/3）

## 1. 定位

| 项目 | 说明 |
|------|------|
| **职责** | **NNNativeAPI** 的 C# 逐字段镜像、**`NativeApiBootstrap`**（禁止 `DllImport` 调引擎，经函数表间接调用）、**`NNNativeApiConstants`** 版本常量。 |
| **不负责** | Native 表构建、CoreCLR 宿主、Engine Service 子表定义（见 **Neverness.Managed.Engine**）。 |
| **程序集** | **`NevernessRuntimeManaged-Core`**（`net10.0`） |
| **命名空间** | **`Neverness.Managed.Core`** |
| **Native 契约** | [`Runtime/NNRuntimeManaged`](../../../../Runtime/NNRuntimeManaged/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)（**`NevernessRuntime-Managed`** SHARED） |

---

## 2. 目录结构

```
Engine/Source/Managed/Runtime/Core/
├── NevernessRuntimeManaged-Core.csproj
├── NNNativeApiConstants.cs
├── NNNativeApi.cs              ← 镜像 Runtime/NNRuntimeManaged/Include/NativeAPI.h
├── NativeApiBootstrap.cs
└── Docs/
    └── MODULE_ARCHITECTURE_AND_PROGRESS.md   ← 本文件
```

---

## 3. 与 VGManagedHost 的边界

- **`Entry.BootstrapNativeApi`**（**NevernessRuntimeManaged-Runtime**）接收 Native 传入的 **`NNNativeAPI*`**，委托 **`NativeApiBootstrap.Install`**。
- 表指针由 **`NNNativeApi_GetDefaultTable()`**（**NNRuntimeManaged.dll**）提供，不由本程序集导出。

---

## 4. 开发进展

| 日期 | 进展 |
|------|------|
| **2026-05-14** | **Phase 2**：**VisionGal.Managed.Core** 首包；后重命名为 **NevernessRuntimeManaged-Core**。 |
| **2026-05-14** | **Phase 3**：**`EngineServices`** 字段与 **Neverness.Managed.Engine** 对齐。 |
| **2026-05-18** | Native ABI 迁至 **NNRuntimeManaged**；本模块仅保留 C# 镜像与 Bootstrap。 |
