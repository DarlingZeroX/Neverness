# NNPlatformCore — 平台抽象层

> 曾用名：**HCorePlatform**；CMake 目标 **`NevernessCore-PlatformCore`**。

## 1. 定位与边界

| 项目 | 说明 |
|------|------|
| **职责** | 封装 **SDL3** 窗口与输入、系统计时器、剪贴板、区域设置、CPU 信息、消息框、文件监视（efsw 风格源码树）、原生文件对话框等 **平台相关** 能力。 |
| **不负责** | 不负责通用数学/序列化（见 **NNCore**）；不负责 VFS 业务策略（见 **NNFileSystem**）。 |
| **CMake 目标** | `NevernessCore-PlatformCore`（`STATIC`） |
| **依赖** | `NevernessCore-Core`、`SDL3::SDL3`（均为 **PUBLIC**）。 |
| **典型消费者** | **NevernessRuntime-Core**、**NevernessRuntime-RmlUI**、**NevernessRuntime-Asset**、**NevernessRuntime-EngineLegacy**、**NevernessRuntime-ImGui** 等。 |

---

## 2. 构建与选项

- `target_include_directories`：**PUBLIC** `Engine/Source/Core`（`VISIONGAL_KERNEL_ROOT`）、`Include`、`Interface`。

---

## 3. 目录结构（摘要）

```
Engine/Source/Core/NNPlatformCore/
├── CMakeLists.txt
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md   ← 本文件
├── Interface/
├── Include/
│   ├── SDL3/
│   ├── FileWatcher/
│   └── NativeFileDialog/
└── Source/
```

---

## 4. 使用说明

### 4.1 包含路径

```cpp
#include <NNPlatformCore/Interface/HWindow.h>
```

需同时链接 **`NevernessCore-PlatformCore`**（并因 **PUBLIC** 依赖继承 **NevernessCore-Core** 与 **SDL3::SDL3**）。

---

## 5. 接口与 API 文档（`Interface/`）

| 头文件 | 主要职责 |
|--------|----------|
| [`HWindow.h`](../Interface/HWindow.h) | 窗口创建、尺寸、交换链相关配合点。 |
| [`HInput.h`](../Interface/HInput.h) | 键盘/鼠标/手柄等输入抽象。 |
| [`HTimer.h`](../Interface/HTimer.h) / [`HSystemTimer.h`](../Interface/HSystemTimer.h) | 计时与帧计时器。 |
| [`HFileWatcher.h`](../Interface/HFileWatcher.h) | 监视目录/文件变更。 |
| [`HMessageBox.h`](../Interface/HMessageBox.h) | 系统消息框。 |
| [`HClipboard.h`](../Interface/HClipboard.h) | 剪贴板读写。 |
| [`HCpuInfo.h`](../Interface/HCpuInfo.h) | CPU 能力查询。 |
| [`HLocale.h`](../Interface/HLocale.h) | 区域与语言环境。 |
| [`HSystemMisc.h`](../Interface/HSystemMisc.h) | 杂项系统调用封装。 |

---

## 6. 开发进展

| 日期 | 进展 |
|------|------|
| 2026-05-17 | 文档与 **NN/Neverness** 命名对齐（无行为变更）。 |
| 2026-05-15 | 文档首版。 |

---

## 7. 相关链接

- [Core 总览](../../KERNEL_ARCHITECTURE_AND_PROGRESS.md)
- [Runtime 总览](../../../Runtime/RUNTIME_ARCHITECTURE_AND_PROGRESS.md)
- [NNCore](../NNCore/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
