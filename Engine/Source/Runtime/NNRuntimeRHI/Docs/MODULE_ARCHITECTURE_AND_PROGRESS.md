# VGRHI — 渲染硬件抽象（OpenGL）

## 1. 定位与边界

| 项目 | 说明 |
|------|------|
| **职责** | 提供 OpenGL 侧 **设备、网格、纹理、着色器、渲染目标** 等抽象接口（`Interface/`）；实现位于 `Source/OpenGL/` 等子目录。 |
| **不负责** | 不负责游戏帧循环与场景裁剪（见 **VGEngine**）；不负责 ImGui 即时模式封装（见 **VGImgui**）。 |
| **CMake 目标** | `VGRHI`（`SHARED` 或依 CMake 配置，以 [`CMakeLists.txt`](../CMakeLists.txt) 为准） |
| **依赖** | `HCore`（PRIVATE）；平台 OpenGL 加载在消费者或本模块内链接。 |
| **典型消费者** | **VGCore**、**VGUI**、**VGEngine**。 |

---

## 2. 构建与选项

- 包含目录：**PRIVATE** `Engine/Source/Runtime`、`Engine/Source/Kernel`、`Include`、`Interface`（以 CMake 为准）。
- Windows 等平台通常需链接 **opengl32** 或通过 **SDL** 创建 GL 上下文（由上层协调）。

---

## 3. 目录结构（摘要）

```
Engine/Source/Runtime/VGRHI/
├── CMakeLists.txt
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md   ← 本文件
├── Interface/                 ← RHI 抽象 API
├── Include/OpenGL/            ← GL 具体类型与扩展
└── Source/OpenGL/             ← 实现
```

---

## 4. 使用说明

### 4.1 包含方式

```cpp
#include <VGRHI/Interface/Device.h>
#include <VGRHI/Interface/Texture.h>
```

依赖方须将 **`Engine/Source/Runtime`** 加入包含路径（多数引擎目标已 **PRIVATE** 包含该路径），以便解析 `VGRHI/...` 前缀。

### 4.2 线程与 GL 上下文

- OpenGL 典型要求：**同一 OpenGL 上下文绑定线程** 上发起 draw/upload；跨线程需 `shared context` 或显式同步策略。
- 资源释放须在上下文仍有效时完成，避免驱动 UB。

---

## 5. 接口与 API 文档（`Interface/`）

| 头文件 | 主要职责 |
|--------|----------|
| [`Device.h`](../Interface/Device.h) | GPU 设备/上下文能力、交换与帧同步入口。 |
| [`Texture.h`](../Interface/Texture.h) | 纹理创建、上传、视图。 |
| [`Shader.h`](../Interface/Shader.h) | 着色器编译与参数绑定。 |
| [`Mesh.h`](../Interface/Mesh.h) | 顶点缓冲与图元批次。 |
| [`VertexElement.h`](../Interface/VertexElement.h) | 顶点布局描述。 |
| [`VGFX.h`](../Interface/VGFX.h) | 高层图形辅助/命名空间入口（以头文件为准）。 |

成员与枚举以各头文件声明为准。

---

## 6. 开发进展

| 日期 | 进展 |
|------|------|
| 2026-05-15 | 文档首版：Interface 索引与线程注意点。 |

---

## 7. 相关链接

- [Runtime 总览](../../RUNTIME_ARCHITECTURE_AND_PROGRESS.md)
- [VGImgui](../VGImgui/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
