# NNRuntimeAsset — 游戏资产类型与加载

> 曾用名：**VGAsset**；CMake 目标 **`NevernessRuntime-Asset`**；C++ 命名空间 **`NN::Runtime`**。

## 1. 定位与边界

| 项目 | 说明 |
|------|------|
| **职责** | 提供 **Galgame** 等资源类型（音频/视频片段、场景访问器等）、**`AssetManager`** 与工厂；头文件主要在 **`Interface/`**。 |
| **不负责** | 不负责 Pak 物理格式（**NNRuntimePak**）；不负责 FFmpeg 底层解码（**NevernessCore-MediaCore**）。 |
| **CMake 目标** | `NevernessRuntime-Asset`（`SHARED`） |
| **宏** | `VG_ASSET_EXPORT`（历史宏名） |
| **依赖** | `SDL3`、`SDL3_image`、`NevernessCore-Core`、`NevernessCore-PlatformCore`、`NevernessCore-FileSystem`、`NevernessRuntime-Core`、`NevernessCore-MediaCore`（PRIVATE）。 |
| **典型消费者** | **NevernessRuntime-EngineLegacy**。 |

---

## 2. 构建与选项

- 包含目录：**PRIVATE** `Engine/Source/Runtime`、`Engine/Source/Core`、`Include`、`Interface`。

---

## 3. 目录结构（摘要）

```
Engine/Source/Runtime/NNRuntimeAsset/
├── CMakeLists.txt
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md   ← 本文件
├── Interface/
├── Include/                   ← 具体资产类声明（若有）
└── Source/
```

---

## 4. 使用说明

### 4.1 包含方式

```cpp
#include "NNRuntimeAsset/Interface/AssetManager.h"
#include "GalGameAsset.h"
```

`GalGameAsset.h` 位于 [`Include/GalGameAsset.h`](../Include/GalGameAsset.h)（非 `Interface/`）；包含路径以链接目标的 CMake 配置为准。

### 4.2 与 NNRuntimeCore / 虚拟文件系统

资产加载通常通过 **NNRuntimeCore** 暴露的 VFS/文件接口访问包内路径；详见 [`AssetManager.h`](../Interface/AssetManager.h) 注释。

---

## 5. 接口与 API 文档（`Interface/`）

| 头文件 | 主要职责 |
|--------|----------|
| [`AssetManager.h`](../Interface/AssetManager.h) | 资源索引、异步/同步加载策略（以实现为准）。 |
| [`AssetFactory.h`](../Interface/AssetFactory.h) | 类型注册与实例创建。 |
| [`AudioClip.h`](../Interface/AudioClip.h) / [`VideoClip.h`](../Interface/VideoClip.h) | 音视频资产句柄。 |
| [`SceneAccessor.h`](../Interface/SceneAccessor.h) | 场景数据访问器。 |
| [`ISceneSerializer.h`](../Interface/ISceneSerializer.h) / [`SceneSerializerFactory.h`](../Interface/SceneSerializerFactory.h) | 场景序列化插件点。 |
| [`Package.h`](../Interface/Package.h) | 与包体协同的辅助声明（若存在与 **NNRuntimePak** 重叠，以头文件注释区分职责）。 |

### 5.2 `Include/`（补充）

| 头文件 | 主要职责 |
|--------|----------|
| [`GalGameAsset.h`](../Include/GalGameAsset.h) | Galgame 资产聚合类型或工厂（以实现为准）。 |

---

## 6. 开发进展

| 日期 | 进展 |
|------|------|
| 2026-05-17 | 文档与 **NN/Neverness** 命名对齐（无行为变更）。 |
| 2026-05-15 | 文档首版。 |

---

## 7. 相关链接

- [Runtime 总览](../../RUNTIME_ARCHITECTURE_AND_PROGRESS.md)
- [NNRuntimePak](../NNRuntimePak/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
- [NNMediaCore](../../Core/NNMediaCore/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
