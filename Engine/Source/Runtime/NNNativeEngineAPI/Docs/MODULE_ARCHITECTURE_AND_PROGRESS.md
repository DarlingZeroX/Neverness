# NNNativeEngineAPI — Native Engine Service ABI（纯契约层）

> 曾用名：**VGNativeEngineAPI**；CMake 目标 **`NevernessRuntime-NativeEngineAPI`**（**`INTERFACE`**，仅头文件）；别名 **`NNNativeEngineAPI`**。C 类型与导出符号统一为 **`NN*`**（如 **`NNRenderAPI`**、**`NNNativeEngineApi_GetDefaultTable`**）。

## 1. 定位与边界

| 项目 | 说明 |
|------|------|
| **职责** | **纯 ABI Contract Layer**：POD、`typedef`、各 `*API.h` 子表、`NNNativeEngineAPI` 聚合体、`NN_NATIVE_ENGINE_API_LAYOUT_VERSION`、`extern "C"` 函数指针 typedef、调用约定宏。**禁止**在本模块内放置 `unordered_map`、`mutex`、Stub 实现或行程单例。 |
| **不负责** | Stub / Mock / `BuildDefault` / `GetDefaultTable` → 见 **[NNRuntimeNativeEngineAPIStub](../NNRuntimeNativeEngineAPIStub/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)**。Runtime 转发 → **[NNRuntimeEngineServices](../NNRuntimeEngineServices/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)**。 |
| **CMake 目标** | `NevernessRuntime-NativeEngineAPI`（`INTERFACE`） |
| **依赖** | 无（仅头文件传播） |
| **典型消费者** | 任意需要 **Handle / 子表布局** 的模块；实现方链接 **Stub** 或 **EngineServices**。 |

---

## 2. 构建与选项

| 项目 | 说明 |
|------|------|
| 导出宏 | （预留）`NN_NATIVE_ENGINE_API`；INTERFACE 目标下通常为空。 |
| 调用约定 | [`NativeInterop.h`](../Include/NativeInterop.h)：`NN_ENGINE_ABI_STDCALL`。 |

根 **`NEVERNESS_USE_ENGINE_RUNTIME_SERVICES`** 决定 **NNRuntimeManaged** 挂载 `NNNativeEngineApi_GetRuntimeTable()` 或 `NNNativeEngineApi_GetDefaultTable()`（符号由 Stub / EngineServices 模块提供）。

---

## 3. 目录结构

```
Engine/Source/Runtime/NNNativeEngineAPI/
├── CMakeLists.txt
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md
└── Include/
    ├── EngineAPIRegistry.h      ← NNNativeEngineAPI 聚合体 + layoutVersion
    ├── NativeEngineAPI.h        ← 伞形 include
    ├── ApplicationAPI.h         ← SDL 生命周期 + 事件泵 + 帧边界
    ├── WindowAPI.h / WindowTypes.h  ← 多窗口 + Native 句柄
    └── *API.h
```

**无 `Private/` 实现翻译单元。**

---

## 4. 使用说明

### 4.1 包含方式

```cpp
#include "EngineAPIRegistry.h"
// 或
#include "NativeEngineAPI.h"
```

### 4.2 填充函数表

1. 链接 **`NevernessRuntime-NativeEngineAPIStub`** 或 **`NevernessRuntime-EngineServices`**。
2. 调用 **`NNNativeEngineApiTable_BuildDefault`** 或 **`NNNativeEngineApiTable_BuildRuntime`**。
3. 将 **`const NNNativeEngineAPI*`** 挂到 **NNRuntimeManaged** 的 `NNNativeAPI.engineServices`。

**布局演进规则**：仅允许在各子表尾部或聚合体尾部 **追加** 字段；破坏性变更时递增 **`NN_NATIVE_ENGINE_API_LAYOUT_VERSION`**。

---

## 5. 接口摘要

| 符号 | 所在模块 |
|------|----------|
| `NNNativeEngineAPI` / `NN_NATIVE_ENGINE_API_LAYOUT_VERSION` | 本模块 **Include**（当前 **layout v10**） |
| `NNApplicationAPI` | SDL Host 生命周期（无窗口 CRUD） |
| `NNWindowAPI` / `NNWindowHandle` | 窗口子系统 |
| `NNVfsAPI` | VFS IO + 相对路径 + Native FS 刷新 |
| `NNNativeEngineApiTable_BuildDefault` | **NNRuntimeNativeEngineAPIStub** |
| `NNNativeEngineApiTable_BuildRuntime` | **NNRuntimeEngineServices** |

---

## 6. 开发进展

| 日期 | 进展 |
|------|------|
| 2026-05-21 | **layout v10**：**`NNVfsAPI`** 追加 `getAbsolutePath`。 |
| 2026-05-21 | **layout v9**：**`NNVfsAPI`** 追加 `getRelativePath` / `rebuildNativeFileSystemFiles`。 |
| 2026-05-21 | **layout v8**：**`NNVfsAPI`**（ReadText/WriteText/ReadBytes + freeBuffer）。 |
| 2026-05-21 | **layout v7**：**`NNApplicationAPI`** 瘦身；新增 **`NNWindowAPI`** / **`WindowTypes.h`**。 |
| 2026-05-18 | **ABI / Stub 解耦**：本模块改为 INTERFACE。 |
| 2026-05-15 | **layout v4/v5**、**EntityAPI**。 |
| 2026-05-20 | **layout v6**：**`NNApplicationAPI`**（含 openWindow，已由 v7 移除）。 |

---

## 7. 相关链接

- [NNRuntimeApplication](../NNRuntimeApplication/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
- [NNRuntimeNativeEngineAPIStub](../NNRuntimeNativeEngineAPIStub/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
- [Runtime 总览](../../RUNTIME_ARCHITECTURE_AND_PROGRESS.md)
