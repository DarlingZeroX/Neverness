# NNRuntimeAsset — Runtime 资产系统核心

> CMake 目标 **`NevernessRuntime-Asset`**（STATIC）；C++ 命名空间 **`NN::Runtime::Asset`**。

## 1. 定位与边界

| 项目 | 说明 |
|------|------|
| **职责** | Runtime 资产生命周期管理：`.nnasset` 二进制格式定义、GUID→Handle 映射、同步/异步加载、引用计数、LRU 缓存、`.nnpack` 包挂载、异步 IO/解码管线、Hot Reload。 |
| **不负责** | 资产导入（`NNAssetImporter`）、资产编译/打包（`NNAssetCooker`）、Editor 侧 AssetDatabase。Runtime 不感知 `.meta` / importer。 |
| **CMake 目标** | `NevernessRuntime-Asset`（`STATIC`） |
| **命名空间** | `NN::Runtime::Asset` |
| **依赖** | `NevernessRuntime-NativeEngineAPI`（PUBLIC：`EngineTypes.h`、`EngineHandles.h`、`AssetManagerAPI.h`）；`NevernessRuntime-VFS`（PUBLIC）。 |
| **消费者** | `NNRuntimeEngineServices`、`NNRuntimeRenderer2D`、`NNRuntimeRenderAssets`、`NNRuntimeMedia`、`NNRuntimeMediaAssets`、`NNRuntimeAssetCooker`（PUBLIC）。 |

### 1.1 在资产管线中的位置

```
Editor AssetDatabase                    构建时
    │
    ▼
AssetImporter → .nnasset 文件           导入
    │
    ▼
AssetCooker → .nnpack 包                编译/打包
    │
    ▼
═══════════════════════════════════════  运行时边界
    │
    ▼
NNAssetManager（本模块）                 加载/缓存/查询
    ├── 同步加载：磁盘 → .nnasset 解析 → Handle
    ├── 异步加载：StreamingManager → IO 线程 → 解码线程 → 完成队列
    ├── 包加载：NNPackManager → 已挂载 .nnpack 中查找
    └── 缓存：NNAssetCache（LRU + 内存预算 + 钉选）
```

---

## 2. 构建与选项

- 在 `Engine/Source/Runtime/CMakeLists.txt` 中注册 `add_subdirectory("NNRuntimeAsset")`。
- **PUBLIC** 包含目录：`NNRuntimeAsset/Include`。
- **PRIVATE** 包含目录：`Engine/Source/Runtime`、`Engine/Source/Core`。
- `ENABLE_TESTS=ON` 且找到 GTest 时构建测试（3 个 GTest 可执行文件）。

```powershell
cmake --build Build/vs/x64-debug --target NevernessRuntime-Asset
```

---

## 3. 目录结构

```
Engine/Source/Runtime/NNRuntimeAsset/
├── CMakeLists.txt
├── Include/
│   ├── NNAssetFormat.h        # .nnasset 二进制格式定义（C ABI）
│   ├── NNPackFormat.h         # .nnpack 二进制格式定义（C ABI）
│   ├── NNAssetTypes.h         # FNV-1a 类型注册表
│   ├── NNAssetHandle.h        # 类型化 Handle 模板 + HandleTable
│   ├── NNAssetRef.h           # RAII 引用计数智能指针
│   ├── NNAssetCache.h         # LRU 内存预算缓存
│   ├── NNAssetManager.h       # 核心单例资产管理器
│   ├── NNStreamingManager.h   # 异步 IO + 解码线程池
│   ├── NNPackManager.h        # .nnpack 包挂载/查询管理器
│   ├── NNAssetMemoryPool.h    # 块内存池
│   └── GuidHashMap.h          # Robin Hood 开放寻址哈希表（header-only）
├── Private/
│   ├── NNAssetManager.cpp     # 资产管理器：同步/异步加载、Tick、Hot Reload
│   ├── NNHandleTable.cpp      # Handle 分配/释放/查询（generation 防 ABA）
│   ├── NNAssetCache.cpp       # LRU 驱逐 + 钉选
│   ├── NNStreamingManager.cpp # IO/解码线程池 + 完成队列
│   ├── NNAssetFormat.cpp      # ReadAssetHeader / ReadAssetFile / WriteAssetFile
│   ├── NNPackManager.cpp      # 包挂载、GUID 查找、包内资产读取
│   ├── NNAssetMemoryPool.cpp  # 固定块池 + malloc 降级
│   └── NNAssetManagerApi.cpp  # C ABI 桥接（填充 NNAssetManagerAPI 函数表）
└── Docs/
    └── MODULE_ARCHITECTURE_AND_PROGRESS.md
```

