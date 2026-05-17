# NNMediaCore — 音视频媒体库

> 曾用名：**HMedia**；CMake 目标 **`NevernessCore-MediaCore`**。

## 1. 定位与边界

| 项目 | 说明 |
|------|------|
| **职责** | 基于 **FFmpeg** 封装音频/视频解码与播放相关能力；暴露 **Interface/** 中的音视频接口头文件。 |
| **不负责** | 不负责游戏级音频总线与 3D 衰减（见 **NNEngineLegacy**）；不负责 UI 视频控件绑定（见 **NNRuntimeRmlui**）。 |
| **CMake 目标** | `NevernessCore-MediaCore`（`SHARED`） |
| **宏** | `H_MEDIA_API_EXPORT`（历史宏名） |
| **依赖** | **FFmpeg**（`find_package(FFMPEG REQUIRED)`）、`NevernessCore-Core`（PRIVATE）。 |
| **典型消费者** | **NevernessRuntime-EngineLegacy**、**NevernessRuntime-Core**、**NevernessRuntime-Asset**、**NevernessRuntime-RmlUI**。 |

---

## 2. 构建与选项

- 需要本机/vcpkg 提供 **FFmpeg** 开发包。
- 包含目录：**PUBLIC** `Engine/Source/Core`（`VISIONGAL_KERNEL_ROOT`）、`Include`。

---

## 3. 目录结构（摘要）

```
Engine/Source/Core/NNMediaCore/
├── CMakeLists.txt
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md   ← 本文件
├── Interface/
├── Include/
└── Source/
```

---

## 4. 使用说明

### 4.1 链接与包含

```cpp
#include <NNMediaCore/Interface/AudioInterface.h>
#include <NNMediaCore/Interface/VideoInterface.h>
```

链接 **`NevernessCore-MediaCore`** 并确保 FFmpeg 运行时可被加载。

---

## 5. 接口与 API 文档（`Interface/`）

| 头文件 | 主要职责 |
|--------|----------|
| [`AudioInterface.h`](../Interface/AudioInterface.h) | 音频播放/解码抽象。 |
| [`AudioDecoderInterface.h`](../Interface/AudioDecoderInterface.h) | 音频解码器接口。 |
| [`VideoInterface.h`](../Interface/VideoInterface.h) | 视频纹理/帧推送抽象。 |
| [`VideoDecoderInterface.h`](../Interface/VideoDecoderInterface.h) | 视频解码器接口。 |

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
