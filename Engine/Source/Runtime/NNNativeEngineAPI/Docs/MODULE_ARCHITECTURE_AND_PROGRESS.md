# NNNativeEngineAPI — Native Engine Service ABI

> 曾用名：**VGNativeEngineAPI**；CMake 目标 **`NevernessRuntime-NativeEngineAPI`**；C 类型与导出符号仍为 **`VG*`**（托管互操作稳定层）。

## 1. 定位与边界

| 项目 | 说明 |
|------|------|
| **职责** | 承载 **Engine Runtime 服务层** 的 C 可互操作 **函数表聚合**（`VGNativeEngineAPI`）：Render、UI、Audio、Asset、Input、Scene、Timing、AsyncWait、Object、AssetRegistry、**Entity（Kernel 首包：魔数 + `getRuntimeTick` 观测）**。提供 **Stub** 实现与测试计数；不包含 Gameplay、对白、变量、存档、Sequence、Editor 产品逻辑。 |
| **不负责** | 不链接 **NevernessRuntime-EngineLegacy** / **NevernessRuntime-RHI** / **NevernessRuntime-RmlUI**。真实能力由未来 Adapter 覆写函数指针；**Timing / Async / Scene / Asset（纹理与音频快捷项）/ Object / AssetRegistry** 的 Runtime 转发由 **NNRuntimeEngineServices** 在可选 CMake 路径下覆写。 |
| **CMake 目标** | `NevernessRuntime-NativeEngineAPI`（`STATIC`） |
| **依赖** | 仅 C++ 标准库；`target_include_directories(... PUBLIC Include)`。 |
| **典型消费者** | **NevernessRuntime-Engine**（PUBLIC 链接以使用 Handle 等头）、**NevernessRuntime-EngineServices**、**VGManagedCore**（经 `engineServices` 指针挂载表）。 |

---

## 2. 构建与选项

| 项目 | 说明 |
|------|------|
| 导出宏 | [`VGNativeEngineApiConfig.h`](../Include/VGNativeEngineApiConfig.h) 中 `VG_NATIVE_ENGINE_API`；当前为静态库时通常为空。 |
| 调用约定 | [`NativeInterop.h`](../Include/NativeInterop.h)：`VG_ENGINE_ABI_STDCALL`（Windows 上为 `__stdcall`，与托管 `UnmanagedCallersOnly` 对齐）。 |

根 [`CMakeLists.txt`](../../../../../CMakeLists.txt) 中 **`VISIONGAL_USE_ENGINE_RUNTIME_SERVICES`**（默认 ON）决定 **VGManagedCore** 使用 `VGNativeEngineApi_GetRuntimeTable()` 或纯 Stub 的 `VGNativeEngineApi_GetDefaultTable()`，详见 [NNRuntimeEngineServices 文档](../../NNRuntimeEngineServices/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)。

---

## 3. 目录结构

```
Engine/Source/Runtime/NNNativeEngineAPI/
├── CMakeLists.txt
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md   ← 本文件
├── Include/
│   ├── NativeInterop.h
│   ├── EngineHandles.h
│   ├── EngineTypes.h
│   ├── RenderAPI.h
│   ├── UIAPI.h
│   ├── AudioAPI.h
│   ├── AssetAPI.h
│   ├── AssetRegistryAPI.h
│   ├── InputAPI.h
│   ├── ObjectAPI.h
│   ├── SceneAPI.h
│   ├── TimingAPI.h
│   ├── AsyncWaitAPI.h
│   ├── EntityAPI.h
│   ├── EngineAPIRegistry.h
│   ├── NativeEngineAPI.h
│   └── VGNativeEngineApiConfig.h
└── Private/
    └── VGNativeEngineApiStubs.cpp
```

---

## 4. 使用说明

### 4.1 包含方式

在已链接 `NevernessRuntime-NativeEngineAPI` 的目标中：

```cpp
#include "NNNativeEngineAPI/Include/NativeEngineAPI.h"
#include "NNNativeEngineAPI/Include/EngineAPIRegistry.h"
```

- 聚合根类型与建表 API 在 **`EngineAPIRegistry.h`** / **`NativeEngineAPI.h`**。
- 子域函数指针 typedef 见各 `*API.h`。

### 4.2 表构建与版本

