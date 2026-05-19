# NNRuntimeManagedHost — CoreCLR 托管宿主（可选 Native 模块）

## 1. 定位

| 项目 | 说明 |
|------|------|
| **职责** | 启动 CoreCLR、程序集加载、`load_assembly_and_get_function_pointer`、多程序集登记、**Native → Managed** 的 UCO 解析；封装 **nethost / hostfxr**，公共头 **不暴露** `hostfxr.h`。 |
| **不负责** | `NNNativeAPI` 定义（见 **NNRuntimeManaged**）；Gameplay / Editor 业务逻辑（**C# 主导**，见 `Managed/Runtime`）。 |
| **CMake 目标** | **`NevernessRuntime-ManagedHost`**（`SHARED`）；别名 **`NNRuntimeManagedHost`**、兼容 **`VGManagedHost`** |
| **依赖** | vcpkg **`nethost`**；**`PRIVATE`** **`NevernessRuntime-Managed`** |
| **构建选项** | **`VISIONGAL_ENABLE_MANAGED_HOST`**（定义于 [Runtime/CMakeLists.txt](../../CMakeLists.txt)），**默认 OFF**；仅 Win/MSVC 可启用 |

**C# 入口程序集**（`[UnmanagedCallersOnly]`）位于 [`Managed/Runtime/Neverness.Runtime.Host/`](../../../Managed/Runtime/Neverness.Runtime.Host/)（**Neverness.Runtime.Host**），不在本目录；由 solution / `dotnet build` 产出至 **`Build/bin/<Config>/`**，本模块不触发 C# 编译。

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

- 运行目录需同时存在 **`NevernessRuntime-ManagedHost.dll`** 与 **`NevernessRuntime-Managed.dll`**（与 C# 程序集同属 **`Build/bin/<Config>/`**，无需 POST_BUILD 拷贝）。
- **`NNNativeApi_GetDefaultTable()`** 由 **NNRuntimeManaged** 导出；宿主将表指针传入托管 **`Entry.BootstrapNativeApi`**。
- 公共 C++ 类名仍为 **`VGManagedHost`**（稳定门面）；CMake 目标已更名为 **NevernessRuntime-ManagedHost**。

---

## 4. 构建

```bat
cmake -B build -DVISIONGAL_ENABLE_MANAGED_HOST=ON
dotnet build Engine/Source/Managed/Runtime/Neverness.Runtime.Host/Neverness.Runtime.Host.csproj -c Debug
cmake --build build --config Debug --target NevernessRuntime-ManagedHost
```

| 项目 | 说明 |
|------|------|
| **C# 产出** | **`Neverness.Runtime.Host`** 等 → **`Build/bin/Debug/`**（与 C++ 统一输出目录） |
| **验证** | 托管侧以 **dotnet test**（如 **VisionGal.Managed.Foundation.Tests**）为主；本模块不再提供 C++ 集成测试目标。 |

---

## 5. 开发进展

| 日期 | 进展 |
|------|------|
| **2026-05-14** | **Phase 1**：`VGManagedHost` 模块、nethost、UCO Smoke。 |
| **2026-05-18** | 自 `Managed/VGManagedHost` 迁至 **`Runtime/NNRuntimeManagedHost`**；**`Include/`** 为头文件根；**`VISIONGAL_ENABLE_MANAGED_HOST` 默认 OFF**；选项移至 **Runtime/CMakeLists.txt**。 |
| **2026-05-19** | 移除 **`Test/`** 与 **`VGManagedHostTest`**；托管宿主验证改由 **dotnet test** 承担。 |
