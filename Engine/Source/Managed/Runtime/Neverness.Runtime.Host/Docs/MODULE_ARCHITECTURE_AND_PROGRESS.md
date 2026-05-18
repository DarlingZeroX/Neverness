# NevernessRuntimeManaged-Runtime — 托管入口程序集（C#）

## 1. 定位

| 项目 | 说明 |
|------|------|
| **职责** | **`[UnmanagedCallersOnly]`** 导出（`Entry.Smoke`、`BootstrapNativeApi`、`BootstrapEngineFoundation` 等）；聚合引用 Foundation / Gameplay / Entity 等 C# 模块；**dotnet publish** 根程序集。 |
| **不负责** | CoreCLR 启动、hostfxr（见 **[NNRuntimeManagedHost](../../../../Runtime/NNRuntimeManagedHost/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)**）；`NNNativeAPI` Native 实现（见 **NNRuntimeManaged**）。 |
| **程序集** | **`NevernessRuntimeManaged-Runtime`** |
| **命名空间** | **`Neverness.Managed.Runtime`** |

---

## 2. 目录结构

```
Engine/Source/Managed/Runtime/Host/
├── NevernessRuntimeManaged-Runtime.csproj
├── Entry.cs
└── Docs/
    └── MODULE_ARCHITECTURE_AND_PROGRESS.md
```

---

## 3. 与 Native 宿主边界

- Native **`VGManagedHost`**（**NevernessRuntime-ManagedHost**）解析 `Neverness.Managed.Runtime.Entry, NevernessRuntimeManaged-Runtime` 上的 UCO。
- **`BootstrapNativeApi`** 接收 **`NNNativeAPI*`**，委托 **Neverness.Managed.Core** 的 **`NativeApiBootstrap.Install`**。

---

## 4. 开发进展

| 日期 | 进展 |
|------|------|
| **2026-05-14** | Phase 1 Smoke + Phase 2 BootstrapNativeApi。 |
| **2026-05-18** | Native 宿主文档迁至 **NNRuntimeManagedHost**；本文件仅描述 C# 入口程序集。 |