---

## 4. 核心类型

### 4.1 NNGuid（外部类型）

定义于 `EngineTypes.h`：`{ uint64_t high; uint64_t low; }`，128-bit 资产 GUID。与 C# `VisionGal.Managed.Assets.GUID` 镜像。

### 4.2 NNAssetHandleT<T>（类型化 Handle）

8 字节模板包装 `uint64_t`。`T = void` 为无类型变体。

| 方法 | 说明 |
|------|------|
| `NNAssetHandleT()` | 无效（value_=0） |
| `explicit NNAssetHandleT(uint64_t raw)` | 从原始值构造 |
| `explicit operator bool()` | 非零为 true |
| `Value()` | 原始值（跨 ABI 用） |
| `ToAbi()` / `FromAbi()` | C ABI handle 转换 |
| `operator==` / `operator!=` | 相等性比较 |

**别名**：`NNAssetHandleGeneric = NNAssetHandleT<void>`

Handle 编码：低 32 位 = index+1，高 32 位 = generation（防 ABA）。

### 4.3 NNAssetRef<T>（RAII 引用计数）

```cpp
NNAssetRef<T> ref(&manager, typedHandle);  // 构造 +1 ref
// ... 使用资产
// 析构自动 -1 ref
```

支持拷贝（+1）、移动（转移所有权）、`Reset()` 手动释放。

### 4.4 NNBlobDescriptor（Blob 描述符）

| 字段 | 类型 | 说明 |
|------|------|------|
| `offset` | `uint64` | 相对于 `payloadOffset` 的偏移 |
| `size` | `uint64` | 未压缩大小 |
| `compressedSize` | `uint64` | 压缩后大小（0=未压缩） |
| `blobType` | `uint32` | `NN_BLOB_TYPE_*` |
| `flags` | `uint32` | 预留标志 |

---

## 5. .nnasset 二进制格式

单个资产的编译后二进制文件，Runtime 不感知 `.meta` 或 importer。

### 5.1 文件布局

```
┌──────────────────────────────────┐
│ NNAssetHeader（96 字节）          │  ← magic='NNAS', version=2
├──────────────────────────────────┤
│ Dependency GUIDs（NNGuid × N）    │  ← 依赖资产 GUID 列表
├──────────────────────────────────┤
│ Blob Descriptors（32 字节 × M）   │  ← 各 blob 的位置/大小/类型
├──────────────────────────────────┤
│ Padding（64 字节对齐）             │
├──────────────────────────────────┤
│ Type-Specific Info（可选）         │  ← flags 有 HAS_TYPE_INFO 时存在
├──────────────────────────────────┤
│ Binary Payload（连续 blob 数据）    │  ← 所有 blob 紧密排列
└──────────────────────────────────┘
```

### 5.2 NNAssetHeader（96 字节，两行 cache line）

**第一行（0–63）：**

| 偏移 | 字段 | 类型 | 说明 |
|------|------|------|------|
| 0 | `magic` | `uint32` | `0x4E4E4153`（`'NNAS'`） |
| 4 | `version` | `uint32` | 格式版本（当前 `2`） |
| 8 | `assetGuid` | `NNGuid` | 128-bit 资产 GUID |
| 24 | `typeId` | `uint64` | 类型 ID（FNV-1a of type name） |
| 32 | `dependencyCount` | `uint32` | 依赖数量 |
| 36 | `blobCount` | `uint32` | blob 数量 |
| 40 | `dependencyOffset` | `uint64` | 依赖表偏移 |
| 48 | `blobTableOffset` | `uint64` | blob 表偏移 |
| 56 | `payloadOffset` | `uint64` | 载荷偏移 |

**第二行（64–95）：**

| 偏移 | 字段 | 类型 | 说明 |
|------|------|------|------|
| 64 | `payloadSize` | `uint64` | 载荷大小 |
| 72 | `flags` | `uint32` | 标志位 |
| 76 | `reserved0` | `uint32` | 预留 |
| 80 | `reserved1` | `uint64` | 预留 |
| 88 | `reserved2` | `uint64` | 预留 |

### 5.3 标志位

