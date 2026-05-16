# HMedia — 音视频媒体库

## 1. 定位与边界

| 项目 | 说明 |
|------|------|
| **职责** | 基于 **FFmpeg** 封装音频/视频解码与播放相关能力；暴露 **Interface/** 中的音视频接口头文件；实现分布于 `Source/Audio`、`Source/Video`、`Source/FFmpeg`、`Source/SDL` 等。 |
| **不负责** | 不负责游戏级音频总线与 3D 衰减（见 **VGEngine** 音频子系统）；不负责 UI 视频控件绑定（见 **VGUI**）。 |
| **CMake 目标** | `HMedia`（`SHARED`） |
| **宏** | `H_MEDIA_API_EXPORT`（`add_definitions`） |
| **依赖** | **FFmpeg**（`find_package(FFMPEG REQUIRED)`，包含目录与库目录由 CMake 注入）、`HCore`（PRIVATE）。 |
| **典型消费者** | **VGEngine**、**VGCore**、**VGAsset**、**VGUI**（均 PRIVATE 链接 HMedia，见各自 CMakeLists）。 |

---

## 2. 构建与选项

- 需要本机/vcpkg 提供 **FFmpeg** 开发包；否则 CMake 配置阶段失败。
- 包含目录：**PUBLIC** `Engine/Source/Kernel`（`VISIONGAL_KERNEL_ROOT`）、`Include`。

---

## 3. 目录结构（摘要）

```
Engine/Source/Kernel/HMedia/
├── CMakeLists.txt
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md   ← 本文件
├── Interface/
├── Include/
│   ├── Audio/
│   ├── Video/
│   ├── Common/
│   ├── SDL/
│   └── FFmpeg/
└── Source/
    ├── Audio/
    ├── Video/
    ├── Common/
    ├── SDL/
    └── FFmpeg/
```

---

## 4. 使用说明

### 4.1 链接与包含

```cpp
#include <HMedia/Interface/AudioInterface.h>
#include <HMedia/Interface/VideoInterface.h>
```

链接 **`HMedia`** 并确保 FFmpeg 运行时可被加载（部署依赖依平台而定）。

### 4.2 线程与实时性

解码与同步语义以实现为准；默认假设 **不在** 音频设备回调中执行重负载分配。

---

## 5. 接口与 API 文档（`Interface/`）

| 头文件 | 主要职责 |
|--------|----------|
| [`AudioInterface.h`](../Interface/AudioInterface.h) | 音频播放/解码抽象。 |
| [`AudioDecoderInterface.h`](../Interface/AudioDecoderInterface.h) | 音频解码器接口。 |
| [`VideoInterface.h`](../Interface/VideoInterface.h) | 视频纹理/帧推送抽象。 |
| [`VideoDecoderInterface.h`](../Interface/VideoDecoderInterface.h) | 视频解码器接口。 |

类与函数签名以头文件为准。

---

## 6. 开发进展

| 日期 | 进展 |
|------|------|
| 2026-05-15 | 文档首版：FFmpeg 依赖与 Interface 索引。 |

---

## 7. 相关链接

- [Kernel 总览](../../KERNEL_ARCHITECTURE_AND_PROGRESS.md)
- [Runtime 总览](../../../Runtime/RUNTIME_ARCHITECTURE_AND_PROGRESS.md)
- [HCore](../HCore/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
