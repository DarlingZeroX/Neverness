# NNRuntimeCore — 应用核心动态库

> 曾用名：**VGCore**；CMake 目标 **`NevernessRuntime-Core`**；C++ 命名空间 **`NN::Runtime`**。

## 1. 定位与边界

| 项目 | 说明 |
|------|------|
| **职责** | 进程级 **应用壳**：窗口/视口、事件总线、输入、VFS 桥、Lua 辅助、与 **NevernessRuntime-Lua** / **NevernessRuntime-RHI** / **NevernessRuntime-ImGui** / **NevernessRuntime-Pak** / **NevernessCore** 等子系统绑定；对外暴露 **`Interface/`** 引擎接口与 **`Include/`** 内部子模块头。 |
| **不负责** | 不负责完整游戏循环与场景编辑（见 **NNEngineLegacy**，原 VGEngine）；不负责 RmlUi 文档模型（见 **NNRuntimeRmlui**，原 VGUI）。 |
| **CMake 目标** | `NevernessRuntime-Core`（`SHARED`） |
| **宏** | `VGCORE_DYNAMIC`、`VG_CORE_API_EXPORT`（历史宏名，未改） |
| **依赖** | `NevernessRuntime-Lua`、`SDL3`、`SDL3_image`、`NevernessCore-Core`、`NevernessCore-PlatformCore`、`NevernessCore-FileSystem`、`NevernessRuntime-ImGui`、`NevernessRuntime-RHI`、`NevernessRuntime-Pak`（均为 PRIVATE，见 CMake）。 |
| **典型消费者** | **NNEngineLegacy**、**NNEditor**、**NNLauncher**。 |

---

## 2. 构建与选项

- **预编译头**：`Include/pch.h`。
- **包含目录**：**PRIVATE** `Engine/Source/Runtime`、`Engine/Source/Core`、`Include`、`Interface`；CMake 中仍引用历史路径 `Engine/Source/Runtime/VGLua/Include`（物理目录为 **NNRuntimeLua**）。
- **SDL3 类型泄漏**：CMake 注释指出部分公共头暴露 SDL 类型，因此 **`SDL3::SDL3` 为 PUBLIC 链接**（见 `NNRuntimeCore/CMakeLists.txt` 注释）。

---

## 3. 目录结构（摘要）

```
Engine/Source/Runtime/NNRuntimeCore/
├── CMakeLists.txt
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md   ← 本文件
├── Interface/                 ← 对外引擎接口（优先）
├── Include/
│   ├── Core/                  ← Application、Window、Input、VFS…
│   ├── Data/                  ← DataContainer、DataVariant
│   └── Utils/                 ← LuaHelper、TimeHelper、TransitionHelper
├── Source/
└── VGCoreConfig.h             ← 历史配置文件名
```

---

## 4. 使用说明

### 4.1 链接与包含

```cmake
target_link_libraries(YourTarget PRIVATE NevernessRuntime-Core)
```

```cpp
#include "NNRuntimeCore/Interface/EngineInterface.h"
#include "NNRuntimeCore/Include/Core/Application.h"
```

实际 `#include` 前缀取决于消费者是否挂载 `Engine/Source/Runtime` 与 `Engine/Source/Core`（`#include <NNCore/...>` 等）。

### 4.2 生命周期

- **`Application` / `Window`**：遵循「构造 → Initialize → 每帧 Poll → Shutdown」顺序（以类实现为准）。
- **Lua**：通过 **`LuaHelper`** 管理 `sol::state` 与错误处理器。

---

## 5. 接口与 API 文档

### 5.1 `Interface/`（引擎边界）

| 头文件 | 主要职责 |
|--------|----------|
| [`EngineInterface.h`](../Interface/EngineInterface.h) | 引擎入口抽象。 |
| [`GameEngineInterface.h`](../Interface/GameEngineInterface.h) | 游戏引擎实例接口。 |
| [`GameInterface.h`](../Interface/GameInterface.h) | 游戏层回调。 |
| [`AppInterface.h`](../Interface/AppInterface.h) | 应用层回调。 |
| [`SceneInterface.h`](../Interface/SceneInterface.h) / [`ISceneFactory.h`](../Interface/ISceneFactory.h) | 场景加载与工厂。 |
| [`RenderInterface.h`](../Interface/RenderInterface.h) | 渲染抽象。 |
| [`AssetInterface.h`](../Interface/AssetInterface.h) / [`VGAsset.h`](../Interface/VGAsset.h) | 资源访问。 |
| [`FileInterface.h`](../Interface/FileInterface.h) / [`Loader.h`](../Interface/Loader.h) | 文件与加载管线。 |
| [`Interface.h`](../Interface/Interface.h) | 聚合/公共 typedef。 |
| [`EngineState.h`](../Interface/EngineState.h) | 引擎状态枚举或结构。 |

### 5.2 `Include/`（实现向但仍可被上层包含）

| 头文件 | 主要职责 |
|--------|----------|
| [`Core/Application.h`](../Include/Core/Application.h) | 应用主对象。 |
| [`Core/Window.h`](../Include/Core/Window.h) | 窗口与 GL 上下文创建。 |
| [`Core/Viewport.h`](../Include/Core/Viewport.h) | 视口矩形与 DPI。 |
| [`Core/Input.h`](../Include/Core/Input.h) | 输入状态缓存。 |
| [`Core/EventBus.h`](../Include/Core/EventBus.h) / [`Events.h`](../Include/Core/Events.h) | 事件系统。 |
| [`Core/VFS.h`](../Include/Core/VFS.h) | 虚拟文件系统桥。 |
| [`Core/CoreTypes.h`](../Include/Core/CoreTypes.h) | 核心类型。 |
| [`Data/DataContainer.h`](../Include/Data/DataContainer.h) / [`DataVariant.h`](../Include/Data/DataVariant.h) | 动态数据模型。 |
| [`Utils/LuaHelper.h`](../Include/Utils/LuaHelper.h) | Lua/sol 辅助。 |
| [`Utils/TimeHelper.h`](../Include/Utils/TimeHelper.h) / [`TransitionHelper.h`](../Include/Utils/TransitionHelper.h) | 时间与过渡。 |

---

## 6. 开发进展

| 日期 | 进展 |
|------|------|
| 2026-05-17 | 文档与 **NN/Neverness** 命名对齐（无行为变更）。 |
| 2026-05-15 | 文档首版：Interface/Include 双索引。 |

---

## 7. 相关链接

- [Runtime 总览](../../RUNTIME_ARCHITECTURE_AND_PROGRESS.md)
- [NNEngineLegacy](../NNEngineLegacy/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
- [NNRuntimeLua](../NNRuntimeLua/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