| 常量 | 值 | 说明 |
|------|----|------|
| `NN_ASSET_FLAG_COMPRESSED` | `1<<0` | Payload 使用 Zstd/LZ4 压缩 |
| `NN_ASSET_FLAG_STREAMING` | `1<<1` | 支持 streaming（mip/LOD 分 blob） |
| `NN_ASSET_FLAG_BUNDLE_MEMBER` | `1<<2` | 属于 bundle/package 成员 |
| `NN_ASSET_FLAG_HAS_TYPE_INFO` | `1<<3` | blob 表之后、payload 之前有额外类型信息 |

### 5.4 预定义类型 ID

| 常量 | 值 | 说明 |
|------|----|------|
| `NN_TYPE_ID_TEXTURE_2D` | `0x01` | 纹理 2D |
| `NN_TYPE_ID_MESH` | `0x02` | 网格 |
| `NN_TYPE_ID_AUDIO_CLIP` | `0x03` | 音频剪辑 |
| `NN_TYPE_ID_MATERIAL` | `0x04` | 材质 |
| `NN_TYPE_ID_SHADER` | `0x05` | 着色器 |
| `NN_TYPE_ID_SCENE` | `0x06` | 场景 |
| `NN_TYPE_ID_PREFAB` | `0x07` | Prefab |
| `NN_TYPE_ID_ANIMATION` | `0x08` | 动画 |
| `NN_TYPE_ID_LUA_SCRIPT` | `0x09` | Lua 脚本 |
| `NN_TYPE_ID_VIDEO_CLIP` | `0x0A` | 视频剪辑 |

### 5.5 Blob 类型

| 常量 | 值 | 说明 |
|------|----|------|
| `NN_BLOB_TYPE_DATA` | 0 | 通用数据 |
| `NN_BLOB_TYPE_MIP_LEVEL` | 1 | 纹理 mip level |
| `NN_BLOB_TYPE_VERTEX_BUF` | 2 | 顶点缓冲区 |
| `NN_BLOB_TYPE_INDEX_BUF` | 3 | 索引缓冲区 |
| `NN_BLOB_TYPE_THUMBNAIL` | 4 | 缩略图 |
| `NN_BLOB_TYPE_AUDIO_PCM` | 5 | 音频 PCM 数据 |
| `NN_BLOB_TYPE_AUDIO_SEEK` | 6 | 音频 seek table |
| `NN_BLOB_TYPE_ENTITY_HIERARCHY` | 7 | 场景实体层次 |
| `NN_BLOB_TYPE_COMPONENT_DATA` | 8 | ECS 组件数据 |
| `NN_BLOB_TYPE_TYPE_INFO` | 9 | 类型特定元数据 |
| `NN_BLOB_TYPE_VIDEO_FRAME` | 10 | 视频帧（RGBA 像素） |
| `NN_BLOB_TYPE_VIDEO_SEEK` | 11 | 视频 seek table（关键帧索引） |
| `NN_BLOB_TYPE_SUBTITLE` | 12 | 字幕数据 |

### 5.6 类型信息结构体

| 结构体 | 对应类型 | 字段 |
|--------|----------|------|
| `NNTextureTypeInfo` | Texture2D | width, height, format(DXGI_FORMAT), mipCount, arraySize, flags |
| `NNMeshTypeInfo` | Mesh | vertexCount, indexCount, vertexStride, indexFormat, boundsMin[3], boundsMax[3] |
| `NNAudioTypeInfo` | AudioClip | sampleRate, channels, sampleCount, format(NNAudioCompressionFormat), flags |
| `NNVideoTypeInfo` | VideoClip | width, height, fpsNum, fpsDen, frameCount, duration, codecId, flags, audioSampleRate, audioChannels |

### 5.7 工具函数

```cpp
uint64_t NNAssetAlign(uint64_t offset);              // 对齐到 64 字节边界
int      NNAssetHeaderIsValid(const NNAssetHeader*);  // 校验 magic + version
```

---

## 6. .nnpack 二进制格式

编译后的资产包格式（多个 `.nnasset` 打包为一个文件），由 `NNAssetCooker` 生成，运行时由 `NNPackManager` 加载。

### 6.1 文件布局

```
┌──────────────────────────────────┐
│ NNPackHeader（64 字节）           │  ← magic='NNPK', version=1
├──────────────────────────────────┤
│ Asset Table（48 字节 × N）        │  ← 64 字节对齐
├──────────────────────────────────┤
│ Manifest（UTF-8 包名）            │  ← 64 字节对齐
├──────────────────────────────────┤
│ Asset Data（各 .nnasset 原始字节） │  ← 每资产 64 字节对齐
└──────────────────────────────────┘
```

