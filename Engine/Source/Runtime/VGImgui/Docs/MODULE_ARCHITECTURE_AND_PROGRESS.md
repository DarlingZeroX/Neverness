# VGImgui — ImGui 与编辑器向 UI 扩展

## 1. 定位与边界

| 项目 | 说明 |
|------|------|
| **职责** | 集成 **Dear ImGui**、**ImGuiColorTextEdit**、**imnodes**、**ImNodeEditor**、**ImGuizmo**、自研 **ImGuiEx** / **ImGuiLayer** 等，构建 **SHARED** 库；供工具、调试 UI、编辑器组件使用。 |
| **不负责** | 不负责游戏内 RmlUi 界面（见 **VGUI**）；不负责主渲染帧图（见 **VGEngine**）。 |
| **CMake 目标** | `VGImgui`（`SHARED`） |
| **依赖** | `Freetype::Freetype`、`SDL3::SDL3`、`HCore`、`HCorePlatform`；平台 **OpenGL**（PRIVATE）。 |
| **宏** | `IMGUI_EXPORT`（`add_definitions`） |
| **典型消费者** | **VGCore**、编辑器相关目标。 |

---

## 2. 构建与选项

- 使用全局 `include_directories(Runtime 根 + 本模块 Include)`（见 CMake）；新目标建议逐步迁移为 `target_include_directories` 风格（工程卫生项，非本文件范围）。
- 需要 **Freetype** 与 **SDL3** 包。

---

## 3. 目录结构（摘要）

```
Engine/Source/Runtime/VGImgui/
├── CMakeLists.txt
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md   ← 本文件
├── Include/
│   ├── Imgui/                 ← Dear ImGui 核心
│   ├── ImGuiEx/               ← 扩展控件、窗口、字体与任务接口
│   ├── ImGuiLayer/            ← SDL3 装饰器、窗口装饰
│   ├── ImGuiColorTextEdit/    ← 代码编辑器控件
│   ├── Imnodes/、ImNodeEditor/、imGuizmo/
│   └── pch.h、imconfig.h
└── Source/                    ← 与 Include 子目录镜像
```

**第三方内嵌**

- **ImGui** 及生态插件以源码树形式位于 `Include/`/`Source/`；上游文档见 [Dear ImGui](https://github.com/ocornut/imgui)。

---

## 4. 使用说明

### 4.1 包含方式

```cpp
#include <Imgui/imgui.h>
#include <ImGuiEx/ImWindow.h>
```

需链接 **`VGImgui`** 并确保 **OpenGL + SDL3** 初始化顺序与 ImGui 后端一致（`imgui_impl_sdl3.h`、`imgui_impl_opengl3.h`）。

### 4.2 线程

ImGui **单线程帧模型**：`NewFrame` → 业务窗口 → `Render` 必须在同一线程；与游戏线程交互需队列化。

---

## 5. 接口与 API 文档（代表性头文件）

| 头文件 | 说明 |
|--------|------|
| [`Imgui/imgui.h`](../Include/Imgui/imgui.h) | ImGui 核心 API。 |
| [`ImGuiEx/ImWindow.h`](../Include/ImGuiEx/ImWindow.h) | 窗口封装与布局辅助。 |
| [`ImGuiEx/ImGuiEx.h`](../Include/ImGuiEx/ImGuiEx.h) | 杂项扩展入口。 |
| [`ImGuiLayer/SDL3Decorator.h`](../Include/ImGuiLayer/SDL3Decorator.h) | 与 SDL3 窗口集成。 |
| [`ImNodeEditor/imgui_node_editor.h`](../Include/ImNodeEditor/imgui_node_editor.h) | 节点编辑器画布。 |
| [`Imnodes/imnodes.h`](../Include/Imnodes/imnodes.h) | 轻量节点编辑。 |
| [`imGuizmo/ImGuizmo.h`](../Include/imGuizmo/ImGuizmo.h) | 视口 Gizmo。 |
| [`ImGuiColorTextEdit/TextEditor.h`](../Include/ImGuiColorTextEdit/TextEditor.h) | 文本编辑控件。 |

完整列表见 CMake `file(GLOB IMGUI_SRC_FILES ...)`。

---

## 6. 开发进展

| 日期 | 进展 |
|------|------|
| 2026-05-15 | 文档首版。 |

---

## 7. 相关链接

- [Runtime 总览](../../RUNTIME_ARCHITECTURE_AND_PROGRESS.md)
- [VGCore](../VGCore/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