1. 分配或栈上准备 `VGNativeEngineAPI table{}`。
2. 调用 **`VGNativeEngineApiTable_BuildDefault(&table)`** 填充 Stub 指针，并设置 **`table.layoutVersion = VG_NATIVE_ENGINE_API_LAYOUT_VERSION`**。
3. 将 **`const VGNativeEngineAPI*`** 挂到 **VGManagedCore** 的 `VGNativeAPI.engineServices`（或通过 **NNRuntimeEngineServices** 的 `VGNativeEngineApi_GetRuntimeTable()`）。

**布局演进规则**：仅允许在各子表尾部或聚合体尾部 **追加** 字段；禁止重排既有字段；破坏性变更时递增 **`VG_NATIVE_ENGINE_API_LAYOUT_VERSION`**（须与托管 `VGNativeEngineApiConstants.LayoutVersion` 对齐）。

### 4.3 字符串与线程

- 文档约定中的 **`const char*`** 参数须为 **NUL 结尾的 UTF-8**；`nullptr` 多为 no-op 或失败返回。
- **Render** 等子表：Stub 与默认 Adapter 假定为 **单线程或 RHI 单点** 调用；并发行为由具体实现定义。

### 4.4 与托管栈的关系

参见 [MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md](../../../Managed/MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md) 与程序集 **VisionGal.Managed.Engine**：托管侧镜像本 ABI 的函数表布局与 Handle 类型。

---

## 5. 接口与 API 文档

### 5.1 根符号（`extern "C"` 导出策略）

| 符号 | 说明 |
|------|------|
| `VG_NATIVE_ENGINE_API_LAYOUT_VERSION` | 当前聚合体内存布局版本（**layout v5** 起 **`VGEntityAPI`** 含 **`getRuntimeTick`**）。 |
| `VGNativeEngineAPI` | 根聚合体，含 `layoutVersion`、`reserved0` 与各子表。 |
| `VGNativeEngineApiTable_BuildDefault` | 将 `outTable` 清零后填入 Stub 函数指针。 |
| `VGNativeEngineApi_GetDefaultTable` | 进程内单例只读表指针（纯 Stub）。 |
| `VGNativeEngineApi_GetStubInvokeCount` | 测试：Stub 被调用累计次数。 |

### 5.2 Handle 类型（`EngineHandles.h`）

| typedef | 说明 |
|---------|------|
| `VGTextureHandle` | 纹理不透明 ID，`0` 无效。 |
| `VGRenderTargetHandle` | 渲染目标。 |
| `VGElementHandle` | UI 元素。 |
| `VGAudioHandle` | 音频。 |
| `VGAssetHandle` | 通用资源。 |
| `VGAsyncWaitHandle` | 异步等待。 |
| `VGEntityHandle` | 场景实体 / Prefab 实例。 |
| `VGObjectHandle` | 托管 `VGObject` 与 Native 桥接。 |

### 5.3 POD 类型（`EngineTypes.h`）

| 类型 | 字段摘要 |
|------|----------|
| `VGGuid` | `high`、`low`（128-bit GUID）。 |
| `VGTransform3` | `position[3]`、`rotation[3]`、`scale[3]`。 |

### 5.4 `VGNativeEngineAPI` 聚合体字段顺序

见 [`EngineAPIRegistry.h`](../Include/EngineAPIRegistry.h)（末尾 **`entity`** 为 **`VGEntityAPI`**）。

### 5.5 子表字段摘要

**Entity（`EntityAPI.h`，layout v4+）**

| 字段 | 说明 |
|------|------|
| `getServiceAbiToken` | 返回 **`VG_ENTITY_SERVICE_ABI_TOKEN`**。 |
| `getRuntimeTick` | **layout v5**；**`VGEngineRuntime::Tick`** 驱动 **EntitySubsystem**；**非**托管 **EntityWorld** 镜像。 |

其余子表（Timing、AsyncWait、Scene、Asset、Object、AssetRegistry、Render、UI、Audio、Input）见对应 `*API.h`。

---

## 6. 开发进展

| 日期 | 进展 |
|------|------|
| 2026-05-17 | 文档与 **NN/Neverness** 命名对齐（无行为变更）。 |
| 2026-05-14 | Phase 3：模块落地、子 API 头拆分、Stub。 |
| 2026-05-15 | **layout v4/v5**；**EntityAPI**；与 MANAGED **§2.7.1** 对齐。 |

---

## 7. 相关链接

- [Runtime 总览](../../RUNTIME_ARCHITECTURE_AND_PROGRESS.md)
- [NNRuntimeEngine](../NNRuntimeEngine/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
- [NNRuntimeEngineServices](../NNRuntimeEngineServices/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