详见 [NNAssetCooker 模块文档](../../NNAssetCooker/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md) §6。

---

## 7. 核心 API

### 7.1 NNAssetManager（单例资产管理器）

#### 生命周期

| 方法 | 说明 |
|------|------|
| `Instance()` | 获取单例引用 |
| `Initialize(assetRoot)` | 初始化：注册默认类型、启动 StreamingManager（2 IO + 2 解码线程）、设置 512MB 缓存预算 |
| `Shutdown()` | 关闭：停止 StreamingManager、清除所有条目 |
| `Tick()` | 每帧调用：排空异步完成队列、处理 Hot Reload |

#### 加载

| 方法 | 说明 |
|------|------|
| `LoadAssetSync(guid, typeId)` | 同步加载，返回 `NNAssetHandleT<void>` |
| `LoadAssetByGuidLow(guidLow, typeId)` | 仅通过 `guid.low` 同步加载（纹理便捷方法） |
| `LoadAssetAsync(guid, typeId, priority, cb, userData)` | 异步加载，返回 `NNAsyncWaitHandle` |

#### 卸载

| 方法 | 说明 |
|------|------|
| `UnloadAsset(rawHandle)` | 引用计数 -1，到 0 时释放 |
| `UnloadAssetByGuid(guid)` | 通过 GUID 卸载 |

#### 查询

| 方法 | 说明 |
|------|------|
| `IsLoaded(rawHandle)` | 是否已加载 |
| `IsLoading(rawHandle)` | 是否正在加载 |
| `GetLoadedAsset(guid)` | 已加载资产的 Handle |
| `GetGuidByHandle(rawHandle)` | Handle → GUID 反查 |

#### 引用计数

| 方法 | 说明 |
|------|------|
| `AddRef(rawHandle)` | 引用 +1 |
| `ReleaseRef(rawHandle)` | 引用 -1（等同 UnloadAsset） |
| `GetRefCount(rawHandle)` | 当前引用计数 |

#### 数据访问

| 方法 | 说明 |
|------|------|
| `GetAssetData(rawHandle)` | 获取原始数据指针 |
| `GetAssetDataSize(rawHandle)` | 原始数据大小 |
| `GetBlobCount(rawHandle)` | blob 数量 |
| `GetBlobData(rawHandle, index)` | 第 index 个 blob 数据指针 |
| `GetBlobSize(rawHandle, index)` | 第 index 个 blob 大小 |
| `GetBlobDesc(rawHandle, index)` | 第 index 个 blob 描述符 |
| `GetBlobByType(rawHandle, blobType, &outData)` | 按类型查找 blob（线性扫描） |

#### 包管理

| 方法 | 说明 |
|------|------|
| `MountPackage(packPath)` | 挂载 `.nnpack`（委托 NNPackManager） |
| `UnmountPackage(packPath)` | 卸载包 |
| `IsAssetInPackage(guid)` | 资产是否在已挂载包中 |

#### Hot Reload

| 方法 | 说明 |
|------|------|
| `MarkForReload(guid)` | 标记资产需要重载 |
| `ReloadMarkedAssets()` | 重载所有已标记资产 |

#### 统计与子系统

| 方法 | 说明 |
|------|------|
| `GetLoadedAssetCount()` | 已加载资产数 |
| `GetTotalMemoryUsage()` | 总内存占用 |
| `GetHandleTable()` | HandleTable 引用 |
| `GetCache()` | Cache 引用 |
| `GetStreaming()` | StreamingManager 引用 |

#### 资产路径解析

`ResolveAssetPath(guid)` 生成：`Library/Imported/<2位hex前缀>/<32位hex-guid>.nnasset`

IO 线程还会搜索 `Assets/Imported/...` 和裸 `<guid>.nnasset` 作为降级路径。

### 7.2 NNAssetCache（LRU 缓存）

| 方法 | 说明 |
|------|------|
| `NNAssetCache(memoryBudget)` | 构造，默认 512MB |
| `SetMemoryBudget(bytes)` | 调整内存预算 |
| `GetCurrentUsage()` | 当前使用量 |
| `Touch(guid, memorySize, handle)` | 插入或 LRU 提升 |
| `Pin(guid)` / `Unpin(guid)` | 钉选（不被驱逐）/ 解除钉选 |
| `Remove(guid)` | 移除指定条目 |
| `Evict(requestedBytes, callback)` | 驱逐，返回释放字节数 |
| `Clear(callback)` | 清除非钉选条目 |
| `Contains(guid)` | 是否存在 |

