# VGUI — RmlUi 与 Lua 驱动的 UI 运行时

## 1. 定位与边界

| 项目 | 说明 |
|------|------|
| **职责** | 基于 **RmlUi** 与 **SDL3** 构建 **SHARED** UI 库；集成 **Lua/Sol** 元素（CMake 中 `Include/Lua`、`Include/Sol`、`Include/Rml`）；对外 **`Interface/UISystem.h`**、**`UIDocument.h`**。 |
| **不负责** | 不负责 ImGui 工具栈（**VGImgui**）；不负责游戏玩法状态机（**VGEngine**）。 |
| **CMake 目标** | `VGUI`（`SHARED`） |
| **宏** | `VG_UI_EXPORT` |
| **依赖** | `SDL3`、`SDL3_image`、`RmlUi::RmlUi`、`HCore`、`HCorePlatform`、`HFileSystem`、`VGLua`、`VGCore`、`HMedia`、`VGRHI`（PRIVATE）。 |
| **包含传播** | **`target_include_directories(VGUI PUBLIC ... VGLua/Include)`** —— 链接 **VGUI** 的目标会自动获得 **sol/Lua** 头路径。 |
| **典型消费者** | **VGEngine**。 |

---

## 2. 构建与选项

- 需要 **RmlUi**、**SDL3**、**SDL3_image** CMake 包。
- 包含目录：**PRIVATE** Runtime 根、`Include`、`Interface`；**PUBLIC** `VGLua/Include`。

---

## 3. 目录结构（摘要）

```
Engine/Source/Runtime/VGUI/
├── CMakeLists.txt
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md   ← 本文件
├── Interface/
├── Include/
│   ├── Lua/、Sol/、Rml/       ← 绑定与元素头
│   └── …
└── Source/
    ├── Lua/、Sol/、Rml/
    └── Sol/Elements/
```

---

## 4. 使用说明

### 4.1 包含方式

```cpp
#include <VGUI/Interface/UISystem.h>
#include <VGUI/Interface/UIDocument.h>
```

并链接 **`VGUI`**（将自动 **PUBLIC** 继承 **VGLua** 头路径）。

### 4.2 与 RmlUi 文档生命周期

- **`UIDocument`**：对应单个 RML 文档实例；加载/重载规则见头文件。
- **`UISystem`**：全局 UI 子系统入口（字体、上下文、输入路由等 —— 以实现为准）。

### 4.3 线程

RmlUi 更新与渲染默认在主线程；与 **OpenGL** 交互时注意与 **VGRHI** 的上下文一致性。

---

## 5. 接口与 API 文档（`Interface/`）

| 头文件 | 主要职责 |
|--------|----------|
| [`UISystem.h`](../Interface/UISystem.h) | UI 子系统：文档管理、字体、输入、与渲染后端挂钩。 |
| [`UIDocument.h`](../Interface/UIDocument.h) | 单文档上下文、元素树访问。 |

`Include/Lua`、`Include/Sol` 下的 **元素** 类型以各自头文件为准（如 `Elements/*.h`）。

---

## 6. 开发进展

| 日期 | 进展 |
|------|------|
| 2026-05-15 | 文档首版。 |

---

## 7. 相关链接

- [Runtime 总览](../../RUNTIME_ARCHITECTURE_AND_PROGRESS.md)
- [VGEngine](../VGEngine/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
- [VGLua](../VGLua/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
