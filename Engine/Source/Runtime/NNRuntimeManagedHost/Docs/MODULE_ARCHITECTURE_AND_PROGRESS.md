# NNRuntimeManagedHost — CoreCLR 托管宿主（可选 Native 模块）

## 1. 定位

| 项目 | 说明 |
|------|------|
| **职责** | 启动 CoreCLR、程序集加载、`load_assembly_and_get_function_pointer`、多程序集登记、**Native → Managed** 的 UCO 解析；封装 **nethost / hostfxr**，公共头 **不暴露** `hostfxr.h`。 |
| **不负责** | `NNNativeAPI` 定义（见 **NNRuntimeManaged**）；Gameplay / Editor 业务逻辑（**C# 主导**，见 `Managed/Runtime`）。 |
| **CMake 目标** | **`NevernessRuntime-ManagedHost`**（`SHARED`）；别名 **`NNRuntimeManagedHost`**、兼容 **`VGManagedHost`** |
| **依赖** | vcpkg **`nethost`**；**`PRIVATE`** **`NevernessRuntime-Managed`** |
| **构建选项** | **`VISIONGAL_ENABLE_MANAGED_HOST`**（定义于 [Runtime/CMakeLists.txt](../../CMakeLists.txt)），**默认 OFF**；仅 Win/MSVC 可启用 |

**C# 入口程序集**（`[UnmanagedCallersOnly]`）位于 [`Managed/Runtime/Host/`](../../../Managed/Runtime/Host/)（**NevernessRuntimeManaged-Runtime**），不在本目录。

---

## 2. 目录结构

```
Engine/Source/Runtime/NNRuntimeManagedHost/
├── CMakeLists.txt
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md
├── Include/                         ← PUBLIC 头文件根
│   ├── NNRuntimeManagedHostConfig.h
│   ├── ManagedHost.h                ← class VGManagedHost
│   ├── ManagedRuntime.h
│   ├── ManagedAssembly.h
│   ├── ManagedFunction.h
│   ├── ManagedContext.h
│   └── ManagedInterop.h
└── Private/
    ├── CoreCLRLoader.{h,cpp}
    └── ...
```

---

## 3. 部署与边界

- 运行目录需同时存在 **`NevernessRuntime-ManagedHost.dll`**（或 VS 输出名 `NevernessRuntime-ManagedHost.dll`）与 **`NevernessRuntime-Managed.dll`**。
- **`NNNativeApi_GetDefaultTable()`** 由 **NNRuntimeManaged** 导出；宿主将表指针传入托管 **`Entry.BootstrapNativeApi`**。
- 公共 C++ 类名仍为 **`VGManagedHost`**（稳定门面）；CMake 目标已更名为 **NevernessRuntime-ManagedHost**。

---

## 4. 构建与测试

```bat
cmake -B build -DVISIONGAL_ENABLE_MANAGED_HOST=ON -DENABLE_TESTS=ON
cmake --build build --config Debug --target NevernessRuntime-ManagedHost visiongal_managed_runtime_publish VGManagedHostTest
```

| 项目 | 说明 |
|------|------|
| **publish** | `visiongal_managed_runtime_publish` → `${CMAKE_BINARY_DIR}/ManagedRuntimePublish` |
| **测试** | **`VGManagedHostTest`**（`ENABLE_TESTS` + `dotnet`）；`POST_BUILD` 拷贝 Host 与 Managed DLL |

---

## 5. 开发进展

| 日期 | 进展 |
|------|------|
| **2026-05-14** | **Phase 1**：`VGManagedHost` 模块、nethost、UCO Smoke。 |
| **2026-05-18** | 自 `Managed/VGManagedHost` 迁至 **`Runtime/NNRuntimeManagedHost`**；**`Include/`** 为头文件根；**`VISIONGAL_ENABLE_MANAGED_HOST` 默认 OFF**；选项移至 **Runtime/CMakeLists.txt**。 |