### 7.3 NNPackManager（包管理器）

| 方法 | 说明 |
|------|------|
| `MountPackage(packPath)` | 读取整个 `.nnpack` 文件到内存并解析 |
| `UnmountPackage(packPath)` | 卸载指定包 |
| `IsAssetInPackage(guid)` | 资产是否在任何已挂载包中 |
| `GetPackageForAsset(guid)` | 返回资产所在包路径 |
| `ReadAssetFromPackage(guid, outData)` | 从包中读取资产原始数据 |
| `GetAssetTypeIdInPackage(guid)` | 包中资产的 typeId |
| `GetMountedPackages()` | 所有已挂载包路径 |
| `UnmountAll()` | 卸载全部 |

### 7.4 NNStreamingManager（异步管线）

三阶段管线：**请求队列**（优先级堆）→ **IO 线程** → **解码线程** → **完成队列**（主线程 `Tick()` 消费）。

| 方法 | 说明 |
|------|------|
| `Start(ioThreads, decodeThreads)` | 启动线程池（默认 2+2） |
| `Stop()` | 停止所有线程、清空队列 |
| `SubmitRequest(request)` | 提交异步加载请求 |
| `CancelRequest(guid)` | 取消指定请求 |
| `GetPendingRequestCount()` | 待处理请求数 |
| `PopCompleted(result)` | 非阻塞弹出完成结果 |

**加载优先级**：`CRITICAL(0) > HIGH(1) > NORMAL(2) > LOW(3) > BACKGROUND(4)`，同优先级按 `distance`（摄像机距离）升序。

### 7.5 NNAssetTypeRegistry（类型注册表）

| 方法 | 说明 |
|------|------|
| `RegisterType(typeName, predefinedId)` | 注册资产类型 |
| `GetTypeId(typeName)` | 名称 → ID（未注册返回 0） |
| `GetTypeName(typeId)` | ID → 名称（未知返回 ""） |
| `IsRegistered(typeId)` | 是否已注册 |

`Initialize()` 自动注册 9 个默认类型：Texture2D, Mesh, AudioClip, Material, Shader, Scene, Prefab, Animation, LuaScript。

---

## 8. 使用说明

### 8.1 初始化与关闭

```cpp
#include "NNAssetManager.h"
using namespace NN::Runtime::Asset;

// 启动时初始化
auto& mgr = NNAssetManager::Instance();
mgr.Initialize("E:/MyProject");  // 设置项目根目录

// 每帧更新（放在主循环中）
while (running) {
    mgr.Tick();  // 排空异步完成队列 + Hot Reload
    // ... 其他更新
}

// 关闭时清理
mgr.Shutdown();
```

### 8.2 同步加载资产

```cpp
#include "NNAssetFormat.h"
#include "NNAssetManager.h"
using namespace NN::Runtime::Asset;

auto& mgr = NNAssetManager::Instance();

// 按 GUID + 类型加载
NNGuid textureGuid{/* 从 Editor 或配置获取 */};
auto handle = mgr.LoadAssetSync(textureGuid, NN_TYPE_ID_TEXTURE_2D);

if (handle) {
    // 获取纹理元数据
    const auto* typeInfo = static_cast<const NNTextureTypeInfo*>(
        mgr.GetBlobByType(handle.Value(), NN_BLOB_TYPE_TYPE_INFO)
    );

    // 获取 mip level 0 数据
    const void* mipData = nullptr;
    auto* mipDesc = mgr.GetBlobByType(handle.Value(), NN_BLOB_TYPE_MIP_LEVEL, &mipData);
    if (mipDesc && mipData) {
        // mipData 指向像素数据，mipDesc->size 为大小
    }

    // 用完后卸载
    mgr.UnloadAsset(handle.Value());
}

// 仅通过 guid.low 加载（纹理便捷方式）
auto handle2 = mgr.LoadAssetByGuidLow(someLow64Bits, NN_TYPE_ID_TEXTURE_2D);
```

### 8.3 异步加载资产

```cpp
#include "NNAssetFormat.h"
#include "NNAssetManager.h"
using namespace NN::Runtime::Asset;

auto& mgr = NNAssetManager::Instance();

// 回调函数
void OnAssetLoaded(NNAssetHandle handle, int result, void* userData) {
    if (result == 0) {
        // 加载成功，handle 可用
        auto* mgr = static_cast<NNAssetManager*>(userData);
        const void* data = mgr->GetAssetData(handle);
        // ... 使用数据
    }
}

// 提交异步加载
NNGuid meshGuid{/* ... */};
auto waitHandle = mgr.LoadAssetAsync(
    meshGuid,
    NN_TYPE_ID_MESH,
    NN_LOAD_PRIORITY_NORMAL,    // 优先级
    OnAssetLoaded,              // 回调
    &mgr                        // 用户数据
);

// 每帧 Tick 会自动排空完成队列并调用回调
```

