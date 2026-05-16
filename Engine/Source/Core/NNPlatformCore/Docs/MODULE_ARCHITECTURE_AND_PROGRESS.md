# HCorePlatform — 平台抽象层

## 1. 定位与边界

| 项目 | 说明 |
|------|------|
| **职责** | 封装 **SDL3** 窗口与输入、系统计时器、剪贴板、区域设置、CPU 信息、消息框、文件监视（efsw 风格源码树）、原生文件对话框等 **平台相关** 能力。 |
| **不负责** | 不负责通用数学/序列化（见 **HCore**）；不负责 VFS 业务策略（见 **HFileSystem**）。 |
| **CMake 目标** | `HCorePlatform`（`STATIC`） |
| **依赖** | `HCore`、`SDL3::SDL3`（均为 **PUBLIC** —— 公共头若直接 `#include <SDL3/...>`，依赖方须能解析 SDL 头并链接 SDL）。 |
| **典型消费者** | **VGCore**、**VGUI**、**VGAsset**、**VGEngine**、**VGImgui** 等。 |

---

## 2. 构建与选项

- `target_include_directories`：**PUBLIC** `Engine/Source/Kernel`（`VISIONGAL_KERNEL_ROOT`）、`Include`、`Interface`。
- 未启用单独的 `H_CORE_PLATFORM_EXPORT` 动态导出宏（见 CMake 注释）。

---

## 3. 目录结构（摘要）

```
Engine/Source/Kernel/HCorePlatform/
├── CMakeLists.txt
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md   ← 本文件
├── Interface/                 ← 对外 API（表见 §5）
├── Include/
│   ├── SDL3/                  ← SDL 头/封装（vendored 或桥接头）
│   ├── FileWatcher/           ← 文件监视实现
│   └── NativeFileDialog/      ← 原生对话框
└── Source/
```

**第三方 / 内嵌**

- **SDL3**：以 vcpkg/CMake `find_package(SDL3)` 链接；头文件布局见 `Include/SDL3`。
- **FileWatcher / NativeFileDialog**：以源码树形式内嵌，行为以各子目录 README/头注释为准。

---

## 4. 使用说明

### 4.1 包含路径

```cpp
#include <HCorePlatform/Interface/HWindow.h>
```

需同时链接 **`HCorePlatform`**（并因 **PUBLIC** 依赖继承 **HCore** 与 **SDL3::SDL3**）。

### 4.2 线程与主循环

窗口与输入回调的线程语义遵循 **SDL** 惯例：除非另行说明，事件泵送应在 **创建窗口的同一线程** 执行。

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

详细成员函数、结构与错误码以各头文件声明为准。

---

## 6. 开发进展

| 日期 | 进展 |
|------|------|
| 2026-05-15 | 文档首版：Interface 索引与 CMake 依赖说明。 |

---

## 7. 相关链接

- [Kernel 总览](../../KERNEL_ARCHITECTURE_AND_PROGRESS.md)
- [Runtime 总览](../../../Runtime/RUNTIME_ARCHITECTURE_AND_PROGRESS.md)
- [HCore](../HCore/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
