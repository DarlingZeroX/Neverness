# NNRuntimeRenderAssets 模块架构与设计文档

## 目录

- [1. 模块概述](#1-模块概述)
- [2. 文件清单](#2-文件清单)
- [3. 核心类关系与职责](#3-核心类关系与职责)
- [4. 数据流与架构设计](#4-数据流与架构设计)
- [5. 与其他模块的集成](#5-与其他模块的集成)
- [6. C API 层（跨语言桥接）](#6-c-api-层跨语言桥接)
- [7. 关键设计决策](#7-关键设计决策)
- [8. 已知问题与可优化点](#8-已知问题与可优化点)
- [9. 开发进度与待办](#9-开发进度与待办)

---

## 1. 模块概述

`NNRuntimeRenderAssets` 是引擎的 **运行时渲染资产管理模块**，负责纹理资源从磁盘到 GPU 的全生命周期管理。它位于资产系统（`NNRuntimeAsset`）和渲染系统（`NNRuntimeRenderer2D`）之间，充当两者的桥梁。

**核心职责：**

- 将 `.nnasset` 资产文件中的纹理数据上传到 GPU
- 管理 GPU 纹理资源的缓存与生命周期
- 提供 GUID 到 GPU 纹理句柄的映射
- 实现基于帧的 LRU 驱逐策略
- 通过 C API 层为 Editor（C#）提供纹理访问能力

**构建产物：** 共享库 `NevernessRuntime-RenderAssets`（DLL）

**依赖关系：**

```
NevernessRuntime-RHI        ← 提供 OpenGL::Texture2D 等底层类型
NevernessRuntime-Asset      ← 提供 NNAssetManager、NNAssetHandleT
NevernessRuntime-NativeEngineAPI ← 提供 EngineTypes.h（NNGuid 等）
NevernessCore-Core (private) ← 提供 NN::Ref (shared_ptr)
```

---

## 2. 文件清单

```
NNRuntimeRenderAssets/
├── Include/
│   ├── NNRenderAssetManager.h      # 核心管理器（单例）
│   ├── NNTextureResource.h         # GPU 纹理资源包装器
│   ├── NNTextureSourceAsset.h      # CPU 侧纹理数据（导入产物）
│   ├── NNTextureFormat.h           # 引擎级纹理格式枚举
│   └── NNTextureCache.h            # LRU 缓存实现（独立组件）
├── Private/
│   ├── NNRenderAssetManager.cpp    # 管理器实现（GL 格式映射、上传、缓存）
│   ├── NNTextureResource.cpp       # 资源实现（ImGui 句柄、GPU 释放）
│   ├── NNTextureSourceAsset.cpp    # 二进制序列化/反序列化
│   └── NNTextureCache.cpp          # LRU 插入/获取/驱逐实现
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md  # 本文档
├── Test/                           # 测试目录（待填充）
├── CMakeLists.txt                  # 构建定义
└── NNRuntimeRenderAssetsExport.h   # DLL 导入/导出宏
```

---

## 3. 核心类关系与职责

本模块采用 **扁平组合** 架构，无继承关系，所有类通过组合协作。

### 3.1 NNTextureFormat（格式枚举）

文件：`Include/NNTextureFormat.h`

引擎级的纹理格式抽象，不绑定任何具体图形 API。

| 格式值 | 含义 |
|---|---|
| `R8_UNorm` ~ `RGBA8_UNorm` | 标准 8-bit 无符号归一化格式 |
| `R16_Float` ~ `RGBA32_Float` | 浮点格式（预留） |
| `BC1`, `BC3`, `BC7` | Block 压缩格式（预留） |
| `ETC2`, `ASTC` | 移动端压缩格式（预留） |

提供 `GetBytesPerPixel()` 内联工具函数。当前仅 R8/RG8/RGB8/RGBA8 被实际使用，其余为预留。

### 3.2 NNTextureSourceAsset（CPU 侧纹理数据）

文件：`Include/NNTextureSourceAsset.h`

导入管线的产物，表示内存中的完整纹理数据。

```
NNTextureSourceAsset
├── Width, Height        # 尺寸
├── Format               # NNTextureFormat
├── IsSRGB, HasAlpha     # 标志位
└── Mips[]               # NNMipLevel 数组
```

**NNMipLevel 结构体：** `Width` + `Height` + `vector<uint8_t> Pixels`（行主序、紧密排列）

**关键方法：**
- `SetFromDecodedImage()` — 从原始像素初始化（仅 base mip）
- `SetMips()` — 设置完整 mip 链
- `Serialize()` / `Deserialize()` — 二进制格式：`[Width:u32][Height:u32][Format:u32][Flags:u32][MipCount:u32][MipSizes...][Pixels...]`

### 3.3 NNTextureResource（GPU 纹理包装器）

文件：`Include/NNTextureResource.h`

GPU 纹理的轻量级包装，存储 **不透明指针** 以避免暴露原生句柄类型。

```
NNTextureResource
├── m_RHITexture           # void* — RHI 纹理指针
├── m_RHIShaderResourceView # void* — SRV 指针
├── m_Desc                 # NNTextureDesc（Width, Height, MipCount, Format, IsSRGB）
└── m_Residency            # NNTextureResidency 状态枚举
```

**关键设计：观察者模式**
- 资源本身 **不管理 GPU 内存**，析构函数和移动操作均不释放 GPU 资源
- GPU 生命周期完全由 `RenderAssetCacheEntry::OwnedTexture`（`shared_ptr<void>`）管理
- `GetImGuiHandle()` 将 SRV 指针 reinterpret_cast 为 `uint64_t`（OpenGL: GLuint; Diligent: ITextureView*）
- `NNRenderAssetManager` 声明为 `friend`，可直接访问内部状态

**NNTextureResidency 枚举：** `NotLoaded` / `Loading` / `Resident` / `Evicted` / `Error` — 为未来流式加载预留。

### 3.4 RenderAssetCacheEntry（缓存条目）

文件：`Include/NNRenderAssetManager.h`

```cpp
struct RenderAssetCacheEntry {
    std::shared_ptr<void> OwnedTexture;            // 类型擦除的纹理所有权
    std::unique_ptr<NNTextureResource> Resource;   // 公共接口资源
    size_t GPUMemory;                               // 估算 GPU 内存占用
};
```

**类型擦除所有权：** `OwnedTexture` 实际是 `shared_ptr<OpenGL::Texture2D>` 转换而来，捕获了原始类型的 deleter。这使得 RenderAssets 模块无需 `OpenGL::Texture2D` 的完整定义即可安全释放资源。

### 3.5 NNTextureCache（LRU 缓存，独立组件）

文件：`Include/NNTextureCache.h`

独立的 LRU 缓存实现，支持配置内存预算（默认 256 MB）。

```
内部结构：
├── m_Items     # unordered_map<uint64_t, CacheEntry> — O(1) 查找
├── m_LruList   # list<uint64_t> — LRU 顺序（front = 最近使用）
└── m_MaxBytes  # 内存预算上限
```

**关键方法：** `Insert()`, `Get()`, `Remove()`, `EvictToTarget()`

**重要：当前未被 NNRenderAssetManager 使用。** 管理器内部实现了自己的简化版 LRU 驱逐（线性扫描）。该类是为未来替换管理器内嵌 LRU 逻辑而准备的。

### 3.6 NNRenderAssetManager（核心管理器）

文件：`Include/NNRenderAssetManager.h`

线程安全的单例（通过 `static local` 实现），是整个模块的核心入口。

```
NNRenderAssetManager
├── m_EntryCache          # unordered_map<uint64_t, unique_ptr<RenderAssetCacheEntry>>
├── m_GuidToCacheKeyMap   # unordered_map<uint64_t, uint64_t> — GUID.Low → cacheKey
├── m_NextKey             # 自增 key 分配器（从 1 开始，0 = 无效）
├── m_CurrentFrame        # 帧计数器（用于 LRU 追踪）
└── m_Mutex               # 互斥锁（线程安全）
```

**公共接口：**

| 方法 | 说明 |
|---|---|
| `LoadTextureFromAsset(handle, guidLow)` | 从 `.nnasset` 资产句柄加载纹理 |
| `LoadTextureFromBlob(typeInfo, pixelData, guidLow)` | 从预解析的 blob 数据加载（绕过 AssetManager 单例问题） |
| `CreateTextureFromPixels(pixels, w, h, name)` | 从原始 RGBA8 像素创建纹理 |
| `GetCacheKeyByGuidLow(guidLow)` | 通过 GUID 查询缓存 key |
| `GetGLTextureId(cacheKey)` | 获取 OpenGL 纹理 ID |
| `GetImGuiTextureHandle(cacheKey)` | 获取 ImGui 兼容句柄 |
| `ReleaseTexture(cacheKey)` | 释放 GPU 纹理 |
| `ReloadTextureFromPixels(cacheKey, pixels, w, h)` | 热重载纹理 |
| `EvictLRU()` | LRU 驱逐（当前为线性扫描） |

---

## 4. 数据流与架构设计

### 4.1 纹理加载完整路径

```
NNSpriteRendererComponent.TextureAsset (NNGuid)
    │
    ▼
SpriteRenderSystem::Collect()
    │
    ├── TextureRuntimeId == 0 && TextureAsset.low != 0 ?
    │   │
    │   ├── 是：尝试 GetCacheKeyByGuidLow(guidLow)
    │   │   ├── 已缓存 → 直接返回 cacheKey
    │   │   └── 未缓存：
    │   │       ├── NNAssetManager::LoadAssetSync(guid, typeId=1)
    │   │       ├── NNAssetManager::GetBlobByType(handle, TYPE_INFO) → NNTextureTypeInfo*
    │   │       ├── NNAssetManager::GetBlobByType(handle, DATA) → uint8_t* pixels
    │   │       └── NNRenderAssetManager::LoadTextureFromBlob(typeInfo, pixels, guidLow)
    │   │           ├── 校验 width/height/format
    │   │           ├── 构造 NNTextureSourceAsset
    │   │           ├── UploadTextureInternal(source)
    │   │           │   ├── MapToGLFormat() → GL internal format
    │   │           │   ├── OpenGL::Texture2D::CreateFromMemory(desc)
    │   │           │   ├── 创建 NNTextureResource（存储原始指针）
    │   │           │   └── 存入 RenderAssetCacheEntry
    │   │           │       （shared_ptr<void> 持有 Texture2D 所有权）
    │   │           └── 注册 guidLow → cacheKey 映射，返回 cacheKey
    │   │
    │   └── GetGLTextureId(cacheKey) → GLuint → 存入 sprite.TextureRuntimeId
    │
    ▼
SpriteDrawCommand.TextureHandle = TextureRuntimeId（或 0 = 白色 fallback）
    │
    ▼
Renderer2D::Submit()
    ├── glActiveTexture(GL_TEXTURE0)
    ├── glBindTexture(GL_TEXTURE_2D, TextureHandle)
    └── glDrawElements(GL_TRIANGLES, 6, ...)  // 单次绘制 = 一个 Sprite
```

### 4.2 核心架构模式

**1. 类型擦除所有权（Type-erased Ownership）**

```
shared_ptr<OpenGL::Texture2D>  ──→  shared_ptr<void>（捕获 deleter）
                                      │
                                      └── RenderAssetCacheEntry::OwnedTexture
```

NNTextureResource 持有 `void*` 观察者指针，不拥有 GPU 资源。真正的所有权在 cache entry 的 `shared_ptr<void>` 中。这解耦了模块对 RHI 具体类型的编译依赖。

**2. 惰性加载（Lazy Loading）**

纹理在 **第一帧渲染时** 才被加载。SpriteRenderSystem 检测到 `TextureRuntimeId == 0` 时触发加载，加载完成后将结果缓存回组件。无需预加载管线。

**3. GUID 去重**

`m_GuidToCacheKeyMap` 提供 O(1) 查找，避免同一纹理被重复上传到 GPU。GUID 使用 `NNGuid.low`（64 位）作为键。

**4. 帧级 LRU 驱逐**

`m_CurrentFrame` 计数器追踪当前帧号，用于判断缓存条目的"新鲜度"。驱逐时优先移除最久未访问的条目。

---

## 5. 与其他模块的集成

### 5.1 与 NNRuntimeAsset（资产系统）的集成

通过 `NNAssetManager` 加载 `.nnasset` 文件，解析其中的 blob 数据：

```
.nnasset 文件
└── NNAssetHeader (96 bytes)
    ├── NN_BLOB_TYPE_TYPE_INFO → NNTextureTypeInfo（width, height, format, mipCount...）
    └── NN_BLOB_TYPE_DATA     → 原始像素数据
```

### 5.2 与 NNRuntimeRenderer2D（渲染器）的集成

- **SpriteRenderSystem**：负责 ECS 查询和纹理惰性解析，将解析后的 GLuint 写入 `SpriteDrawCommand`
- **Renderer2D**：接收 `SpriteDrawCommand` 向量，执行 OpenGL 绘制调用
- **SceneRenderer**：顶层编排器，管理帧缓冲区和绘制顺序

### 5.3 与 C# Editor 的集成

通过 `NNRenderAssetAPI` 函数指针表（C ABI, `__stdcall`）桥接，Editor 无需链接 DLL 即可访问渲染资源。

### 5.4 跨模块单例问题

**问题描述：** `NNAssetManager` 定义在 `NevernessRuntime-Asset` DLL 中。每个 DLL 边界可能获得独立的单例实例。

**解决方案：** `SpriteRenderSystem` 在 **Renderer2D 模块** 内调用 `NNAssetManager::Instance()` 提取 blob 指针，然后通过 `LoadTextureFromBlob()` 传递原始指针，避免 RenderAssets 模块访问到不同的单例实例。

**替代方案（已实现）：** `LoadTextureFromBlob()` 方法接受预解析的 `typeInfoData` 和 `pixelData` 指针，完全绕过 `NNAssetManager`。

---

## 6. C API 层（跨语言桥接）

文件：`Engine/Source/Runtime/NNNativeEngineAPI/Include/RenderAssetAPI.h`

`NNRenderAssetAPI` 是一个函数指针结构体（C ABI, `__stdcall`），由 `NNBuildRenderAssetRuntimeApi()` 填充。

| 函数指针 | 功能 |
|---|---|
| `getImGuiTextureHandle` | 从 cache key 获取 ImGui 兼容句柄 |
| `createTextureFromPixels` | 上传原始 RGBA8 像素，返回 cache key |
| `releaseTexture` | 按 cache key 释放 GPU 纹理 |
| `reloadTextureFromPixels` | 热重载纹理 |
| `getTextureDesc` | 查询纹理宽高 |
| `isTextureResident` | 检查纹理是否驻留在 GPU |
| `getCachedTextureCount` | 获取缓存纹理数量 |
| `getTotalGPUMemory` | 获取总估算 GPU 内存 |
| `loadTextureFromAsset` | 从 `.nnasset` 句柄 + GUID 加载 |
| `loadTextureFromBlob` | 从预解析 blob 数据加载 |

实现位于 `Engine/Source/Runtime/NNRuntimeEngineServices/Private/RenderAsset/RenderAssetRuntimeApi.cpp`，每个函数是转发到 `NNRenderAssetManager::Get()` 的薄封装。

---

## 7. 关键设计决策

### 7.1 为什么使用 shared_ptr<void> 而非 void*？

`shared_ptr<void>` 捕获了原始类型的 deleter，确保在 cache entry 被驱逐时，`OpenGL::Texture2D` 的析构函数能被正确调用。普通 `void*` 无法做到这一点。

### 7.2 为什么 NNTextureResource 不拥有 GPU 资源？

将所有权（cache entry）和观察者（Resource）分离，使得多个系统可以安全持有 Resource 指针而不用担心释放顺序。GPU 资源的生命周期由缓存策略统一管理。

### 7.3 为什么有 LoadTextureFromBlob？

解决跨 DLL 单例问题。当调用方已经通过自己的 `NNAssetManager` 实例解析出 blob 数据时，直接传递指针比传递 asset handle 更安全。

### 7.4 为什么 NNTextureCache 存在但未被使用？

它是为替换 `NNRenderAssetManager::EvictLRU()` 的 O(n) 线性扫描而准备的独立组件。提供了 O(1) 的 LRU 实现，但尚未集成到管理器中。

### 7.5 为什么纹理加载是同步的？

当前 MVP 阶段采用同步加载以简化实现。`NNTextureResidency` 枚举中的 `Loading` 状态已预留，为未来异步流式加载做准备。

---

## 8. 已知问题与可优化点

### 8.1 LRU 驱逐效率（高优先级）

**现状：** `EvictLRU()` 使用线性扫描（O(n)），在大量纹理缓存时性能差。

**优化方案：** 将 `NNTextureCache`（已有 O(1) LRU 实现）集成到 `NNRenderAssetManager` 中，替换内嵌的线性扫描逻辑。

### 8.2 OpenGL 格式映射不完整（中优先级）

**现状：** `MapToGLFormat()` 仅处理 R8/RG8/RGB8/RGBA8，未知格式一律 fallback 到 RGBA8。

**需补充：** R16_Float ~ RGBA32_Float 浮点格式的 GL 映射，以及 BC1/BC3/BC7/ETC2/ASTC 压缩格式支持。

### 8.3 同步纹理加载阻塞主线程（中优先级）

**现状：** `LoadTextureFromAsset` / `LoadTextureFromBlob` 是同步的，大纹理加载会导致帧率卡顿。

**优化方案：**
- 实现异步加载管线（预创建纹理占位 → 后台线程解码/上传 → 原子切换）
- 利用 `NNTextureResidency::Loading` 状态跟踪加载中资源
- 基础 mip 级别可先上传低分辨率版本（mip streaming）

### 8.4 跨模块单例问题的根本解决（低优先级）

**现状：** 通过 `LoadTextureFromBlob()` 绕过了 `NNAssetManager` 单例不一致的问题，但增加了调用方的复杂度。

**优化方案：**
- 将 `NNAssetManager` 改为通过 C API 桥接（类似 `NNRenderAssetAPI`）
- 或将 `NNAssetManager` 的实例通过依赖注入传递，而非使用全局单例

### 8.5 纹理热重载覆盖不完整（低优先级）

**现状：** `ReloadTextureFromPixels()` 仅支持从原始像素重载，不支持从更新的 `.nnasset` 文件重载。

**优化方案：** 增加 `ReloadTextureFromAsset(cacheKey, assetHandle, guidLow)` 方法。

### 8.6 缓存驱逐策略单一（低优先级）

**现状：** 仅支持 LRU 驱逐，无优先级、引用计数或用户锁定机制。

**优化方向：**
- 支持标记纹理为"不可驱逐"（如 UI 常驻纹理）
- 支持按场景/区域分组的缓存策略
- 考虑 LFU（最少使用）或自适应策略

### 8.7 GPU 内存估算不精确（低优先级）

**现状：** `GPUMemory` 字段为 `size_t` 估算值，可能未考虑压缩格式、mip 链等因素。

**优化方案：** 基于格式和 mip 级别精确计算，并在格式扩展后同步更新。

### 8.8 缺少单元测试（低优先级）

**现状：** `Test/` 目录为空。

**建议覆盖：**
- `NNTextureSourceAsset` 的序列化/反序列化一致性
- `NNTextureCache` 的 LRU 驱逐正确性
- `NNRenderAssetManager` 的 GUID 去重和缓存查询

---

## 9. 开发进度与待办

### 已完成

- [x] 基础纹理加载管线（.nnasset → GPU）
- [x] GPU 纹理缓存与 GUID 去重
- [x] 类型擦除所有权机制（shared_ptr<void>）
- [x] NNTextureResource 轻量包装（观察者模式）
- [x] LRU 驱逐框架（EvictLRU + NNTextureCache 独立组件）
- [x] C API 桥接层（NNRenderAssetAPI）
- [x] ImGui 纹理句柄支持
- [x] 跨模块单例绕过（LoadTextureFromBlob）
- [x] 二进制序列化格式（NNTextureSourceAsset）
- [x] 基础格式枚举与 GetBytesPerPixel()

### 待实现

- [ ] 集成 NNTextureCache 替换 EvictLRU() 线性扫描
- [ ] 浮点格式和压缩格式的 GL 映射
- [ ] 异步纹理加载管线
- [ ] Mip streaming（低分辨率占位 → 逐步升级）
- [ ] 跨模块单例根本解决方案
- [ ] 从 .nnasset 文件的热重载支持
- [ ] 不可驱逐标记和分组缓存策略
- [ ] 精确 GPU 内存计算
- [ ] 单元测试

---

*最后更新：2026/05/28*