### 8.4 使用 RAII 引用计数

```cpp
#include "NNAssetRef.h"
#include "NNAssetManager.h"
using namespace NN::Runtime::Asset;

auto& mgr = NNAssetManager::Instance();
auto handle = mgr.LoadAssetSync(someGuid, NN_TYPE_ID_MATERIAL);

{
    // RAII 包装：构造 +1 ref，析构 -1 ref
    NNAssetRef<void> ref(&mgr, handle);

    if (ref) {
        // 安全使用，ref 存在期间资产不会被释放
        const void* data = mgr.GetAssetData(ref.GetHandle().Value());
    }

}   // ← ref 析构，自动 ReleaseRef

// 拷贝也安全（+1 ref）
NNAssetRef<void> copy = ref;
```

### 8.5 访问 Blob 数据

```cpp
auto& mgr = NNAssetManager::Instance();
auto handle = mgr.LoadAssetSync(audioGuid, NN_TYPE_ID_AUDIO_CLIP);
uint64_t raw = handle.Value();

// 获取 blob 数量
uint32_t blobCount = mgr.GetBlobCount(raw);

// 按索引遍历
for (uint32_t i = 0; i < blobCount; ++i) {
    const auto* desc = mgr.GetBlobDesc(raw, i);
    const void* data = mgr.GetBlobData(raw, i);
    uint64_t size = mgr.GetBlobSize(raw, i);

    switch (desc->blobType) {
        case NN_BLOB_TYPE_AUDIO_PCM:
            // data 指向 PCM 采样数据，size 为字节数
            break;
        case NN_BLOB_TYPE_AUDIO_SEEK:
            // data 指向 seek table
            break;
        case NN_BLOB_TYPE_TYPE_INFO:
            // data 指向 NNAudioTypeInfo
            break;
    }
}

// 按类型查找（线性扫描，适合偶尔查询）
const void* pcmData = nullptr;
auto* pcmDesc = mgr.GetBlobByType(raw, NN_BLOB_TYPE_AUDIO_PCM, &pcmData);
if (pcmDesc) {
    // pcmData 可用
}

// 类型信息也可以直接获取
auto* audioInfo = static_cast<const NNAudioTypeInfo*>(
    mgr.GetBlobByType(raw, NN_BLOB_TYPE_TYPE_INFO)
);
if (audioInfo) {
    uint32_t sampleRate = audioInfo->sampleRate;
    uint32_t channels = audioInfo->channels;
}
```

### 8.6 挂载 .nnpack 包

```cpp
auto& mgr = NNAssetManager::Instance();

// 挂载包（整个文件读入内存）
if (mgr.MountPackage("Build/Windows/game.nnpack")) {
    // 包内资产可通过 GUID 直接加载
    NNGuid assetGuid{/* ... */};
    if (mgr.IsAssetInPackage(assetGuid)) {
        auto handle = mgr.LoadAssetSync(assetGuid);  // 从包中读取
    }
}

// 卸载包
mgr.UnmountPackage("Build/Windows/game.nnpack");
```

### 8.7 缓存控制

```cpp
auto& cache = mgr.GetCache();

// 调整内存预算
cache.SetMemoryBudget(256 * 1024 * 1024);  // 256MB

// 钉选关键资产（不被驱逐）
cache.Pin(someGuid);
// ... 使用期间不会被 LRU 驱逐
cache.Unpin(someGuid);

// 手动驱逐
auto evicted = cache.Evict(64 * 1024 * 1024, [](NNGuid guid, uint64_t handle) {
    // 驱逐回调：释放 GPU 资源等
});

// 查看状态
uint64_t usage = cache.GetCurrentUsage();
uint64_t budget = cache.GetBudget();
uint32_t entries = cache.GetEntryCount();
```

### 8.8 Hot Reload 工作流

```cpp
// Editor 修改资产后通知 Runtime
mgr.MarkForReload(modifiedGuid);

// 每帧 Tick() 中自动处理已标记资产：
//   1. 重新读取 .nnasset 文件
//   2. 更新 HandleTable 中的数据
//   3. 触发回调通知消费者
// 或手动触发：
mgr.ReloadMarkedAssets();
```

