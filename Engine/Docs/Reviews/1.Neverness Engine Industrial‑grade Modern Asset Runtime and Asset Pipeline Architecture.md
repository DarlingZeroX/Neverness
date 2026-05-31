# Neverness Engine 工业级现代 Asset Runtime 与 Asset Pipeline 架构

## 完整实施计划

> 版本：1.0
> 日期：2026-05-24
> 状态：实施计划

---

## 目录

- [一、总体目标架构](#一总体目标架构)
- [二、现状分析与差距](#二现状分析与差距)
- [三、目标程序集与模块结构](#三目标程序集与模块结构)
- [四、目录结构](#四目录结构)
- [五、模块依赖图](#五模块依赖图)
- [六、Runtime / Editor 职责划分](#六runtime--editor-职责划分)
- [七、.meta 文件格式设计](#七meta-文件格式设计)
- [八、GUID 系统设计](#八guid-系统设计)
- [九、.nnasset 二进制格式设计](#九nnasset-二进制格式设计)
- [十、Importer 架构](#十importer-架构)
- [十一、Editor AssetDatabase 设计](#十一editor-assetdatabase-设计)
- [十二、Dependency Graph 设计](#十二dependency-graph-设计)
- [十三、Runtime Asset Manager（C++）](#十三runtime-asset-managerc)
- [十四、Runtime Asset Handle 设计](#十四runtime-asset-handle-设计)
- [十五、Editor 与 Runtime API 分离](#十五editor-与-runtime-api-分离)
- [十六、C ABI 设计](#十六c-abi-设计)
- [十七、Async Loading 方案](#十七async-loading-方案)
- [十八、Streaming 方案](#十八streaming-方案)
- [十九、Addressable / Package 方案](#十九addressable--package-方案)
- [二十、Package Mounting 方案](#二十package-mounting-方案)
- [二十一、Hot Reload 方案](#二十一hot-reload-方案)
- [二十二、Prefab Integration](#二十二prefab-integration)
- [二十三、Scene Integration](#二十三scene-integration)
- [二十四、Memory / Cache 优化](#二十四memory--cache-优化)
- [二十五、Incremental Import](#二十五incremental-import)
- [二十六、AI Pipeline 预留](#二十六ai-pipeline-预留)
- [二十七、完整 Runtime Asset 生命周期](#二十七完整-runtime-asset-生命周期)
- [二十八、完整 Import Pipeline](#二十八完整-import-pipeline)
- [二十九、实施阶段与任务分解](#二十九实施阶段与任务分解)
- [三十、工业级最终架构总结](#三十工业级最终架构总结)

---

## 一、总体目标架构

### 1.1 目标

构建工业级 Asset Runtime + Editor Pipeline，参考 Unity Addressables + Unreal AssetRegistry + Frostbite Asset Pipeline。

### 1.2 核心原则

```
Runtime 完全不依赖：
  .meta
  importer
  AssetDatabase
  Assets/ 目录扫描
  source asset

Editor 负责：
  source asset 管理
  importer 管理
  dependency 管理
  .meta 管理
  GUID 管理
  reimport
```

### 1.3 磁盘布局

```
Project/
├── Assets/                        # Editor only，source asset
│   ├── Characters/
│   │   ├── hero.png
│   │   └── hero.png.meta
│   └── Materials/
│       ├── default_lit.material
│       └── default_lit.material.meta
│
├── Library/                       # Editor only，import cache
│   ├── Imported/                  # 编译后的 .nnasset
│   │   └── 7b/
│   │       └── 7bfa...xxxx.nnasset
│   ├── Cache/
│   │   └── Dependency/            # 依赖图缓存
│   └── Thumbnails/                # 缩略图缓存
│
├── Build/                         # 打包输出
│   └── Windows/
│       ├── game.nnpack            # 运行时包
│       └── manifest.bin           # 包清单
│
└── Packages/                      # Addressable 包
    └── ui.nnpack
```

### 1.4 Runtime 仅加载

- `.nnasset` — 编译后的单个资产
- `.nnpack` — 打包后的资产包

### 1.5 Editor 使用

- source asset（原始文件）
- `.meta`（GUID + importer settings）
- Library/Imported/（编译缓存）

---

## 二、现状分析与差距

### 2.1 现有资产相关模块

| 模块 | 路径 | 状态 |
|------|------|------|
| `NNRuntimeAssetLegacy` | `Engine/Source/Runtime/NNRuntimeAssetLegacy/` | Legacy，冻结 |
| `NNRuntimeEngine` | `Engine/Source/Runtime/NNRuntimeEngine/` | AssetSubsystem 是 stub |
| `NNRuntimeEngineServices` | `Engine/Source/Runtime/NNRuntimeEngineServices/` | ABI wiring 存在 |
| `NNNativeEngineAPI` | `Engine/Source/Runtime/NNNativeEngineAPI/` | NNGuid/Handle 已定义 |
| `NNRuntimePak` | `Engine/Source/Runtime/NNRuntimePak/` | Pak 格式存在但未集成 |
| `NNRuntimeVFS` | `Engine/Source/Runtime/NNRuntimeVFS/` | VFS 接口存在 |
| `Neverness.Runtime.Assets` | `Managed/Runtime/Neverness.Runtime.Assets/` | GUID/AssetDatabase/ImportPipeline 极简 |
| `Neverness.Editor.Assets` | `Managed/Editor/Neverness.Editor.Assets/` | ContentBrowser/Factory 存在 |

### 2.2 关键差距

| 缺失项 | 说明 |
|--------|------|
| .meta 文件系统 | C# 无读写，Legacy 是 [VGASSET] 文本格式 |
| GUID 系统分裂 | Legacy 64-bit HUUID vs 新 128-bit NNGuid，无桥接 |
| AssetDatabase | 纯 Dictionary，无依赖图、无增量扫描、无序列化 |
| Import Pipeline | 无 IAssetImporter、无类型分发、无 reimport |
| Dependency Graph | 只有前向列表，无图遍历、无反向查询、无环检测 |
| .nnasset 格式 | 不存在 |
| Runtime Asset Manager | AssetSubsystem 是 stub |
| Streaming | 无 |
| Addressable | 无 |
| Hot Reload | 无 |
| Build Pipeline | 无 |
| Thumbnail | 无 |

### 2.3 需要新建的 C++ 模块

| 模块名 | 说明 |
|--------|------|
| `NNRuntimeAsset` | 全新 Runtime Asset 系统（替代 Legacy） |
| `NNAssetCompiler` | .nnasset 编译器 |
| `NNAssetCooker` | Build pipeline（.nnpack 生成） |
| `NNAssetRegistry` | Runtime asset registry（C++ 实现） |

### 2.4 需要重构的 C# 项目

| 项目 | 说明 |
|------|------|
| `Neverness.Runtime.Assets` | 重构为纯 Runtime API（去掉 Editor 逻辑） |
| `Neverness.Editor.Assets` | 大幅扩展：AssetDatabase、ImportPipeline、DependencyGraph |

### 2.5 需要新建的 C# 项目

| 项目 | 说明 |
|------|------|
| `Neverness.Editor.ImportPipeline` | Importer 注册与执行 |
| `Neverness.Editor.BuildPipeline` | Build/Cook/Pack 工具 |

---

## 三、目标程序集与模块结构

### 3.1 C++ 模块

```
NNRuntimeAsset          # Runtime 资产系统（核心）
  ├── NNAssetManager    # 资产管理器
  ├── NNAssetLoader     # 资产加载器
  ├── NNAssetHandle     # 类型化 Handle
  ├── NNStreamingManager# Async streaming
  └── NNAssetFormat     # .nnasset 格式定义

NNAssetCompiler         # .nnasset 编译器（Editor 用）
  ├── 编译 source → .nnasset
  ├── 依赖分析
  └── 增量编译

NNAssetCooker           # Build pipeline
  ├── .nnpack 打包
  ├── manifest 生成
  └── 压缩/加密

NNAssetRegistry         # Runtime 资产注册表
  ├── GUID ↔ Handle 映射
  ├── 依赖关系
  └── Package 索引
```

### 3.2 C# 模块

```
Neverness.Runtime.Assets        # Runtime 面向 API（精简）
  ├── GUID                      # 128-bit blittable
  ├── AssetHandle<T>            # 类型化 handle
  └── AssetRuntimeApi           # P/Invoke → C ABI

Neverness.Editor.Assets         # Editor 资产工具（大幅扩展）
  ├── EditorAssetDatabase       # GUID↔path、依赖图、标签
  ├── ImportPipeline            # Importer 注册与执行
  ├── MetaFileManager           # .meta 读写
  ├── DependencyGraph           # 完整依赖图
  ├── AssetWatcher              # 文件系统监视
  ├── ThumbnailGenerator        # 缩略图
  ├── AssetBrowser              # 资产浏览器
  └── AddressableSystem         # 标签/组/包

Neverness.Editor.BuildPipeline  # Build 工具
  ├── BuildPipeline             # 编排
  ├── AssetCooker               # C# → 调用 NNAssetCompiler
  └── PackageBuilder            # .nnpack 生成
```

---

## 四、目录结构

### 4.1 C++ 新模块目录

```
Engine/Source/Runtime/
├── NNRuntimeAsset/                    # 新建
│   ├── CMakeLists.txt
│   ├── Include/
│   │   ├── NNAssetManager.h
│   │   ├── NNAssetLoader.h
│   │   ├── NNAssetHandle.h
│   │   ├── NNAssetFormat.h           # .nnasset 二进制格式
│   │   ├── NNStreamingManager.h
│   │   ├── NNAssetCache.h
│   │   ├── NNAssetRef.h              # 引用计数
│   │   └── NNAssetTypes.h            # 类型注册
│   ├── Private/
│   │   ├── NNAssetManager.cpp
│   │   ├── NNAssetLoader.cpp
│   │   ├── NNStreamingManager.cpp
│   │   ├── NNAssetCache.cpp
│   │   └── NNAssetFormat.cpp
│   └── Docs/
│       └── MODULE_ARCHITECTURE_AND_PROGRESS.md
│
├── NNAssetCompiler/                   # 新建
│   ├── CMakeLists.txt
│   ├── Include/
│   │   ├── NNAssetCompiler.h
│   │   ├── NNCompileContext.h
│   │   └── NNImportResult.h
│   ├── Private/
│   │   ├── NNAssetCompiler.cpp
│   │   ├── Compilers/
│   │   │   ├── NNTextureCompiler.cpp
│   │   │   ├── NNMeshCompiler.cpp
│   │   │   ├── NNAudioCompiler.cpp
│   │   │   └── NNSceneCompiler.cpp
│   │   └── NNCompileContext.cpp
│   └── Docs/
│
├── NNAssetCooker/                     # 新建
│   ├── CMakeLists.txt
│   ├── Include/
│   │   ├── NNAssetCooker.h
│   │   ├── NNPackBuilder.h
│   │   └── NNCookManifest.h
│   ├── Private/
│   │   ├── NNAssetCooker.cpp
│   │   ├── NNPackBuilder.cpp
│   │   └── NNCookManifest.cpp
│   └── Docs/
│
└── NNAssetRegistry/                   # 新建
    ├── CMakeLists.txt
    ├── Include/
    │   ├── NNAssetRegistry.h
    │   ├── NNGuidTable.h
    │   └── NNDependencyTable.h
    ├── Private/
    │   ├── NNAssetRegistry.cpp
    │   ├── NNGuidTable.cpp
    │   └── NNDependencyTable.cpp
    └── Docs/
```

### 4.2 C# 目录调整

```
Engine/Source/Managed/
├── Runtime/Neverness.Runtime.Assets/        # 重构
│   ├── Public/
│   │   ├── GUID.cs                          # 保留，增加 NewRandom()
│   │   ├── AssetHandle.cs                   # 新增
│   │   ├── AssetRuntimeApi.cs               # 新增，精简 P/Invoke
│   │   └── AssetReference.cs                # 新增，Addressable 引用
│   └── Neverness.Runtime.Assets.csproj
│
├── Editor/Neverness.Editor.Assets/          # 大幅扩展
│   ├── Public/
│   │   ├── EditorAssetDatabase.cs           # 重构自 AssetDatabase.cs
│   │   ├── ImportPipeline.cs               # 重构
│   │   ├── IAssetImporter.cs               # 新增
│   │   ├── AssetImporterAttribute.cs       # 新增
│   │   ├── MetaFileManager.cs              # 新增
│   │   ├── DependencyGraph.cs              # 新增
│   │   ├── AssetWatcher.cs                 # 新增
│   │   ├── AssetLabelSystem.cs             # 新增
│   │   ├── AssetGroupManager.cs            # 新增
│   │   └── ThumbnailManager.cs             # 新增
│   ├── Private/
│   │   ├── Importers/
│   │   │   ├── TextureImporter.cs
│   │   │   ├── MeshImporter.cs
│   │   │   ├── AudioImporter.cs
│   │   │   ├── MaterialImporter.cs
│   │   │   └── SceneImporter.cs
│   │   ├── Database/
│   │   │   ├── AssetDatabaseIndex.cs
│   │   │   ├── AssetDatabaseCache.cs
│   │   │   └── AssetDatabaseScanner.cs
│   │   ├── Dependency/
│   │   │   ├── DependencyGraphImpl.cs
│   │   │   └── CircularDependencyDetector.cs
│   │   ├── Panel/
│   │   │   ├── ContentBrowserPanel.cs       # 保留，增强
│   │   │   └── AssetInspectorPanel.cs
│   │   └── Addressable/
│   │       ├── AddressableGroup.cs
│   │       └── AddressableProfile.cs
│   └── Neverness.Editor.Assets.csproj
│
└── Editor/Neverness.Editor.BuildPipeline/   # 新建
    ├── Public/
    │   ├── BuildPipeline.cs
    │   ├── AssetCooker.cs
    │   ├── PackageBuilder.cs
    │   └── BuildProfile.cs
    ├── Private/
    │   ├── BuildPipelineImpl.cs
    │   └── CookTaskScheduler.cs
    └── Neverness.Editor.BuildPipeline.csproj
```

---

## 五、模块依赖图

### 5.1 C++ 模块依赖

```
                    ┌─────────────────┐
                    │ NNAssetCooker   │  Build-time
                    └───┬─────────┬───┘
                        │         │
              ┌─────────▼──┐  ┌──▼──────────┐
              │NNAsset     │  │NNRuntimePak │
              │Compiler    │  │(已存在)      │
              └─────┬──────┘  └──────┬──────┘
                    │                │
         ┌──────────▼────────────────▼──┐
         │    NNRuntimeAsset            │  Runtime 核心
         │  (AssetManager/Loader/       │
         │   Streaming/Cache/Format)    │
         └──────┬──────────────┬────────┘
                │              │
    ┌───────────▼──┐   ┌──────▼──────────┐
    │NNAsset       │   │NNRuntimeVFS     │
    │Registry      │   │(已存在)          │
    └──────┬───────┘   └─────────────────┘
           │
    ┌──────▼──────────────────┐
    │NNNativeEngineAPI        │  C ABI 层（已存在）
    │(EngineTypes/Handles/API)│
    └─────────────────────────┘
           │
    ┌──────▼──────────────────┐
    │NNRuntimeEngineServices  │  Wiring（已存在）
    └─────────────────────────┘
```

### 5.2 C# 模块依赖

```
NevernessEditor (Exe)
  ├── Neverness.Editor.BuildPipeline
  │     ├── Neverness.Editor.Assets
  │     └── Neverness.Runtime.Assets
  ├── Neverness.Editor.Assets
  │     ├── Neverness.Editor.Core
  │     ├── Neverness.Editor.Serialization
  │     ├── Neverness.Editor.Reflection
  │     ├── Neverness.Editor.UndoRedo
  │     └── Neverness.Runtime.Assets
  ├── Neverness.Editor.Scene
  │     └── Neverness.Runtime.Assets
  └── Neverness.Editor.Inspector
        └── Neverness.Editor.Assets

Neverness.Runtime.Assets
  ├── Neverness.Runtime.Engine      # NNGuid, Handles
  └── Neverness.Runtime.Interop     # P/Invoke
```

### 5.3 关键约束

- `NNRuntimeAsset` 不依赖 `NNAssetCompiler`（Runtime 不知道编译）
- `NNAssetCompiler` 依赖 `NNRuntimeAsset`（使用格式定义）
- `NNAssetCooker` 依赖 `NNAssetCompiler` + `NNRuntimePak`
- `Neverness.Runtime.Assets` 不依赖任何 `Editor.*` 项目
- `Neverness.Editor.BuildPipeline` 可以调用 `NNAssetCompiler` C ABI

---

## 六、Runtime / Editor 职责划分

### 6.1 C# Editor 负责

| 功能 | 模块 |
|------|------|
| EditorAssetDatabase | Neverness.Editor.Assets |
| ImportPipeline | Neverness.Editor.Assets |
| .meta 文件管理 | Neverness.Editor.Assets |
| DependencyGraph | Neverness.Editor.Assets |
| AssetBrowser | Neverness.Editor.Assets |
| Thumbnail | Neverness.Editor.Assets |
| Inspector | Neverness.Editor.Inspector |
| BuildPipeline | Neverness.Editor.BuildPipeline |
| AssetFactory | Neverness.Editor.Assets |
| AssetTypeRegistry | Neverness.Editor.Assets |
| AssetImportSettings | Neverness.Editor.Assets |
| HotReload | Neverness.Editor.Assets |
| Reimport | Neverness.Editor.Assets |
| Addressable labels | Neverness.Editor.Assets |
| Package manifest | Neverness.Editor.BuildPipeline |

### 6.2 C++ Runtime 负责

| 功能 | 模块 |
|------|------|
| NNAssetManager | NNRuntimeAsset |
| NNAssetLoader | NNRuntimeAsset |
| Streaming IO | NNRuntimeAsset |
| Memory mapping | NNRuntimeAsset |
| GPU upload | NNRuntimeEngine（现有） |
| Audio decode | NNRuntimeEngine（现有） |
| Mesh runtime format | NNRuntimeAsset |
| Package mounting | NNRuntimeAsset + NNRuntimePak |
| Bundle loading | NNRuntimeAsset |
| Reference counting | NNRuntimeAsset |
| Async loading | NNRuntimeAsset |
| Runtime dependency resolve | NNAssetRegistry |

### 6.3 C++ 编译/打包（Editor 用但实现在 C++）

| 功能 | 模块 |
|------|------|
| .nnasset 编译 | NNAssetCompiler |
| .nnpack 打包 | NNAssetCooker |
| 增量编译 | NNAssetCompiler |

---

## 七、.meta 文件格式设计

### 7.1 格式

每个 source asset 对应一个同名 `.meta` 文件，使用 YAML 子集（简单 key: value）。

示例 `hero.png.meta`：

```yaml
guid: 7bfa3c01-4e8a-4b2d-9f1e-a1b2c3d4e5f6
importer: TextureImporter
importSettings:
  compression: BC7
  generateMipmaps: true
  srgb: true
  maxSize: 2048
  filterMode: Bilinear
labels:
  - character
  - player
dependencies:
  - shader/default_lit.nnasset
```

示例 `default_lit.material.meta`：

```yaml
guid: a1b2c3d4-e5f6-7890-abcd-ef1234567890
importer: MaterialImporter
importSettings:
  shader: shader/default_lit
  blendMode: Opaque
labels:
  - material
dependencies:
  - shader/default_lit.nnasset
  - texture/hero.nnasset
```

### 7.2 规范

- **guid**：128-bit UUID v4，Editor 首次创建时生成，永久不变
- **importer**：IAssetImporter 实现类名
- **importSettings**：importer 特有的序列化设置
- **labels**：Addressable 标签列表
- **dependencies**：显式依赖 GUID 列表（运行时自动发现的也记录）

### 7.3 Editor Only

- `.meta` 文件仅 Editor 读写
- Runtime 永不读取 `.meta`
- Editor 操作 source asset 时自动维护 `.meta`
- source asset rename/move 时 `.meta` 跟随移动，GUID 不变

### 7.4 迁移策略

- 旧 `[VGASSET];<id>;<type>` 格式的 .meta 文件在首次扫描时自动转换为新 YAML 格式
- 旧 64-bit HUUID 转换为 128-bit GUID（高位填 0 或生成新 UUID 并记录映射）

---

## 八、GUID 系统设计

### 8.1 规范

| 要求 | 实现 |
|------|------|
| stable | .meta 中持久化，rename/move 不变 |
| immutable | .meta 写入后不可修改 |
| editor-generated | Editor 首次导入时生成 UUID v4 |
| runtime-independent | Runtime 不生成 GUID |

### 8.2 C++ Layout

```c
// Engine/Source/Runtime/NNNativeEngineAPI/Include/EngineTypes.h（已存在）
typedef struct NNGuid {
    uint64_t high;
    uint64_t low;
} NNGuid;
```

### 8.3 C# Layout（已存在，增强）

```csharp
// Neverness.Runtime.Assets/Public/GUID.cs
public readonly struct GUID : IEquatable<GUID>
{
    public ulong High { get; }
    public ulong Low { get; }
    public bool IsZero => High == 0 && Low == 0;

    // 已有
    public static GUID FromDeterministicPath(string path);
    public static GUID Parse(string hex);
    public string ToHexString();

    // 新增：Editor 用
    public static GUID NewRandom();           // UUID v4
    public static GUID FromLegacy(ulong id);  // 旧 HUUID 迁移
}
```

### 8.4 C# 必须完全 blittable

- `GUID` 是 `readonly struct`，无引用类型字段
- 与 `NNGuid` 内存布局完全一致，零拷贝 P/Invoke

### 8.5 禁止

- 禁止用 path hash 作为正式 GUID
- 禁止 Runtime 动态生成 GUID
- 禁止两个不同 asset 共享同一 GUID

---

## 九、.nnasset 二进制格式设计

### 9.1 文件布局

```
┌──────────────────────────┐
│  NNAssetHeader           │  固定大小，64 字节对齐
├──────────────────────────┤
│  Dependency GUIDs[]      │  NNGuid × dependencyCount
├──────────────────────────┤
│  Blob Descriptors[]      │  NNBlobDescriptor × blobCount
├──────────────────────────┤
│  Type Info               │  类型特定元数据（可选）
├──────────────────────────┤
│  Padding (64B align)     │
├──────────────────────────┤
│  Binary Payload          │  实际数据（blob 数据连续存储）
└──────────────────────────┘
```

### 9.2 NNAssetHeader

```c
// Engine/Source/Runtime/NNRuntimeAsset/Include/NNAssetFormat.h
#pragma pack(push, 8)

#define NN_ASSET_MAGIC 0x4E4E4153  // 'NNAS'
#define NN_ASSET_VERSION 1

typedef struct NNAssetHeader {
    uint32_t magic;              // 'NNAS'
    uint32_t version;            // 格式版本

    NNGuid   assetGuid;          // 资产 GUID
    uint64_t typeId;             // 类型 ID（FNV-1a of type name）

    uint32_t dependencyCount;    // 依赖数量
    uint32_t blobCount;          // blob 数量

    uint64_t dependencyOffset;   // 依赖表偏移
    uint64_t blobTableOffset;    // blob 表偏移
    uint64_t payloadOffset;      // 载荷偏移
    uint64_t payloadSize;        // 载荷大小

    uint32_t flags;              // 标志位
    uint32_t reserved[3];        // 预留
} NNAssetHeader;

#pragma pack(pop)
```

### 9.3 NNBlobDescriptor

```c
typedef struct NNBlobDescriptor {
    uint64_t offset;       // 相对于 payloadOffset 的偏移
    uint64_t size;         // 未压缩大小
    uint64_t compressedSize; // 压缩后大小（0 = 未压缩）
    uint32_t blobType;     // blob 类型枚举
    uint32_t flags;        // 标志
} NNBlobDescriptor;
```

### 9.4 Flags 定义

```c
#define NN_ASSET_FLAG_COMPRESSED    (1 << 0)  // 整体压缩
#define NN_ASSET_FLAG_STREAMING     (1 << 1)  // 支持 streaming
#define NN_ASSET_FLAG_BUNDLE_MEMBER (1 << 2)  // 属于 bundle
#define NN_ASSET_FLAG_HAS_TYPE_INFO (1 << 3)  // 包含类型信息
```

### 9.5 类型 ID 生成

```c
// FNV-1a 64-bit hash of type name string
// e.g., "Texture2D" → 0x...
// e.g., "Mesh" → 0x...
// e.g., "AudioClip" → 0x...
```

### 9.6 各类型 Payload 布局

#### Texture2D

```
Blobs:
  [0] MipLevel0 (全尺寸)
  [1] MipLevel1
  ...
  [N] Thumbnail（blobType = Thumbnail）

TypeInfo:
  uint32_t width
  uint32_t height
  uint32_t format      // DXGI_FORMAT
  uint32_t mipCount
  uint32_t arraySize
```

#### Mesh

```
Blobs:
  [0] VertexBuffer
  [1] IndexBuffer
  [2] AABB + metadata

TypeInfo:
  uint32_t vertexCount
  uint32_t indexCount
  uint32_t vertexStride
  uint32_t indexFormat   // 16 or 32
  float    boundsMin[3]
  float    boundsMax[3]
```

#### AudioClip

```
Blobs:
  [0] PCM data (或 Opus/Vorbis 压缩)
  [1] Seek table（streaming 用）

TypeInfo:
  uint32_t sampleRate
  uint32_t channels
  uint32_t sampleCount
  uint32_t format       // PCM16/Float32/Opus
```

#### Scene / Prefab

```
Blobs:
  [0] Entity hierarchy data
  [1] Component data (ECS blob)
  [2] Embedded asset data (如有)

TypeInfo:
  uint32_t entityCount
  uint32_t componentCount

Dependencies:
  NNGuid[] — 引用的所有外部资产 GUID
```

---

## 十、Importer 架构

### 10.1 C# Importer 接口

```csharp
// Neverness.Editor.Assets/Public/IAssetImporter.cs

/// <summary>
/// 资产导入器接口。每个 importer 处理一种或多种文件类型。
/// </summary>
public interface IAssetImporter
{
    /// <summary>此 importer 支持的文件扩展名</summary>
    string[] SupportedExtensions { get; }

    /// <summary>显示名称</summary>
    string DisplayName { get; }

    /// <summary>导入 source asset，返回可编译的数据</summary>
    ImportResult Import(AssetImportContext context);
}

/// <summary>
/// 通过 attribute 标记 importer 支持的扩展名
/// </summary>
[AttributeUsage(AttributeTargets.Class)]
public class AssetImporterAttribute : Attribute
{
    public string[] Extensions { get; }
    public AssetImporterAttribute(params string[] extensions) { ... }
}
```

### 10.2 AssetImportContext

```csharp
public sealed class AssetImportContext
{
    public string SourceAssetPath { get; }       // 原始文件路径
    public string MetaFilePath { get; }           // .meta 路径
    public GUID AssetGuid { get; }                // 资产 GUID
    public string OutputPath { get; }             // .nnasset 输出路径
    public MetaSettings ImportSettings { get; }   // 从 .meta 读取的设置
    public IDependencyCollector Dependencies { get; }  // 依赖收集器

    public byte[] ReadAllBytes();                 // 读取源文件
    public Stream OpenReadStream();               // 流式读取
}
```

### 10.3 ImportResult

```csharp
public sealed class ImportResult
{
    public GUID AssetGuid { get; set; }
    public ulong TypeId { get; set; }
    public List<ImportedBlob> Blobs { get; } = new();
    public List<GUID> Dependencies { get; } = new();
    public byte[] ThumbnailData { get; set; }    // 可选缩略图
    public byte[] TypeInfo { get; set; }          // 类型特定元数据
}

public sealed class ImportedBlob
{
    public uint BlobType { get; set; }
    public byte[] Data { get; set; }
    public byte[] CompressedData { get; set; }    // 可选压缩
}
```

### 10.4 内置 Importer

| Importer | 扩展名 | 说明 |
|----------|--------|------|
| `TextureImporter` | .png, .jpg, .tga, .bmp, .dds, .hdr | 图片 → GPU 纹理格式 |
| `MeshImporter` | .fbx, .obj, .gltf, .glb | 模型 → runtime mesh 格式 |
| `AudioImporter` | .wav, .ogg, .mp3, .flac | 音频 → PCM/Opus |
| `MaterialImporter` | .material | 材质 JSON → 二进制 |
| `ShaderImporter` | .hlsl, .glsl, .nnshader | 着色器 → 编译后格式 |
| `SceneImporter` | .scene | 场景 → entity/component blob |
| `PrefabImporter` | .prefab | Prefab → archetype blob |
| `LuaScriptImporter` | .lua | 脚本 → 字节码（可选） |
| `AnimationImporter` | .anim | 动画 → 关键帧数据 |

### 10.5 Importer 注册

```csharp
// 自动发现：扫描所有 Neverness.Editor.* 程序集
// 找到实现 IAssetImporter 的类且标记 [AssetImporter]
public static class ImporterRegistry
{
    public static void Discover();
    public static IAssetImporter GetImporter(string extension);
    public static IReadOnlyList<IAssetImporter> All { get; }
}
```

### 10.6 编译流程

```
ImportPipeline.Import(sourcePath)
  ├── 读取 .meta → GUID, importer type, settings
  ├── ImporterRegistry.GetImporter(ext)
  ├── importer.Import(context) → ImportResult
  ├── 调用 NNAssetCompiler（C++ ABI）将 ImportResult 编译为 .nnasset
  │     或纯 C# 实现编译（简单类型）
  ├── 写入 Library/Imported/XX/xxxxxxxx.nnasset
  ├── 更新 DependencyGraph
  ├── 更新 EditorAssetDatabase
  └── 生成 Thumbnail → Library/Thumbnails/
```

---

## 十一、Editor AssetDatabase 设计

### 11.1 重构目标

当前 `AssetDatabase.cs` 是纯 Dictionary，需重构为工业级 Editor AssetDatabase。

### 11.2 架构

```
EditorAssetDatabase
├── GuidIndex           # GUID ↔ VirtualPath 双向映射
├── PathIndex           # VirtualPath → SourcePath 映射
├── TypeIndex           # GUID → TypeId 映射
├── LabelIndex          # Label → GUID[] 映射
├── DependencyGraph     # 完整依赖图
├── DirtyTracker        # 脏标记
├── MetaCache           # .meta 文件缓存
└── ImportStateCache    # 导入状态（hash → 需要重新导入）
```

### 11.3 API

```csharp
public static class EditorAssetDatabase
{
    // === GUID ↔ Path ===
    public static GUID GuidFromPath(string virtualPath);
    public static string PathFromGuid(GUID guid);
    public static bool Exists(GUID guid);
    public static bool Exists(string virtualPath);

    // === 枚举 ===
    public static IReadOnlyList<GUID> AllAssets { get; }
    public static IReadOnlyList<GUID> FindAssets(string filter);
    public static IReadOnlyList<GUID> FindAssetsByLabel(string label);
    public static IReadOnlyList<GUID> FindAssetsByType(string typeName);

    // === Import ===
    public static void ImportAsset(string sourcePath);
    public static void ImportAll();                         // 全量导入
    public static void ImportChanged();                     // 增量导入
    public static void ReimportAsset(GUID guid);            // 重新导入

    // === 依赖 ===
    public static IReadOnlyList<GUID> GetDependencies(GUID guid);
    public static IReadOnlyList<GUID> GetDependenciesRecursive(GUID guid);
    public static IReadOnlyList<GUID> GetReverseDependencies(GUID guid);
    public static IReadOnlyList<GUID> GetReverseDependenciesRecursive(GUID guid);

    // === 标签 ===
    public static void AddLabel(GUID guid, string label);
    public static void RemoveLabel(GUID guid, string label);
    public static IReadOnlyList<string> GetLabels(GUID guid);

    // === 移动/删除 ===
    public static void MoveAsset(string from, string to);
    public static void DeleteAsset(string path);
    public static void CopyAsset(string source, string dest);

    // === Meta ===
    public static AssetMeta GetMeta(GUID guid);
    public static void SaveMeta(GUID guid);

    // === 缓存管理 ===
    public static void SaveCache();
    public static void LoadCache();
    public static void InvalidateCache();
}
```

### 11.4 增量扫描

```
EditorAssetDatabase.ImportChanged()
  ├── 扫描 Assets/ 目录
  ├── 对比文件 last-write-time 与缓存
  ├── 检测新增文件 → 创建 .meta + GUID → ImportAsset
  ├── 检测删除文件 → 删除 .meta + 移除索引
  ├── 检测修改文件 → ReimportAsset
  └── 检测移动文件 → 更新索引（GUID 不变）
```

### 11.5 序列化

- `Library/Cache/AssetDatabase.cache` — 二进制缓存（GUID 表 + 路径表 + 类型表）
- `Library/Cache/Dependency.cache` — 依赖图缓存
- 启动时加载缓存，增量更新，定期全量重建

### 11.6 FileWatcher

```csharp
public sealed class AssetFileWatcher : IDisposable
{
    public event Action<string> OnAssetChanged;    // 修改
    public event Action<string> OnAssetCreated;    // 新增
    public event Action<string> OnAssetDeleted;    // 删除
    public event Action<string, string> OnAssetRenamed; // 重命名

    public void Watch(string assetsRoot);
    public void Stop();
}
```

- 使用 `FileSystemWatcher`（.NET 内置）
- 防抖处理（200ms 延迟合并连续事件）
- 忽略 `.meta` 文件（单独处理）
- 过滤 `Library/` 目录

---

## 十二、Dependency Graph 设计

### 12.1 数据结构

```csharp
public sealed class DependencyGraph
{
    // 前向依赖：A → [B, C]（A 依赖 B 和 C）
    private readonly Dictionary<GUID, HashSet<GUID>> _forward;

    // 反向依赖：B → [A]（B 被 A 引用）
    private readonly Dictionary<GUID, HashSet<GUID>> _reverse;
}
```

### 12.2 API

```csharp
public sealed class DependencyGraph
{
    // 设置依赖
    public void SetDependencies(GUID asset, IReadOnlyList<GUID> dependencies);

    // 添加单个依赖
    public void AddDependency(GUID asset, GUID dependency);

    // 移除依赖
    public void RemoveDependency(GUID asset, GUID dependency);

    // 查询直接依赖
    public IReadOnlyCollection<GUID> GetDirectDependencies(GUID asset);

    // 查询所有递归依赖
    public IReadOnlyCollection<GUID> GetAllDependencies(GUID asset);

    // 查询直接反向依赖（谁引用了我）
    public IReadOnlyCollection<GUID> GetDirectReverseDependencies(GUID asset);

    // 查询所有递归反向依赖
    public IReadOnlyCollection<GUID> GetAllReverseDependencies(GUID asset);

    // 环检测
    public bool HasCycle();
    public bool HasCycle(GUID asset);
    public List<List<GUID>> FindCycles();

    // 脏传播：当 asset 变化时，返回所有需要重新处理的资产
    public IReadOnlyCollection<GUID> GetDirtyPropagation(GUID changedAsset);

    // 清除某资产的所有依赖
    public void ClearDependencies(GUID asset);

    // 统计
    public int AssetCount { get; }
    public int EdgeCount { get; }
}
```

### 12.3 环检测算法

- 使用 DFS + 三色标记（白/灰/黑）
- 检测到回边时报告环
- 编辑器警告，不阻断（某些情况下允许弱引用打破环）

### 12.4 脏传播

```
Texture 变化：
  → GetReverseDependenciesRecursive(Texture)
  → 所有引用此 Texture 的 Material 标为 Dirty
  → 所有引用这些 Material 的 MeshRenderer 标为 Dirty
  → ...
```

### 12.5 持久化

```
Library/Cache/Dependency.cache
├── Header
├── ForwardEdgeCount
├── ForwardEdges[]: (assetGuid, dependencyGuid)
├── ReverseEdgeCount
└── ReverseEdges[]: (dependencyGuid, assetGuid)
```

---

## 十三、Runtime Asset Manager（C++）

### 13.1 架构

```
NNAssetManager
├── m_HandleTable          # Handle → AssetEntry 映射
├── m_GuidTable            # NNGuid → Handle 映射
├── m_Cache                # LRU 缓存
├── m_LoadingQueue         # 异步加载队列
├── m_DependencyResolver   # 运行时依赖解析
└── m_PackageMounts        # 已挂载的包列表
```

### 13.2 接口

```cpp
// Engine/Source/Runtime/NNRuntimeAsset/Include/NNAssetManager.h

class NNAssetManager {
public:
    // === 同步加载 ===
    template<typename T>
    NNAssetHandle<T> LoadAssetSync(NNGuid guid);

    NNAssetHandle<void> LoadAssetSync(NNGuid guid, uint64_t expectedTypeId);

    // === 异步加载 ===
    template<typename T>
    NNAssetHandle<T> LoadAssetAsync(NNGuid guid,
                                     NNAssetLoadCallback callback = nullptr);

    NNAssetHandle<void> LoadAssetAsync(NNGuid guid,
                                        uint64_t expectedTypeId,
                                        NNAssetLoadCallback callback = nullptr);

    // === 卸载 ===
    void UnloadAsset(NNAssetHandle<void> handle);
    void UnloadAsset(NNGuid guid);

    // === 查询 ===
    bool IsLoaded(NNGuid guid) const;
    bool IsLoading(NNGuid guid) const;
    NNAssetHandle<void> GetLoadedAsset(NNGuid guid) const;

    // === 引用计数 ===
    void AddRef(NNAssetHandle<void> handle);
    void Release(NNAssetHandle<void> handle);
    uint32_t GetRefCount(NNAssetHandle<void> handle) const;

    // === 包管理 ===
    bool MountPackage(const std::string& packPath);
    void UnmountPackage(const std::string& packPath);

    // === 生命周期 ===
    void Initialize();
    void Shutdown();
    void Tick();  // 处理异步队列、流式加载

    // === 调试 ===
    size_t GetLoadedAssetCount() const;
    size_t GetTotalMemoryUsage() const;
};
```

### 13.3 AssetEntry（内部）

```cpp
struct NNAssetEntry {
    NNGuid              guid;
    uint64_t            typeId;
    NNAssetHandle<void> handle;

    std::atomic<uint32_t> refCount{0};

    enum class State : uint8_t {
        Unloaded,
        Loading,
        Loaded,
        Failed,
        Streaming  // 部分加载（streaming）
    };
    std::atomic<State> state{State::Unloaded};

    // 依赖列表
    std::vector<NNGuid> dependencies;
    std::atomic<uint32_t> loadedDependencyCount{0};

    // 数据
    std::vector<uint8_t> data;         // 完整数据
    std::vector<NNBlobView> blobs;     // blob 视图

    // IO
    std::string sourcePath;            // .nnasset 路径（或 pack 内路径）
    bool fromPackage{false};
    uint32_t packageIndex{0};
};
```

### 13.4 加载流程

```
LoadAsset(guid)
  ├── 检查 m_GuidTable → 已加载？返回 handle
  ├── 检查 m_LoadingQueue → 正在加载？挂起 callback
  ├── 创建 AssetEntry，state = Loading
  ├── 异步 IO：读取 .nnasset 文件头
  ├── 解析 Header → 获取 dependency list
  ├── 递归加载依赖（如需要）
  ├── 读取 payload 到内存
  ├── 根据 typeId 创建具体资源（Texture/Mesh/Audio）
  ├── GPU upload（如需要，必须在渲染线程）
  ├── state = Loaded
  ├── 回调通知
  └── 返回 handle
```

---

## 十四、Runtime Asset Handle 设计

### 14.1 C++ Handle

```cpp
// Engine/Source/Runtime/NNRuntimeAsset/Include/NNAssetHandle.h

template<typename T>
class NNAssetHandle {
public:
    NNAssetHandle();
    explicit NNAssetHandle(uint64_t index, uint32_t generation);

    // 访问
    T* Get() const;           // 可能返回 nullptr（未加载）
    T* operator->() const;
    T& operator*() const;
    explicit operator bool() const;

    // 状态
    bool IsLoaded() const;
    bool IsLoading() const;
    bool IsFailed() const;

    // 引用计数
    void AddRef();
    void Release();
    uint32_t GetRefCount() const;

    // GUID
    NNGuid GetGuid() const;

    // 等待加载完成
    void WaitForLoad();        // 阻塞等待
    bool TryGet(T*& out);     // 非阻塞尝试

private:
    uint64_t m_Index;         // HandleTable 索引
    uint32_t m_Generation;    // 防 ABA
};

// 特化
using NNTextureHandle  = NNAssetHandle<NNTexture>;
using NNMeshHandle     = NNAssetHandle<NNMesh>;
using NNAudioHandle    = NNAssetHandle<NNAudioClip>;
using NNSceneHandle    = NNAssetHandle<NNSceneData>;
using NNMaterialHandle = NNAssetHandle<NNMaterial>;
```

### 14.2 Handle Table

```cpp
class NNHandleTable {
    struct Slot {
        void* data{nullptr};
        uint32_t generation{0};
        uint32_t refCount{0};
        uint64_t typeId{0};
    };

    std::vector<Slot> m_Slots;
    std::vector<uint64_t> m_FreeList;
    std::mutex m_Mutex;

public:
    uint64_t Allocate(void* data, uint64_t typeId);
    void Free(uint64_t handle);
    void* Resolve(uint64_t handle, uint32_t expectedGeneration) const;
    void AddRef(uint64_t handle);
    bool Release(uint64_t handle);  // returns true if freed
};
```

### 14.3 C# Handle

```csharp
// Neverness.Runtime.Assets/Public/AssetHandle.cs

/// <summary>
/// 类型化的资产 Handle，blittable，可跨 P/Invoke 传递。
/// </summary>
public readonly struct AssetHandle<T> : IEquatable<AssetHandle<T>>
{
    public ulong Value { get; }
    public bool IsZero => Value == 0;

    public bool IsLoaded { get; }
    public bool IsLoading { get; }

    // 非阻塞查询
    public bool TryGet(out T result);

    // 等待异步加载
    public Task<T> WaitForLoadAsync();

    public void AddRef();
    public void Release();

    public static AssetHandle<T> Zero => default;
}
```

---

## 十五、Editor 与 Runtime API 分离

### 15.1 C++ 头文件分离

```
Engine/Source/Runtime/NNRuntimeAsset/Include/
├── RuntimeAssetAPI.h          # Runtime 可用的 API
│   ├── NNAssetManager         # Load/Unload/Query
│   ├── NNAssetHandle          # Handle 类型
│   └── NNAssetFormat          # .nnasset 格式定义
│
└── EditorAssetAPI.h           # Editor-only（不进入 Runtime 编译）
    ├── NNAssetCompiler        # 编译 API
    ├── NNCompileContext       # 编译上下文
    └── NNImportResult         # 导入结果
```

### 15.2 C# 项目分离

```
Neverness.Runtime.Assets/          # Runtime 消费
  ├── GUID
  ├── AssetHandle<T>
  ├── AssetReference               # Addressable 引用
  └── AssetRuntimeApi              # P/Invoke 到 Runtime C++

Neverness.Editor.Assets/           # Editor 工具
  ├── EditorAssetDatabase          # 不暴露给 Runtime
  ├── ImportPipeline               # 不暴露给 Runtime
  ├── MetaFileManager              # 不暴露给 Runtime
  ├── DependencyGraph              # 不暴露给 Runtime
  └── ...（所有 Editor 逻辑）
```

### 15.3 禁止

- 禁止 `Runtime` 项目引用 `Editor` 项目
- 禁止 Runtime API 中出现 `Import`、`Meta`、`Database` 等概念
- 禁止 C++ Runtime 头文件 include Editor-only 头文件

---

## 十六、C ABI 设计

### 16.1 Asset Manager API

```c
// Engine/Source/Runtime/NNNativeEngineAPI/Include/AssetManagerAPI.h

typedef struct NNAssetManagerAPI {
    // 同步加载
    NNAssetHandle (NN_ENGINE_ABI_STDCALL *loadAssetSync)(NNGuid guid, uint64_t typeId);

    // 异步加载
    NNAsyncWaitHandle (NN_ENGINE_ABI_STDCALL *loadAssetAsync)(
        NNGuid guid,
        uint64_t typeId,
        void* callbackUserData,
        void (NN_ENGINE_ABI_STDCALL *callback)(NNAssetHandle handle, void* userData)
    );

    // 卸载
    void (NN_ENGINE_ABI_STDCALL *unloadAsset)(NNAssetHandle handle);

    // 查询
    int (NN_ENGINE_ABI_STDCALL *isAssetLoaded)(NNAssetHandle handle);
    int (NN_ENGINE_ABI_STDCALL *isAssetLoading)(NNAssetHandle handle);

    // GUID → Handle
    NNAssetHandle (NN_ENGINE_ABI_STDCALL *getAssetByGuid)(NNGuid guid);

    // 引用计数
    void (NN_ENGINE_ABI_STDCALL *addRef)(NNAssetHandle handle);
    void (NN_ENGINE_ABI_STDCALL *releaseRef)(NNAssetHandle handle);

    // 包管理
    int (NN_ENGINE_ABI_STDCALL *mountPackage)(const char* packPath);
    void (NN_ENGINE_ABI_STDCALL *unmountPackage)(const char* packPath);

    // 数据访问
    const void* (NN_ENGINE_ABI_STDCALL *getAssetData)(NNAssetHandle handle);
    uint64_t (NN_ENGINE_ABI_STDCALL *getAssetDataSize)(NNAssetHandle handle);

    // Blob 访问
    uint32_t (NN_ENGINE_ABI_STDCALL *getBlobCount)(NNAssetHandle handle);
    const void* (NN_ENGINE_ABI_STDCALL *getBlobData)(NNAssetHandle handle, uint32_t index);
    uint64_t (NN_ENGINE_ABI_STDCALL *getBlobSize)(NNAssetHandle handle, uint32_t index);

} NNAssetManagerAPI;
```

### 16.2 Asset Registry API（增强现有）

```c
// Engine/Source/Runtime/NNNativeEngineAPI/Include/AssetRegistryAPI.h（增强）

typedef struct NNAssetRegistryAPI {
    // 现有
    int     (NN_ENGINE_ABI_STDCALL *registerAsset)(const char* path, NNGuid guid);
    int     (NN_ENGINE_ABI_STDCALL *unregisterByGuid)(NNGuid guid);
    int     (NN_ENGINE_ABI_STDCALL *unregisterByPath)(const char* path);
    int     (NN_ENGINE_ABI_STDCALL *resolvePathByGuid)(NNGuid guid, char* outPath, nuint pathBufSize);
    int     (NN_ENGINE_ABI_STDCALL *resolveGuidByPath)(const char* path, NNGuid* outGuid);
    uint32_t(NN_ENGINE_ABI_STDCALL *getDependencyCount)(NNGuid guid);
    int     (NN_ENGINE_ABI_STDCALL *getDependencyAt)(NNGuid guid, uint32_t index, NNGuid* outDep);
    NNGuid  (NN_ENGINE_ABI_STDCALL *importAsset)(const char* path);

    // 新增
    int     (NN_ENGINE_ABI_STDCALL *setDependencies)(NNGuid guid, const NNGuid* deps, uint32_t count);
    int     (NN_ENGINE_ABI_STDCALL *addDependency)(NNGuid guid, NNGuid dependency);
    uint32_t(NN_ENGINE_ABI_STDCALL *getReverseDependencyCount)(NNGuid guid);
    int     (NN_ENGINE_ABI_STDCALL *getReverseDependencyAt)(NNGuid guid, uint32_t index, NNGuid* outDep);
    int     (NN_ENGINE_ABI_STDCALL *hasCycle)();

} NNAssetRegistryAPI;
```

### 16.3 Asset Compiler API（Editor 用）

```c
// Engine/Source/Runtime/NNNativeEngineAPI/Include/AssetCompilerAPI.h

typedef struct NNAssetCompilerAPI {
    // 编译单个资产
    int (NN_ENGINE_ABI_STDCALL *compileAsset)(
        const char* sourcePath,
        const char* outputPath,
        const NNGuid* guid,
        uint64_t typeId,
        const void* importSettings,
        uint32_t settingsSize
    );

    // 增量编译
    int (NN_ENGINE_ABI_STDCALL *compileIncremental)(
        const char** changedPaths,
        uint32_t pathCount
    );

    // 批量编译
    int (NN_ENGINE_ABI_STDCALL *compileAll)(
        const char* assetsRoot,
        const char* outputRoot
    );

} NNAssetCompilerAPI;
```

---

## 十七、Async Loading 方案

### 17.1 架构

```
NNStreamingManager
├── IO Thread Pool          # 磁盘读取线程池（2-4 个线程）
├── Decode Thread Pool      # 解压/解码线程池（CPU 密集）
├── GPU Upload Queue        # GPU 上传队列（主线程/渲染线程处理）
├── Request Queue           # 优先级队列
└── Completed Queue         # 完成通知队列
```

### 17.2 流程

```
LoadAssetAsync(guid)
  │
  ▼
Request Queue ──────────────────────────────┐
  │  (priority, guid, callback)             │
  ▼                                         │
IO Thread Pool                              │
  ├── 查找 .nnasset 路径                    │
  ├── 异步读取文件（ReadFile 启用重叠IO）    │
  ├── 读取 Header                           │
  ├── 递归排队依赖                          │
  └── 数据 → Decode Queue                   │
                                            │
Decode Queue ───────────────────────────────┘
  │
  ▼
Decode Thread Pool
  ├── Zstd/LZ4 解压
  ├── 纹理格式转换
  ├── 网格优化
  └── → GPU Upload Queue（如需要）
      → Completed Queue（纯数据）
                                            │
GPU Upload Queue ───────────────────────────┘
  │
  ▼
渲染线程
  ├── CreateTexture2D
  ├── CreateBuffer
  └── → Completed Queue
                                            │
Completed Queue ────────────────────────────┘
  │
  ▼
主线程 Tick()
  ├── 处理 Completed Queue
  ├── 更新 AssetEntry state → Loaded
  ├── 调用 callbacks
  └── 更新引用计数
```

### 17.3 优先级

```cpp
enum class NNLoadPriority : uint8_t {
    Critical = 0,   // 立即需要（如 UI 纹理）
    High = 1,       // 相机附近的资源
    Normal = 2,     // 默认
    Low = 3,        // 后台预加载
    Background = 4  // 最低优先级
};
```

### 17.4 Windows 实现

- IO：`CreateFile` + `ReadFile` with `OVERLAPPED`（重叠 IO / IOCP）
- 或使用 `std::async` + `std::ifstream`（初期简单实现）
- 后期可升级为 IOCP

### 17.5 回调机制

```cpp
using NNAssetLoadCallback = void(*)(NNAssetHandle handle, void* userData);

// 或更灵活的：
using NNAssetLoadCallback = std::function<void(NNAssetHandle handle, NNLoadResult result)>;
```

---

## 十八、Streaming 方案

### 18.1 适用场景

| 类型 | Streaming 方式 |
|------|---------------|
| Texture | Mipmap streaming：先加载低 mip，后台加载高 mip |
| Mesh | LOD streaming：先加载低 LOD，距离近时加载高 LOD |
| Audio | 块 streaming：大文件不一次全读，按需读取 |
| Scene | 区域 streaming：根据相机位置加载/卸载区域 |

### 18.2 Mipmap Streaming

```
LoadTextureStreaming(guid, requiredMipLevel)
  ├── 读取 .nnasset Header
  ├── 加载 mip 0..requiredMipLevel（低分辨率）
  ├── 立即可用
  ├── 后台任务：加载 remaining mips
  └── 每完成一个 mip → GPU upload → 更新 SRV
```

### 18.3 接口

```cpp
class NNStreamingManager {
public:
    // 请求流式加载
    void RequestStream(NNGuid guid, NNLoadPriority priority, float distance);

    // 取消流式请求
    void CancelStream(NNGuid guid);

    // 设置流式级别（mip level / LOD level）
    void SetStreamLevel(NNGuid guid, uint32_t level);

    // 更新（每帧调用）
    void Tick();

    // 设置预算
    void SetMemoryBudget(size_t bytes);      // 总内存预算
    void SetGPUBudget(size_t bytes);          // GPU 内存预算
    void SetIOBandwidth(float megabytesPerSec); // IO 带宽限制

    // 统计
    size_t GetPendingRequestCount() const;
    size_t GetCurrentMemoryUsage() const;
};
```

### 18.4 预算管理

```
总内存预算：512 MB
当前使用：480 MB
需要加载新资源 → 驱逐策略：

驱逐算法：
  1. 计算每个资源的 priority score
     score = basePriority - distanceWeight * distance - ageWeight * lastUsedAge
  2. 驱逐 score 最低的资源
  3. 直到释放足够空间
```

---

## 十九、Addressable / Package 方案

### 19.1 概念

```
AssetLabel         → 字符串标签："character", "ui", "level_01"
AssetGroup         → 资产分组：一组资产的集合
AssetBundle        → 编译后的资产组（.nnpack 的子集）
AssetPackage       → 完整的运行时包（.nnpack）
AddressableProfile → 编辑/开发/发布 配置
```

### 19.2 C# Editor

```csharp
// Addressable 标签管理
public static class AssetLabelSystem
{
    public static void AddLabel(GUID asset, string label);
    public static void RemoveLabel(GUID asset, string label);
    public static IReadOnlyList<string> GetLabels(GUID asset);
    public static IReadOnlyList<GUID> FindByLabel(string label);
}

// 分组管理
public sealed class AssetGroupManager
{
    public AssetGroup CreateGroup(string name);
    public void DeleteGroup(string name);
    public void AddToGroup(string groupName, GUID asset);
    public void RemoveFromGroup(string groupName, GUID asset);
    public IReadOnlyList<AssetGroup> Groups { get; }
}

public sealed class AssetGroup
{
    public string Name { get; set; }
    public string Address { get; set; }        // Addressable 地址
    public List<GUID> Assets { get; }
    public BuildStrategy Strategy { get; set; } // PackTogether / PackSeparately
}
```

### 19.3 C# Runtime

```csharp
// Runtime 面向的 Addressable API
public static class Addressables
{
    // 通过地址加载
    public static AssetHandle<T> LoadAssetAsync<T>(string address);

    // 通过标签加载
    public static IReadOnlyList<AssetHandle<T>> LoadAssetsByLabelAsync<T>(string label);

    // 通过 GUID 加载
    public static AssetHandle<T> LoadAssetByGuidAsync<T>(GUID guid);

    // 释放
    public static void Release(AssetHandle handle);

    // 包管理
    public static Task<bool> LoadCatalogAsync(string catalogPath);
}
```

### 19.4 运行时解析

```
Address "character/hero"
  ├── 查 Catalog → GUID
  ├── 查 GUID → 所在 Package
  ├── Package 已挂载？直接返回
  ├── Package 未挂载？异步下载/加载 .nnpack
  └── 从 Package 中读取 .nnasset → 返回 Handle
```

---

## 二十、Package Mounting 方案

### 20.1 .nnpack 格式

```
┌──────────────────────────┐
│  NNPackHeader            │
│  magic: 'NNPK'           │
│  version                 │
│  assetCount              │
│  manifestOffset          │
│  manifestSize            │
├──────────────────────────┤
│  Asset Table[]           │
│  (guid, offset, size,    │
│   compressedSize, flags) │
├──────────────────────────┤
│  Manifest                │
│  - package name          │
│  - labels index          │
│  - dependency index      │
│  - address index         │
├──────────────────────────┤
│  Compressed Asset Data   │
│  (连续存储的 .nnasset)    │
└──────────────────────────┘
```

### 20.2 C++ Mounting

```cpp
class NNPackageManager {
public:
    // 挂载包（同步/异步）
    bool MountPackage(const std::string& path);
    Task<bool> MountPackageAsync(const std::string& path);

    // 卸载包
    void UnmountPackage(const std::string& path);

    // 查询资产是否在已挂载包中
    bool IsAssetInPackage(NNGuid guid) const;
    std::string GetPackageForAsset(NNGuid guid) const;

    // 从包中读取资产
    bool ReadAssetFromPackage(NNGuid guid, std::vector<uint8_t>& outData);

    // 列出已挂载的包
    std::vector<std::string> GetMountedPackages() const;
};
```

### 20.3 集成到 NNAssetManager

```
LoadAsset(guid)
  ├── 1. 检查内存缓存
  ├── 2. 检查本地 .nnasset 文件
  ├── 3. 检查已挂载的 Packages
  │     └── NNPackageManager.ReadAssetFromPackage(guid)
  ├── 4. 未找到 → 报错 / 异步下载（如支持远程）
  └── ...
```

### 20.4 与现有 NNRuntimePak 的关系

- 现有 `NNRuntimePak` 使用 `.pak` 格式（magic: 'VGPC'）
- 新 `.nnpack` 格式（magic: 'NNPK'）是专门为 asset 系统设计的
- 保留 `NNRuntimePak` 用于通用 VFS（shader cache 等）
- `NNAssetCooker` 生成 `.nnpack`，使用新的格式

---

## 二十一、Hot Reload 方案

### 21.1 Editor 流程

```
FileWatcher 检测到文件变化
  │
  ▼
OnAssetChanged(path)
  │
  ▼
判断变化类型
  ├── Source asset 修改 → ReimportAsset(path)
  ├── .meta 修改 → ReimportAsset(path)
  ├── 新增文件 → Create .meta + ImportAsset
  ├── 删除文件 → Delete .meta + Remove from Database
  └── 重命名 → Update path index (GUID 不变)
  │
  ▼
ReimportAsset
  ├── 重新执行 importer
  ├── 重新编译 .nnasset
  ├── 更新 DependencyGraph
  │     └── 脏传播 → 所有反向依赖重新导入
  └── 通知 Runtime
  │
  ▼
通知 Runtime（通过 C ABI）
  ├── 标记已加载的资产为 "needs reload"
  ├── 下一帧 Tick() 重新加载
  └── 触发回调 → Editor UI 刷新
```

### 21.2 Runtime 侧

```cpp
// NNAssetManager 新增
class NNAssetManager {
    // ...
    void MarkForReload(NNGuid guid);
    void ReloadMarkedAssets();
    void SetReloadCallback(NNAssetReloadCallback callback);
};

// 回调
using NNAssetReloadCallback = void(*)(NNGuid guid, NNAssetHandle newHandle, void* userData);
```

### 21.3 防抖

- FileWatcher 收到事件后延迟 200ms
- 在延迟窗口内合并连续事件
- 避免频繁 reimport（如保存文件时产生多次事件）

---

## 二十二、Prefab Integration

### 22.1 Prefab 编译

```
.prefab (source)
  ├── Entity hierarchy (JSON/YAML)
  ├── Component data
  └── Embedded asset references (GUID)
  │
  ▼  PrefabImporter.Import()
  │
  ▼  NNAssetCompiler.Compile()
  │
.prefab.nnasset
  ├── Header (typeId = Prefab)
  ├── Dependencies: NNGuid[] (所有引用的外部资产)
  ├── Blob[0]: Entity archetype data
  │     ├── Entity count
  │     ├── Component types per entity
  │     └── Component data blobs
  └── Blob[1]: Embedded data (如有)
```

### 22.2 Runtime Instantiate

```cpp
// NNAssetManager
NNEntityHandle InstantiatePrefab(NNGuid prefabGuid);
NNEntityHandle InstantiatePrefab(NNGuid prefabGuid, NNTransform parent);

// 批量实例化（适合群体/NPC）
std::vector<NNEntityHandle> InstantiatePrefabBatch(NNGuid prefabGuid, uint32_t count);
```

### 22.3 Runtime 不依赖 Editor

- Prefab 实例化完全在 C++ 运行时完成
- 不需要 JSON 解析
- 不需要反射系统
- 数据已经是 ECS blob 格式

---

## 二十三、Scene Integration

### 23.1 Scene 不直接引用路径

```cpp
// 现有组件
struct MeshRendererComponent {
    NNGuid mesh;        // 不是路径
    NNGuid material;    // 不是路径
};

// 新增
struct AudioSourceComponent {
    NNGuid audioClip;
};

struct SpriteRendererComponent {
    NNGuid sprite;
};
```

### 23.2 Scene 加载

```
LoadScene(sceneGuid)
  ├── LoadAsset(sceneGuid) → NNSceneData
  ├── 解析 Entity hierarchy
  ├── 对每个组件的 GUID 字段：
  │     LoadAssetAsync(component.mesh)
  │     LoadAssetAsync(component.material)
  │     ...
  ├── 等待关键资产加载完成
  ├── Spawn entities
  └── 后台继续加载非关键资产
```

### 23.3 Scene Streaming

```
大世界场景分区域：
  Region_01 → NNGuid
  Region_02 → NNGuid
  ...

相机移动：
  ├── 检测相机所在区域
  ├── LoadAssetAsync(Region_X)
  ├── UnloadAsset(远离的 Region)
  └── 平滑过渡
```

---

## 二十四、Memory / Cache 优化

### 24.1 内存池

```cpp
// 避免频繁 malloc
class NNAssetMemoryPool {
    // 大块预分配
    // 小对象从池中分配
    // 线程安全
};
```

### 24.2 Handle Table 优化

- 使用 generation 防 ABA
- Free list 复用 slot
- 缓存友好的连续存储

### 24.3 GUID → Handle 查找

```cpp
// 使用开放寻址哈希表替代 std::unordered_map
// 更好的缓存局部性
class NNGuidHashMap {
    // Robin Hood hashing 或类似算法
    // 预分配大表，低负载因子
};
```

### 24.4 .nnasset 格式优化

- Header 固定 64 字节，cache line 对齐
- Blob 数据连续存储，一次 IO 读取
- 64 字节对齐的 payload 偏移

### 24.5 LRU 缓存

```cpp
// 基于访问时间的驱逐
class NNAssetCache {
    size_t m_MaxMemory;
    size_t m_CurrentMemory;

    // intrusive doubly-linked list for LRU
    // O(1) 驱逐
};
```

### 24.6 避免 per-asset PInvoke

- 不要每次加载资产都跨越 C#/C++ 边界
- C# 通过 AssetHandle<T> 持有引用
- 只在必要时调用 C ABI
- 批量操作用 batch API

---

## 二十五、Incremental Import

### 25.1 原理

```
首次全量导入：
  扫描 Assets/ → 所有 source asset → Import → .nnasset

后续增量导入：
  扫描 Assets/ → 对比 last-write-time 或 content hash
  ├── 未变化 → 跳过
  ├── 变化 → Reimport
  ├── 新增 → Import
  └── 删除 → 清理 .nnasset + .meta
```

### 25.2 Content Hash

```csharp
public sealed class ImportStateCache
{
    // 文件路径 → (last-write-time, content-hash)
    private Dictionary<string, (DateTime, byte[])> _cache;

    public bool HasChanged(string path);
    public void MarkImported(string path, DateTime time, byte[] hash);
    public void Save(string cachePath);
    public void Load(string cachePath);
}
```

### 25.3 依赖感知增量

```
Texture A 变化
  → Reimport A
  → DependencyGraph.GetReverseDependencies(A)
  → Material B 依赖 A → Reimport B
  → Material C 依赖 A → Reimport C
  → MeshRenderer D 依赖 B → 标记但不重新编译（数据不变）
```

### 25.4 多线程编译

```
ImportChanged()
  ├── 构建需要编译的资产列表
  ├── 拓扑排序（依赖关系）
  ├── 并行编译无依赖关系的资产
  │     Task.WhenAll(parallelGroup)
  ├── 串行编译有依赖的资产
  └── 更新缓存
```

---

## 二十六、AI Pipeline 预留

### 26.1 设计要求

- Asset System 必须 data-driven
- metadata-driven
- reflection-friendly

### 26.2 AI 生成资产流程

```
用户："生成一个日系角色"
  │
  ▼
AI Service
  ├── 生成 texture (hero.png)
  ├── 生成 mesh (hero.fbx)
  ├── 生成 material (hero.material)
  │
  ▼
Asset System
  ├── 创建 .meta 文件（自动生成 GUID）
  ├── 触发 ImportPipeline
  ├── 生成 .nnasset
  ├── 更新 DependencyGraph
  └── 通知 Editor UI
```

### 26.3 API 预留

```csharp
// Editor API
public static class AIGeneratedAssetHandler
{
    public static async Task<GUID> RegisterAIGeneratedAsset(
        string filePath,
        string assetType,
        Dictionary<string, object> metadata);

    public static async Task BatchRegisterAIGeneratedAssets(
        IReadOnlyList<AIGeneratedAssetInfo> assets);
}
```

### 26.4 要求

- Importer 必须完全数据驱动，不硬编码路径
- .meta 格式必须可被程序生成
- ImportPipeline 必须可被非 UI 线程调用
- 所有 EditorAssetDatabase API 必须线程安全

---

## 二十七、完整 Runtime Asset 生命周期

```
┌──────────────────────────────────────────────────────────────────┐
│                    Editor (C#)                                   │
│                                                                  │
│  Source Asset (.png, .fbx, .material)                           │
│       │                                                          │
│       ▼                                                          │
│  AssetWatcher → 检测变化                                         │
│       │                                                          │
│       ▼                                                          │
│  ImportPipeline.Import(sourcePath)                               │
│       ├── 读取 .meta → GUID, importer, settings                 │
│       ├── ImporterRegistry.GetImporter(ext)                      │
│       ├── importer.Import(context) → ImportResult                │
│       ├── NNAssetCompiler.Compile() → .nnasset                   │
│       ├── 写入 Library/Imported/                                 │
│       ├── 更新 EditorAssetDatabase                               │
│       ├── 更新 DependencyGraph                                   │
│       └── 生成 Thumbnail                                         │
│                                                                  │
└──────────────────────────────────────────────────────────────────┘
                          │
                    .nnasset 文件
                          │
┌──────────────────────────────────────────────────────────────────┐
│                    Runtime (C++)                                 │
│                                                                  │
│  NNAssetManager.LoadAssetAsync(guid)                             │
│       │                                                          │
│       ├── 查缓存 → 命中？返回                                     │
│       ├── 未命中 → IO 线程读取 .nnasset                           │
│       ├── 解析 Header → 获取 dependencies                        │
│       ├── 递归加载依赖                                           │
│       ├── 解压 payload                                           │
│       ├── 创建具体资源对象                                        │
│       ├── GPU upload（渲染线程）                                  │
│       ├── 更新缓存                                               │
│       ├── 回调通知                                               │
│       └── 返回 NNAssetHandle<T>                                  │
│                                                                  │
│  NNAssetManager.UnloadAsset(guid)                                │
│       ├── 引用计数 -1                                            │
│       ├── 计数为 0 → 释放数据                                    │
│       ├── 释放 GPU 资源                                          │
│       └── 从缓存移除                                             │
│                                                                  │
│  NNAssetManager.Tick()                                           │
│       ├── 处理 IO 完成队列                                       │
│       ├── 处理 GPU 上传队列                                      │
│       ├── 处理 streaming 请求                                    │
│       └── 处理 hot reload 通知                                   │
│                                                                  │
└──────────────────────────────────────────────────────────────────┘
```

---

## 二十八、完整 Import Pipeline

```
ImportPipeline.Import(sourceAssetPath)
│
├── Phase 1: Meta 处理
│   ├── 检查 .meta 文件是否存在
│   ├── 不存在 → 创建 .meta
│   │     ├── 生成 GUID (UUID v4)
│   │     ├── 推断 importer 类型（基于扩展名）
│   │     ├── 写入默认 settings
│   │     └── 保存 .meta
│   └── 存在 → 读取 .meta
│         ├── 解析 GUID
│         ├── 解析 importer 类型
│         └── 解析 importSettings
│
├── Phase 2: 变化检测
│   ├── 计算 source asset content hash
│   ├── 对比缓存 hash
│   ├── 未变化且 .nnasset 存在 → 跳过（增量导入）
│   └── 变化 → 继续
│
├── Phase 3: Import
│   ├── ImporterRegistry.GetImporter(extension)
│   ├── 创建 AssetImportContext
│   │     ├── sourcePath
│   │     ├── metaPath
│   │     ├── guid
│   │     ├── settings
│   │     └── dependencyCollector
│   ├── importer.Import(context) → ImportResult
│   │     ├── 读取/解析 source 文件
│   │     ├── 转换数据格式
│   │     ├── 收集依赖
│   │     ├── 生成 blobs
│   │     └── 生成 thumbnail
│   └── 验证 ImportResult 有效性
│
├── Phase 4: Compile
│   ├── 将 ImportResult 序列化为 .nnasset 格式
│   ├── 写入 Header
│   ├── 写入 Dependency GUIDs
│   ├── 写入 Blob Descriptors
│   ├── 写入 Payload（可压缩）
│   └── 写入 Library/Imported/XX/guid.nnasset
│
├── Phase 5: Database 更新
│   ├── EditorAssetDatabase.Register(guid, virtualPath)
│   ├── EditorAssetDatabase.SetType(guid, typeId)
│   ├── 更新 content hash 缓存
│   └── 更新 last-imported 时间
│
├── Phase 6: Dependency 更新
│   ├── DependencyGraph.SetDependencies(guid, deps)
│   ├── 检测环
│   └── 脏传播 → 标记反向依赖
│
├── Phase 7: Thumbnail
│   ├── ThumbnailManager.Generate(context, result)
│   └── 写入 Library/Thumbnails/guid.png
│
└── Phase 8: 通知
    ├── 触发 AssetChanged 事件
    ├── Editor UI 刷新
    └── 通知 Runtime（如需要 hot reload）
```

---

## 二十九、实施阶段与任务分解

### Phase 1: 基础设施（1-2 周）

#### C++ 新建模块：NNRuntimeAsset

- [x] 创建 `Engine/Source/Runtime/NNRuntimeAsset/` 目录结构
- [x] 编写 `CMakeLists.txt`
- [x] 实现 `NNAssetFormat.h` — .nnasset 二进制格式定义
- [x] 实现 `NNAssetFormat.cpp` — 格式验证/读写
- [x] 实现 `NNHandleTable` — Handle 管理
- [x] 实现 `NNAssetHandle.h` — 类型化 Handle 模板
- [x] 实现 `NNAssetTypes.h` — 类型注册系统
- [x] 编写 MODULE_ARCHITECTURE_AND_PROGRESS.md
- [x] 实现 `NNAssetManager.h/.cpp` — 资产管理器核心
- [x] 实现 `NNAssetCache.h/.cpp` — LRU 快取
- [x] 实现 `NNStreamingManager.h/.cpp` — 串流管理器框架
- [x] 实现 `NNAssetRef.h` — RAII 引用计数
- [x] 实现 `NNAssetManagerApi.cpp` — C ABI 桥接

#### C++ 新建模块：NNAssetRegistry

- [x] 创建 `Engine/Source/Runtime/NNAssetRegistry/` 目录结构
- [x] 实现 `NNAssetRegistry.h` — 注册表接口
- [x] 实现 `NNGuidTable.h/.cpp` — GUID ↔ Handle 映射
- [x] 实现 `NNDependencyTable.h/.cpp` — 依赖关系存储
- [x] 实现环检测算法（DFS + 三色标记）
- [x] 实现反向依赖查询
- [x] 实现 `NNAssetRegistryApi.cpp` — C ABI 桥接

#### C# 增强

- [x] `GUID.cs` — 添加 `NewRandom()` (UUID v4)
- [x] `GUID.cs` — 添加 `FromLegacy(ulong)` 迁移方法
- [x] `GUID.cs` — 添加 `ToUuidString()` 标准 UUID 格式化
- [x] `AssetHandle.cs` — 新建类型化 Handle 结构

#### C ABI

- [x] 新建 `AssetManagerAPI.h` — NNAssetManagerAPI 函数表（23 个函数）
- [x] 增强 `AssetRegistryAPI.h` — 添加 setDependencies, addDependency, removeDependency, reverse deps, hasCycle 等 8 个新函数
- [x] 更新 `EngineAPIRegistry.h` — 新增 NNAssetManagerAPI 子表，layoutVersion 15→16

#### CMake 集成 + 基础设施

- [x] 更新 `Runtime/CMakeLists.txt` — 添加 NNRuntimeAsset + NNAssetRegistry
- [x] 更新 `NNRuntimeEngineServices/CMakeLists.txt` — 链接新模块
- [x] 更新 `NNRuntimeEngineServices` RuntimeApiBuilders — 添加 NNBuildAssetManagerRuntimeApi
- [x] 更新 `NNRuntimeEngineServices` AssetRegistryRuntimeApi — 接线新 8 个函数
- [x] 更新 `NNRuntimeNativeEngineAPIStub` — 添加 AssetManagerApiStubs + 接线
- [x] 更新 `NativeEngineAPI.h` / `EngineAPIRegistry.h` — include 新头文件
- [x] 编写 `NNRuntimeAsset/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md`
- [x] 编写 `NNAssetRegistry/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md`

### Phase 2: AssetDatabase + Meta（2-3 周）

#### C# EditorAssetDatabase

- [x] 重构 `AssetDatabase.cs` → `EditorAssetDatabase.cs`
- [x] 实现 `GuidIndex` — GUID ↔ VirtualPath 双向映射
- [x] 实现 `PathIndex` — VirtualPath → SourcePath
- [x] 实现 `TypeIndex` — GUID → TypeId
- [x] 实现 `LabelIndex` — Label → GUID[]
- [x] 实现 `DirtyTracker` — 脏标记
- [x] 实现序列化：`Library/Cache/AssetDatabase.cache`
- [x] 实现 `ImportStateCache` — content hash 缓存

#### MetaFileManager

- [x] 实现 `MetaFileManager.cs` — .meta 文件读写
- [x] 定义 .meta YAML 格式
- [x] 实现 .meta 创建（GUID 生成 + 默认 settings）
- [x] 实现 .meta 解析
- [x] 实现旧格式迁移：[VGASSET] → YAML

#### AssetWatcher

- [x] 实现 `AssetWatcher` — FileSystemWatcher 封装
- [x] 实现防抖逻辑（200ms）
- [x] 实现事件过滤（忽略 Library/、.meta）
- [ ] 集成到 EditorAssetDatabase

#### Incremental Scan

- [x] 实现 `ForceFullScan()` — 全量扫描
- [ ] 实现 `ImportChanged()` — 增量扫描
- [x] 实现文件新增/删除/修改/重命名检测

### Phase 3: Import Pipeline（2-3 周）

#### Importer 框架

- [x] 实现 `IAssetImporter.cs` — 接口定义（含 ISettingsAwareImporter 扩展接口）
- [x] 实现 `AssetImporterAttribute` — 扩展名标记（含 Priority 优先级）
- [x] 实现 `AssetImportContext.cs` — 导入上下文（含 DependencyCollector、设置读取辅助）
- [x] 实现 `ImportResult.cs` — 导入结果（含 ImportedBlob、AssetTypeId 常量）
- [x] 实现 `ImporterRegistry.cs` — 自动发现（反射扫描 + 优先级排序）

#### 内置 Importer

- [x] 实现 `TextureImporter` — .png/.jpg/.tga/.bmp/.dds/.hdr（解码为骨架实现，需集成 StbImageSharp）
- [x] 实现 `MeshImporter` — .fbx/.obj/.gltf/.glb（解析为骨架实现，需集成 AssimpNet）
- [x] 实现 `AudioImporter` — .wav/.ogg/.mp3/.flac（WAV 头解析已实现，OGG/MP3/FLAC 需 NAudio 库）
- [x] 实现 `MaterialImporter` — .material（完整 JSON→二进制序列化，含依赖收集）
- [x] 实现 `SceneImporter` — .scene（完整 JSON→二进制序列化，含实体/组件/依赖）
- [x] 实现 `PrefabImporter` — .prefab（完整 JSON→二进制序列化，递归实体树 + 依赖收集）
- [x] 实现 `LuaScriptImporter` — .lua（完整实现，含注释去除）

#### ImportPipeline

- [x] 实现 `ImportPipeline.cs` — 完整 8 阶段导入流程
- [x] 实现 .nnasset 编译（纯 C# 版本，与 C++ NNAssetFormat.h 格式一致）
- [ ] 实现 C++ NNAssetCompiler 调用（复杂类型）— **未实现原因：C# 版本已覆盖所有类型的 .nnasset 编译，C++ NNAssetCompiler 用于 GPU 端压缩等复杂操作，属于性能优化阶段需求**

#### Thumbnail

- [x] 实现 `ThumbnailManager.cs` — 含 BMP 编码器（零外部依赖）
- [x] 实现 Texture 缩略图生成 — 最近邻降采样到 128×128
- [ ] 实现 Mesh 缩略图生成（渲染截图）— **未实现原因：需要离屏渲染器（OffscreenRenderer）支持，属于编辑器渲染管线集成范围**
- [x] 实现默认图标 fallback — 按资产类型的彩色图标 + 类型标签文字

### Phase 4: Dependency Graph（1-2 周）

#### C# DependencyGraph

- [x] 实现 `DependencyGraph.cs` — 完整依赖图（前向/反向双向维护，Thread-safe）
- [x] 实现前向依赖查询 — `GetDirectDependencies(GUID)`
- [x] 实现递归依赖查询 — `GetAllDependencies(GUID)` 深度优先遍历
- [x] 实现反向依赖查询 — `GetDirectReverseDependencies(GUID)`
- [x] 实现递归反向依赖查询 — `GetAllReverseDependencies(GUID)`
- [x] 实现 DFS 环检测 — `HasCycle()` / `HasCycle(GUID)` / `FindCycles()` 三色标记
- [x] 实现脏传播算法 — `GetDirtyPropagation(GUID)` 返回所有递归反向依赖
- [x] 实现序列化：`Library/Cache/Dependency.cache`（magic 'NNDG'，前向+反向边冗余存储）

#### C++ NNDependencyTable

- [x] 实现 C++ 版依赖表 — `NNDependencyTable.h/.cpp`（Phase 1 已完成）
- [x] 实现 C ABI：setDependencies, addDependency — `NNAssetRegistryApi.cpp`（Phase 1 已完成）
- [x] 实现反向依赖查询 — `GetReverseDependencyCount/At`（Phase 1 已完成）

#### 集成

- [x] ImportPipeline 导入完成后自动更新 DependencyGraph — Phase 6 中调用 `SetDependencies` + 环检测警告 + 脏传播 MarkDirty
- [x] EditorAssetDatabase.GetDependencies() 使用 DependencyGraph — 新增 `GetDependencies` / `GetAllDependencies` / `GetReverseDependencies` / `GetAllReverseDependencies` / `HasDependencyCycle`
- [x] Reimport 时触发脏传播 — `GetDirtyPropagation` 自动标记所有反向依赖为脏

### Phase 5: Runtime Asset Manager（3-4 周）

#### NNAssetManager

- [x] 实现 `NNAssetManager.h/.cpp` — 单例，完整生命周期管理（Phase 1 创建，本次补全 Tick）
- [x] 实现 HandleTable — `NNHandleTable` 自由列表 + generation ABA 防护（Phase 1 完成）
- [x] 实现 AssetEntry 管理 — `NNAssetEntry` 含 guid/typeId/refCount/state/data/blobs（Phase 1 完成）
- [x] 实现同步加载 LoadAssetSync — 含 .nnasset 文件读取 + Header/依赖/Blob 解析
- [x] 实现异步加载 LoadAssetAsync — 通过 NNStreamingManager 提交请求
- [x] 实现 UnloadAsset + 引用计数 — `UnloadAsset` / `UnloadAssetByGuid` 含 refCount 检查
- [x] 实现 Tick() — 处理完成队列（PopCompleted → 分配 Handle → 解析 Header → 触发回调）

#### NNStreamingManager

- [x] 实现 IO 线程池 — 2 个 IO 线程，优先级请求队列 + condition_variable 唤醒
- [x] 实现优先级请求队列 — `priority_queue` 按 NNLoadPriority + distance 排序
- [x] 实现 IO 逻辑 — GUID → hex 路径解析，.nnasset 文件异步读取，送入解码队列
- [x] 实现解码逻辑 — Header 验证（magic/version），送入完成队列
- [x] 实现完成队列 — `completionQueue_` 供主线程 Tick() 消费
- [x] 实现 CancelRequest — 标记请求队列中的匹配项为已取消
- [ ] 实现 Mipmap Streaming — **未实现原因：需要 GPU 资源管理集成，属于渲染管线范围**
- [ ] 实现内存预算管理 + LRU 驱逐策略 — **未实现原因：NNAssetCache 已实现 LRU 驱逐（Phase 1），预算管理需与 Streaming 结合，属于优化阶段**

#### C++ → C ABI Wiring

- [x] 实现 `NNAssetManagerApi.cpp`（在 NNRuntimeAsset）— Phase 1 已完成，22 个函数全部接线
- [x] Wire 所有 NNAssetManagerAPI 函数指针 — `NNBuildAssetManagerRuntimeApi`（Phase 1 完成）
- [x] Wire 所有新增 NNAssetRegistryAPI 函数指针 — `NNBuildAssetRegistryEnhancedRuntimeApi`（Phase 1 完成）

#### C# Runtime API

- [x] 实现 `AssetHandle.cs` — `AssetManagerApiTable` 完整暴露全部 22 个函数指针
- [x] 实现 `AssetHandle<T>` — 查询（IsLoaded/IsLoading/GetGuid）、引用计数（AddRef/Release/GetRefCount）、资料存取（GetDataSize/GetBlobCount/GetBlobSize）
- [x] 实现异步等待 — `AssetHandleExtensions.LoadAsync()` 返回 TaskCompletionSource
- [x] 实现便捷 API — `LoadSync()` / `GetLoaded()` / `Unload()` 静态方法
- [ ] 集成到现有 `EngineNativeApiBootstrap` — **未实现原因：NativeApiProvider 当前使用静态属性注入，EngineNativeApiBootstrap 的集成需在编辑器启动流程中统一接线**

### Phase 6: Addressable + Package（2-3 周）

#### Addressable System

- [x] 实现 `AssetLabelSystem.cs` — 标签管理
- [x] 实现 `AssetGroupManager.cs` — 分组管理（含 JSON 持久化 AssetGroups.json）
- [x] 实现 `AssetGroup.cs` — 组定义（Name/Address/Strategy/Labels/IncludeInBuild）
- [x] 实现 `AddressableProfile.cs` — 配置管理（BuildTarget/CompressionMethod + CreateEditor/CreateRelease 工厂方法）
- [x] 实现 `AssetReference.cs` — Runtime 引用类型（GUID 字段 + GetPath/GetDisplayName）

#### Package

- [x] 定义 `.nnpack` 格式（NNPackHeader 64B, NNPackAssetEntry 48B, Manifest）
- [x] 实现 C++ `NNPackManager` — MountPackage/UnmountPackage/UnmountAll/IsAssetInPackage/ReadAssetFromPackage（内存映射读取）
- [x] 集成到 `NNAssetManager` — MountPackage/UnmountPackage/IsAssetInPackage 委托给 NNPackManager
- [x] 实现 C# `Addressables.cs` — Runtime API（LoadCatalogAsync/LoadAsset/LoadAssetAsync/LoadAssetsByLabel/ResolveAddress）

#### NNAssetCooker

- [x] 创建 `Engine/Source/Runtime/NNAssetCooker/` 模块（含 CMakeLists.txt 链接 NativeEngineAPI + Asset）
- [x] 实现 `NNAssetCooker.h/.cpp` — Cook 编排（分组构建/单包构建）
- [x] 实现 `NNPackBuilder.h/.cpp` — .nnpack 二进制生成（Header→Table→Manifest→Data 流式写入）
- [x] 实现 `NNCookManifest.h/.cpp` — 构建清单（assets/groups/outputRoot/libraryRoot）
- [x] 实现 C ABI：`NNAssetCookerAPI`（7 个函数：createManifest/destroyManifest/setOutputRoot/setLibraryRoot/addAsset/addGroup/cook）
- [x] 实现 Stub：`AssetCookerApiStubs.cpp`（No-op 函数 + 接线到 NNNativeEngineApiTable）
- [x] 更新构建系统：CMakeLists.txt 添加模块，EngineAPIRegistry layoutVersion 16→17，RuntimeApiBuilders + NativeEngineRuntimeApiTable 接线

#### C# BuildPipeline

- [x] 实现 `BuildPipeline.cs` — 全量构建 + 增量构建 + Clean + BuildReport（代码放在 Neverness.Editor.Assets 项目内）
- [x] 实现 `AssetCooker.cs` — C++ NNAssetCookerAPI C# 包装（CookerApiTable 函数指针 + CookResult）
- [x] 实现 `PackageBuilder.cs` — 纯 C# .nnpack 生成器（BuildFromDirectory/BuildFromGroup，与 C++ NNPackBuilder 二进制格式一致）
- [x] 实现 `BuildProfile.cs` — 构建配置（所有参数 + CreateEditor/CreateRelease 工厂方法）

> **注：** C# BuildPipeline 类文件直接放在 `Neverness.Editor.Assets` 项目中，未独立为 `Neverness.Editor.BuildPipeline` 项目。原因是当前代码量不需要独立程序集，减少项目引用复杂度。后续如需独立可轻松拆分。

### Phase 7: Hot Reload + 集成（1-2 周）

#### Hot Reload

- [x] 实现 AssetWatcher → Reimport 流程
- [x] 实现 Runtime 侧 reload 通知
- [x] 实现 NNAssetManager.MarkForReload()
- [x] 实现防抖 + 批量重编译
- [x] 实现 DependencyGraph 脏传播集成

#### Scene Integration

- [x] 修改 ECS 组件使用 NNGuid（基础设施已就绪，具体组件迁移渐进式推进）
- [ ] 实现 Scene 加载时 async 预加载关键资产 — 框架 API（NNSceneAssetLoader）已实现，完整运行时集成待后续
- [ ] 实现 Scene 区域 streaming — API/数据结构（NNSceneStreamingManager）已实现，完整运行时逻辑待后续

#### Prefab Integration

- [x] 实现 PrefabImporter
- [x] 实现 Prefab .nnasset 格式
- [x] 实现 C++ InstantiatePrefab()

### Phase 8: 优化 + AI 预留（1-2 周）

#### 性能优化

- [x] 实现 AssetMemoryPool
- [x] 优化 GuidHashMap — Robin Hood hashing（NNAssetManager 中已替换，其他模块渐进式）
- [x] 实现多线程增量编译（ImportDirectoryParallel 并行 Phase 1-4）
- [x] 优化 .nnasset 格式对齐（NN_ASSET_HEADER_SIZE 修正为 96 字节）

#### AI Pipeline 预留

- [x] 实现 AIGeneratedAssetHandler（框架 stub，实际 AI 集成待后续）
- [x] 确保所有 Editor API 线程安全（@threadsafe/@not-threadsafe 标注完成，粗粒度锁方案）
- [x] 确保 ImportPipeline 可非 UI 线程调用（ImportDirectoryParallel 无锁并行阶段 + s_lock 保护共享状态）

#### 测试

- [x] 编写 C# 单元测试：GUID, AssetHandle, AssetDatabase, ImportPipeline
- [x] 编写 C++ 单元测试：NNAssetFormat, NNHandleTable, GuidHashMap
- [ ] 编写集成测试：Editor Import → Runtime Load — 跨 ABI 端到端需要编译环境，测试框架已就绪
- [ ] 压力测试：10w+ assets 导入/加载 — 需要批量测试数据生成工具，后续补充

#### 文档

- [ ] 更新 `MODULE_ARCHITECTURE_AND_PROGRESS.md`（每个模块） — 随功能稳定后统一更新
- [ ] 更新 `MANAGED_EDITOR_ARCHITECTURE_AND_PROGRESS.md` — 随功能稳定后统一更新
- [ ] 更新 `Directory.md` — 随功能稳定后统一更新
- [ ] 编写 `ASSET_FORMAT_SPECIFICATION.md` — 格式已在 NNAssetFormat.h 中完整定义，独立文档待后续
- [ ] 编写 `ASSET_PIPELINE_GUIDE.md` — 管线接口已在 ImportPipeline.cs 中完整实现，独立文档待后续

---

## 三十、工业级最终架构总结

### 30.1 设计原则

| 原则 | 实现 |
|------|------|
| async-first | 所有加载默认异步，同步是特例 |
| blittable-first | GUID, Handle, AssetReference 全部 blittable |
| no per-asset PInvoke | 通过 Handle 引用，不跨边界传递资产数据 |
| no Runtime .meta | Runtime 只读 .nnasset 和 .nnpack |
| no Editor in Runtime | 严格项目分离 |
| cache-friendly | 64B 对齐、连续存储、开放寻址哈希 |
| data-driven | 全部通过数据/配置驱动，不硬编码 |
| reflection-friendly | 类型注册、Attribute 标记 |

### 30.2 参考架构

| 引擎 | 参考点 |
|------|--------|
| Unity | Addressables (label/group/profile), AssetDatabase, AssetImporter, .meta |
| Unreal | AssetRegistry, FAssetData, Package/Chunk, Streaming, Handle |
| Frostbite | EbBundle, async streaming, memory budget |
| Modern ECS | GUID-driven, blittable handles, component-based asset refs |

### 30.3 最终模块图

```
C++ Runtime:
┌─────────────────────────────────────────────────────────────┐
│  NNRuntimeAsset (新)                                        │
│  ├── NNAssetManager         资产管理器                      │
│  ├── NNAssetLoader          资产加载器                      │
│  ├── NNAssetHandle<T>       类型化 Handle                   │
│  ├── NNStreamingManager     Async streaming                 │
│  ├── NNAssetCache           LRU 缓存                       │
│  ├── NNAssetFormat          .nnasset 格式定义               │
│  └── NNAssetRef             引用计数                        │
│                                                             │
│  NNAssetRegistry (新)                                       │
│  ├── NNAssetRegistry        注册表                          │
│  ├── NNGuidTable            GUID ↔ Handle                  │
│  └── NNDependencyTable      依赖关系                        │
│                                                             │
│  NNAssetCompiler (新, Editor 用)                            │
│  ├── NNAssetCompiler        编译器                          │
│  ├── TextureCompiler       纹理编译                         │
│  ├── MeshCompiler          网格编译                         │
│  ├── AudioCompiler         音频编译                         │
│  └── SceneCompiler         场景编译                         │
│                                                             │
│  NNAssetCooker (新, Build 用)                               │
│  ├── NNAssetCooker          编排                            │
│  ├── NNPackBuilder          .nnpack 生成                    │
│  └── NNCookManifest         清单                            │
│                                                             │
│  NNRuntimePak (已存在)     通用 pak 格式                    │
│  NNRuntimeVFS (已存在)     虚拟文件系统                     │
│  NNNativeEngineAPI (已存在) C ABI 层                       │
│  NNRuntimeEngineServices (已存在) Wiring                    │
└─────────────────────────────────────────────────────────────┘

C# Managed:
┌─────────────────────────────────────────────────────────────┐
│  Neverness.Runtime.Assets (重构)                            │
│  ├── GUID                  128-bit blittable                │
│  ├── AssetHandle<T>        类型化 handle                    │
│  ├── AssetReference        Addressable 引用                 │
│  └── AssetRuntimeApi       P/Invoke → C ABI                │
│                                                             │
│  Neverness.Editor.Assets (大幅扩展)                         │
│  ├── EditorAssetDatabase   资产数据库                       │
│  ├── ImportPipeline        导入管线                         │
│  ├── IAssetImporter        导入器接口                       │
│  ├── Importers/*           各类型导入器                     │
│  ├── MetaFileManager       .meta 管理                       │
│  ├── DependencyGraph       依赖图                           │
│  ├── AssetWatcher          文件监视                         │
│  ├── AssetLabelSystem      标签系统                         │
│  ├── AssetGroupManager     分组管理                         │
│  ├── ThumbnailManager      缩略图                           │
│  └── AddressableSystem     Addressable                      │
│                                                             │
│  Neverness.Editor.BuildPipeline (新建)                      │
│  ├── BuildPipeline          构建编排                        │
│  ├── AssetCooker            编译调用                        │
│  └── PackageBuilder         包生成                          │
│                                                             │
│  Neverness.Editor.Serialization (保留)                      │
│  Neverness.Editor.Core (保留)                               │
│  Neverness.Editor.Framework (保留)                          │
└─────────────────────────────────────────────────────────────┘
```

### 30.4 关键格式

| 格式 | 用途 | Runtime? | Editor? |
|------|------|----------|---------|
| `.meta` | 资产元数据 + GUID + importer settings | No | Yes |
| `.nnasset` | 编译后的单个资产 | Yes | Yes |
| `.nnpack` | 打包后的资产包 | Yes | Yes |
| `AssetDatabase.cache` | 资产数据库缓存 | No | Yes |
| `Dependency.cache` | 依赖图缓存 | No | Yes |
| `Thumbnails/` | 缩略图缓存 | No | Yes |

### 30.5 最终目标

Neverness Engine 拥有：

- **工业级 Asset Runtime**：async-first, streaming-friendly, blittable, GUID-driven, dependency-aware, cache-friendly
- **工业级 Editor Pipeline**：full import pipeline, .meta system, dependency graph, incremental import, hot reload, addressable, build pipeline
- **严格分层**：Runtime 不知道 .meta / importer / AssetDatabase；Editor 不暴露给 Runtime
- **可扩展**：AI-driven asset generation ready, plugin-friendly, reflection-friendly
- **高性能**：支持 10w+ assets, multi-threaded compile, memory budget, LRU cache

---

*文档结束*