### 8.9 自定义类型注册

```cpp
#include "NNAssetTypes.h"
using namespace NN::Runtime::Asset;

auto& typeReg = NNAssetTypeRegistry::Instance();

// 注册自定义类型（使用预定义 ID 确保跨版本稳定）
typeReg.RegisterType("ParticleSystem", 0x00000010ull);
typeReg.RegisterType("UILayout", 0x00000011ull);

// 查询
uint64_t id = typeReg.GetTypeId("ParticleSystem");  // 返回 0x10
const std::string& name = typeReg.GetTypeName(0x10);  // 返回 "ParticleSystem"
bool exists = typeReg.IsRegistered(0x10);  // true
```

### 8.10 通过 C ABI 使用（C#/Lua）

```cpp
// Native 侧：获取函数指针表
#include "AssetManagerAPI.h"
extern "C" void NNBuildAssetManagerRuntimeApi(NNAssetManagerAPI* api);

NNAssetManagerAPI api;
NNBuildAssetManagerRuntimeApi(&api);

// 初始化
api.initializeAssetManager("");

// 同步加载
NNAssetHandle handle = api.loadAssetSync(guid, typeId);

// 获取数据
const void* data = api.getAssetData(handle);
uint64_t size = api.getAssetDataSize(handle);

// 卸载
api.unloadAsset(handle);
```

C# 侧通过 `NNNativeEngineApiTypes.cs` 中的 `NNAssetManagerApi` 结构体（`delegate* unmanaged[Stdcall]`）调用。

### 8.11 统计监控

```cpp
auto& mgr = NNAssetManager::Instance();

uint64_t loadedCount = mgr.GetLoadedAssetCount();  // HandleTable 中的分配数
uint64_t memoryUsage = mgr.GetTotalMemoryUsage();   // Cache 中的内存占用
uint32_t pendingIO = mgr.GetStreaming().GetPendingRequestCount();  // 待处理 IO
```

---

## 9. C ABI 层

### 9.1 NNAssetManagerAPI（函数指针表）

定义于 `NNNativeEngineAPI/Include/AssetManagerAPI.h`，嵌入 `NNNativeEngineAPI.assetManager` 字段。

23 个函数指针覆盖：`loadAssetSync`、`loadAssetAsync`、`unloadAsset`、`unloadAssetByGuid`、`isAssetLoaded`、`isAssetLoading`、`getAssetByGuid`、`getGuidByAsset`、`addRef`、`releaseRef`、`getRefCount`、`getAssetData`、`getAssetDataSize`、`getBlobCount`、`getBlobData`、`getBlobSize`、`mountPackage`、`unmountPackage`、`isAssetInPackage`、`markForReload`、`reloadMarkedAssets`、`getLoadedAssetCount`、`getTotalMemoryUsage`、`initializeAssetManager`。

所有函数使用 `NN_ENGINE_ABI_STDCALL` 调用约定。

### 9.2 接线链路

```
EngineAPIRegistry.h
    └── NNNativeEngineAPI.assetManager（NNAssetManagerAPI）
            │
            ├── 真实路径：
            │   NNRuntimeEngineServices/NativeEngineRuntimeApiTable.cpp
            │       └── NNBuildAssetManagerRuntimeApi(&api->assetManager)
            │
            └── Stub 路径：
                NNRuntimeNativeEngineAPIStub/AssetManagerApiStubs.cpp
                    └── NNBuildAssetManagerApiStubs(&api->assetManager)
```

---

## 10. 依赖关系

```
NNRuntimeAsset 依赖：
├── NevernessRuntime-NativeEngineAPI（PUBLIC）
│   ├── EngineTypes.h        — NNGuid
│   ├── EngineHandles.h      — NNAssetHandle, NNAsyncWaitHandle
│   └── AssetManagerAPI.h    — NNAssetManagerAPI, NNLoadPriority
└── NevernessRuntime-VFS（PUBLIC）
    └── 虚拟文件系统抽象

依赖 NNRuntimeAsset 的模块：
├── NevernessRuntime-EngineServices    — 链接 + 接线 C ABI
├── NevernessRuntime-Renderer2D        — 纹理资产加载
├── NevernessRuntime-RenderAssets      — NNAssetHandleT, NNAssetManager
├── NevernessRuntime-Media             — 音频资产
├── NevernessRuntime-MediaAssets       — 视频资产
├── NevernessRuntime-AssetCooker（PUBLIC）— NNPackFormat.h
└── GTest 测试（×3）                   — 单元测试
```

---

## 11. 关键设计决策

| 决策 | 理由 |
|------|------|
| **Runtime 不感知 .meta / importer** | 解耦编辑器与运行时，Runtime 只读取编译后的二进制 |
| **async-first** | 大型资产（纹理/音频/视频）必须异步加载，避免帧率卡顿 |
| **Handle 而非指针** | 跨 ABI 安全、支持重定位、generation 防 ABA |
| **GUID 两级索引** | `GuidHashMap`（Robin Hood）提供 O(1) 查找，header-only 无外部依赖 |
| **LRU + 内存预算** | 可预测的内存占用，自动驱逐非钉选资产 |
| **钉选机制** | 关键资产（shader/材质）不被驱逐 |
| **64 字节对齐** | cache line 友好、SIMD/GPU 读取友好、SSD 页对齐 |
| **Blob 分类型** | 同一资产文件可包含多种数据（mip levels + type info + thumbnail） |
| **C ABI 函数表** | C#/Lua/LuaJIT 通过函数指针直接调用，零额外开销 |
| **STATIC 库** | 链入消费者模块，无需独立 DLL，减少动态链接开销 |

---

## 12. 开发进展

| 日期 | 进展 |
|------|------|
| 2026-05-23 | **Phase 1** 合入：`NevernessRuntime-Asset` STATIC 库、`.nnasset` 格式定义 + 文件读写、`NNHandleTable`（generation 防 ABA）、`NNAssetHandleT<T>` 模板、`NNAssetTypeRegistry`（FNV-1a）、`NNAssetManager` 单例（同步/异步加载 + 引用计数 + Hot Reload）、`NNAssetCache`（LRU + 内存预算 + 钉选）、`NNStreamingManager`（IO/解码线程池框架）、`NNAssetRef<T>` RAII、`NNAssetManagerApi.cpp` C ABI 桥接（23 函数指针）、`GuidHashMap`（Robin Hood header-only）。 |
| 2026-05-23 | **Phase 2** 合入：`NNPackManager`（`.nnpack` 挂载/查询/包内读取）、`NNAssetMemoryPool`（块内存池）、`.nnpack` 格式与 `.nnasset` 格式对齐（`NNPackFormat.h`）。 |

---

## 13. 未完成项

- **异步 IO 完善**：Overlapped IO / IOCP 高性能异步文件读取（当前为同步 fread + 线程池模拟）。
- **解码管线完善**：Zstd/LZ4 解压、格式转换（当前 `compressedSize` 未使用）。
- **包管理完善**：增量挂载、包版本校验、依赖包链。
- **Streaming 完善**：mip level 按需加载、LOD 分级 streaming。
- **Hot Reload 深度整合**：GPU 资源重建、消费者通知链。
- **GTest 测试覆盖**：3 个测试可执行文件的用例补充。
- **C# AssetCooker.cs 接线**：`GetCookerApi()` 当前返回空表（TODO：从 `NativeApiProvider` 接线）。

---

## 14. 未来规划

1. **高性能 IO**：Windows Overlapped IO / Linux io_uring，零拷贝异步文件读取。
2. **压缩支持**：Zstd 压缩集成，`NN_ASSET_FLAG_COMPRESSED` 启用，解码线程自动解压。
3. **Streaming 深化**：纹理 mip level 按距离/分辨率按需加载，音频 seek table 支持随机播放。
4. **包增量挂载**：支持热更新场景下的增量 `.nnpack` 覆盖。
5. **依赖解析**：自动加载依赖资产，依赖图拓扑排序。
6. **内存池集成**：`NNAssetMemoryPool` 替代 `std::vector<uint8_t>` 分配，减少碎片。

---

## 15. 相关文档

- [资产管线总架构](../../../Docs/1.Neverness%20Engine%20Industrial-grade%20Modern%20Asset%20Runtime%20and%20Asset%20Pipeline%20Architecture.md) — 运行时资产系统的完整架构设计
- [NNAssetCooker 模块文档](../../NNAssetCooker/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md) — `.nnpack` 打包器
- [纹理资产管线](../../../Docs/5.Texture-Asset-Pipeline-Architecture.md) — 纹理导入与 Runtime 加载
- [音视频资产管线](../../../Docs/15.Audio_Video_Asset_Pipeline_Architecture.md) — 音频/视频资产格式与加载
- `AssetManagerAPI.h`（`NNNativeEngineAPI/Include/`）— C ABI 函数指针表定义
