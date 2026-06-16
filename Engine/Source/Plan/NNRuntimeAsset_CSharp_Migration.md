# NNRuntimeAsset C# 迁移计划

> 将 C++ `NevernessRuntime-Asset` 模块的核心功能复刻到 C# 端，使 Runtime 资产系统完全由 C# 驱动。

## 0. 实施状态总览

| Phase | 名称 | 状态 | 备注 |
|-------|------|------|------|
| Phase 0 | 基础设施（格式定义） | ✅ **已完成** | AssetFormat / PackFormat / AssetTypeRegistry + Fnv1a64 |
| Phase 1 | Handle 系统 | ⚠️ **部分完成** | HandleTable 已实现；AssetHandle.cs 仍为 Native 包装 |
| Phase 2 | GUID 表 + 依赖表 + Registry | ✅ **已完成** | GuidTable / DependencyTable / AssetRegistry / AssetDatabase / DependencyTracking 已重写 |
| Phase 3 | LRU 缓存 | ✅ **已完成** | AssetCache（LRU + 钉选 + 512MB 预算） |
| Phase 4 | 异步加载管线 | ✅ **已完成** | StreamingManager（Channels + IO/Decode Workers） |
| Phase 5 | Pack 管理器 | ✅ **已完成** | PackManager + PackMount（MemoryMappedFile 零拷贝） |
| Phase 6 | 核心资产管理器 | ✅ **已完成** | AssetManager 单例，串联所有子系统 |
| Phase 7 | 重写已有薄包装类 | ✅ **已完成** | AssetHandle / AssetDatabase / Addressables / DependencyTracking / ImportPipeline 全部改为 C# |
| Phase 8 | ABI 层废弃 | ✅ **已完成** | C# + C++ 端 Asset ABI 全部删除；LayoutVersion 升至 24 |

**结论**：Phase 0-8 全部完成。C# AssetManager 全面接管 Runtime 资产系统，C++/C# Asset ABI 层已彻底删除。

## 1. 背景与目标

### 1.1 当前状态

C++ `NNRuntimeAsset` 模块（16 头文件、12 实现文件）是 Runtime 资产系统的核心，负责：
- `.nnasset` 二进制格式读写
- GUID ↔ Handle 映射（HandleTable + generation 防 ABA）
- 同步/异步资产加载
- LRU 内存缓存（512MB 预算 + 钉选）
- `.nnpack` 包挂载/查询
- 异步 IO/解码线程池（2 IO + 2 decode 线程）
- Hot Reload 支持
- C ABI 函数指针表桥接（23 + 16 = 39 个函数）

C# 端现有类是 **C++ ABI 的薄包装**：
- `AssetHandle<T>` → 调用 `NativeApiProvider.AssetManagerApi.IsAssetLoaded` 等
- `AssetDatabase` → 调用 `EngineNativeApiBootstrap.EngineApi.AssetRegistry.RegisterAsset` 等
- `Addressables` → 通过 `AssetHandleExtensions` 间接调用 Native
- `DependencyTracking` → 调用 `AssetRegistry.GetDependencyCount` 等

### 1.2 迁移目标

| 目标 | 说明 |
|------|------|
| **C# 成为权威源** | 资产生命周期完全由 C# 管理，不再依赖 C++ 运行时 |
| **保留 .nnasset 格式** | 二进制格式不变，保证 Editor→Runtime 兼容 |
| **保留 .nnpack 格式** | 打包格式不变，保证构建产物兼容 |
| **异步加载用 Channels** | `System.Threading.Channels` 生产者-消费者 + 限制并发度 |
| **Pack 用 MemoryMappedFile** | 零拷贝读取，`Span<byte>` 直传后续系统 |
| **ABI 层逐步废弃** | 迁移完成后删除 C++ ABI 函数指针表 |

### 1.3 不在范围内

- Editor 层资产导入管线（`EditorAssetDatabase`、`ImportPipeline`、`AssetCooker`）—— 后续单独迁移
- GPU Texture 资源管理（`NNRenderAssetApi` / `TextureInterop`）—— 属于渲染模块
- VFS 层（`NNRuntimeVFS`）—— 后续迁移，当前通过 `System.IO` 直接读文件

---

## 2. 架构决策

### 2.1 C# 端已有类的角色变化

| 类 | 当前角色 | 迁移后角色 | 当前状态 |
|----|----------|------------|----------|
| `AssetHandle<T>` | Native ABI 薄包装，所有方法通过函数指针调用 C++ | **完整实现**，内部持有 `AssetEntry` 引用，引用计数/数据存取全部 C# 完成 | ✅ **已重写**（调用 C# AssetManager） |
| `AssetDatabase` | Native `NNAssetRegistryApi` 薄包装 | **完整实现**，GUID↔Path 映射、依赖图全部 C# 内存管理 | ✅ **已重写**（调用 C# AssetRegistry） |
| `Addressables` | 通过 `AssetHandleExtensions` 间接调用 Native | **完整实现**，直接调用 C# `AssetManager`，不再经过 ABI | ✅ **已重写**（加载/释放直接调用 AssetManager） |
| `DependencyTracking` | Native `AssetRegistry` 薄包装 | **完整实现**，依赖图查询直接走 C# `DependencyTable` | ✅ **已重写**（调用 C# DependencyTable） |
| `ImportPipeline` (Runtime) | 调用 Native `importAsset` | **完整实现**，合成 GUID + 登记全部 C# 完成 | ✅ **已重写**（使用 C# AssetRegistry.ImportAsset） |
| `NativeApiProvider` | 桥接 `EngineNativeApiCache` 函数指针 | **废弃**（迁移完成后删除） | ✅ **已删除** |
| `AssetManagerApiTable` | C# 镜像 22 个函数指针 | **废弃**（迁移完成后删除） | ✅ **已删除** |

### 2.2 异步加载策略

**选型：`System.Threading.Channels` 生产者-消费者模式 + 限制并发度**

```
调用方 → Channel<LoadRequest> → IO Workers (有限并发) → Channel<DecodeRequest>
        → Decode Workers (有限并发) → Channel<LoadResult> → 主线程 Tick() 消费
```

- 使用 `Channel.CreateBounded<LoadRequest>(capacity: 256)` 背压控制
- IO Workers：`Task.Run` + `SemaphoreSlim(maxConcurrency)` 限制并发（默认 4）
- Decode Workers：同上（默认 2）
- 优先级：内部使用 `PriorityQueue<LoadRequest, int>` 在 Channel 消费端排序
- 取消：`CancellationTokenSource` 支持
- 延迟保证：如果实测延迟有问题，可退回专用 `Thread` + `BlockingCollection`

### 2.3 Pack 管理策略

**选型：`MemoryMappedFile` + `Span<byte>` 零拷贝**

```csharp
// 挂载 .nnpack
var mmf = MemoryMappedFile.CreateFromFile(packPath, FileMode.Open, null, 0, MemoryMappedFileAccess.Read);
var accessor = mmf.CreateViewAccessor(0, 0, MemoryMappedFileAccess.Read);

// 读取资产数据 — 零拷贝
unsafe ReadOnlySpan<byte> ReadAssetFromPackage(Guid guid)
{
    var entry = FindEntry(guid);
    byte* ptr = null;
    accessor.SafeMemoryMappedViewHandle.AcquirePointer(ref ptr);
    return new ReadOnlySpan<byte>(ptr + entry.Offset, (int)entry.Size);
}
```

- 挂载时只读 Header + AssetTable（很小），数据区域按需通过 `Span<byte>` 访问
- 避免将数据 marshal 成托管对象，减少 GC 压力
- 卸载时 `Dispose` accessor 和 mmf

---

## 3. 类映射表（C++ → C#）

| C++ 类/文件 | C# 目标 | 说明 |
|---|---|---|
| `NNAssetFormat.h/.cpp` | `AssetFormat.cs` (新) | `.nnasset` 二进制格式读写，`Span<byte>` 操作 |
| `NNPackFormat.h` | `PackFormat.cs` (新) | `.nnpack` 二进制格式定义，`StructLayout` 对齐 |
| `NNAssetTypes.h` | `AssetTypeRegistry.cs` (新) | FNV-1a 类型注册表 |
| `NNAssetHandle.h` + `NNHandleTable.cpp` | `HandleTable.cs` (重写) | generation 防 ABA，泛型 `AssetHandle<T>` 保留 |
| `NNAssetRef.h` | `AssetRef.cs` (新) | RAII 引用计数（`IDisposable` 模式） |
| `NNAssetCache.h/.cpp` | `AssetCache.cs` (新) | LRU 缓存 + 钉选 + 内存预算 |
| `NNAssetManager.h/.cpp` | `AssetManager.cs` (新) | 核心单例，同步/异步加载、Tick、Hot Reload |
| `NNStreamingManager.h/.cpp` | `StreamingManager.cs` (新) | Channels 生产者-消费者异步管线 |
| `NNPackManager.h/.cpp` | `PackManager.cs` (新) | MemoryMappedFile 包管理 |
| `NNAssetMemoryPool.h/.cpp` | `AssetMemoryPool.cs` (新) | 可选，C# 有 GC 可能不需要 |
| `NNAssetRegistry.h/.cpp` | `AssetRegistry.cs` (重写 `AssetDatabase`) | GUID↔Path + 依赖图 |
| `NNGuidTable.h/.cpp` | `GuidTable.cs` (新) | GUID↔Path 双向映射 |
| `NNDependencyTable.h/.cpp` | `DependencyTable.cs` (新) | 前向/反向依赖图 + 环检测 |
| `GuidHashMap.h` | 不需要 | C# 用 `Dictionary<ulong, T>` 即可 |
| `NNAssetManagerApi.cpp` | 废弃 | ABI 层逐步删除 |
| `NNAssetRegistryApi.cpp` | 废弃 | ABI 层逐步删除 |

---

## 4. Phase 分解

### Phase 0：基础设施（.nnasset/.nnpack 格式定义） ✅ 已完成

**目标**：定义二进制格式的 C# 结构体，保证与 C++ 内存布局一致。

**新建文件**：
- `Runtime/Neverness.Runtime.Assets/Formats/AssetFormat.cs` ✅
  - `AssetHeader` (96 bytes, `StructLayout Sequential, Pack=8`) ✅
  - `BlobDescriptor` (32 bytes) ✅
  - `TextureTypeInfo`, `MeshTypeInfo`, `AudioTypeInfo`, `VideoTypeInfo` ✅
  - `AssetFormatReader` — `ReadHeader(ReadOnlySpan<byte>)`, `ReadFile(ReadOnlySpan<byte>)` ✅
  - `AssetFormatWriter` — `WriteFile(...)` → `byte[]` ✅
- `Runtime/Neverness.Runtime.Assets/Formats/PackFormat.cs` ✅
  - `PackHeader` (64 bytes) ✅
  - `PackAssetEntry` (48 bytes) ✅
  - `PackFlags` 常量 ✅
- `Runtime/Neverness.Runtime.Assets/Formats/AssetTypeRegistry.cs` ✅
  - `TypeId` 常量（与 C++ `NN_TYPE_ID_*` 对齐） ✅
  - `BlobType` 常量（与 C++ `NN_BLOB_TYPE_*` 对齐） ✅
  - `AssetTypeRegistry` — FNV-1a 注册表 ✅
  - `Fnv1a64` — 哈希函数 ✅

**验证**：写单元测试验证 `sizeof(AssetHeader) == 96`，字段偏移与 C++ 一致。⚠️ 单元测试待补。

**依赖**：无外部依赖。

---

### Phase 1：Handle 系统 ⚠️ 部分完成

**目标**：实现 generation 防 ABA 的 Handle 分配表。

**重写文件**：
- `Runtime/Neverness.Runtime.Assets/AssetHandle.cs` — ❌ **未重写**，仍调用 `NativeApiProvider.AssetManagerApi`

**新建文件**：
- `Runtime/Neverness.Runtime.Assets/HandleTable.cs` ✅
  - `HandleTable` — 线程安全 Handle 分配/释放/查询 ✅
  - `Slot` — `{object? Data, ulong TypeId, int RefCount, uint Generation, bool Alive}` ✅
  - Handle 编码：低 32 位 = index+1，高 32 位 = generation ✅
  - `Allocate(object data, ulong typeId) -> ulong` ✅
  - `Free(ulong handle)` ✅
  - `Resolve(ulong handle) -> object?` ✅
  - `AddRef(ulong handle)`, `Release(ulong handle) -> bool`, `GetRefCount(ulong handle)` ✅

**关键细节**：
- 使用 `lock` 保护（与 C++ `std::mutex` 对应） ✅
- `Interlocked.Increment/Decrement` 用于 `RefCount`（与 C++ `std::atomic` 对应） ✅
- free list 用 `Stack<int>` 实现 ✅

**依赖**：Phase 0（`AssetTypeId` 常量）

**剩余工作**：重写 `AssetHandle<T>` 的所有方法改为调用 C# `AssetManager.Instance`，删除 `NativeApiProvider` 调用。

---

### Phase 2：GUID 表 + 依赖表 + Registry ✅ 已完成

**目标**：实现资产注册表的核心数据结构。

**新建文件**：
- `Runtime/Neverness.Runtime.Assets/Registry/GuidTable.cs` ✅
  - `GuidTable` — `Dictionary<ulong, string>` guid→path + `Dictionary<string, GUID>` path→guid ✅
  - `Register(GUID, string path)`, `UnregisterByGuid(GUID)`, `UnregisterByPath(string)` ✅
  - `ResolvePath(GUID) -> string?`, `ResolveGuid(string) -> GUID?` ✅

- `Runtime/Neverness.Runtime.Assets/Registry/DependencyTable.cs` ✅
  - `DependencyTable` — 前向 `Dictionary<ulong, List<GUID>>` + 反向 `Dictionary<ulong, List<GUID>>` ✅
  - `SetDependencies(GUID asset, ReadOnlySpan<GUID> deps)` ✅
  - `AddDependency(GUID asset, GUID dep)`, `RemoveDependency(GUID asset, GUID dep)` ✅
  - `GetDependencies(GUID) -> IReadOnlyList<GUID>` ✅
  - `GetReverseDependencies(GUID) -> IReadOnlyList<GUID>` ✅
  - `HasCycle() -> bool` — DFS 白/灰/黑标记 ✅
  - `GetNodeCount()`, `GetEdgeCount()` ✅

- `Runtime/Neverness.Runtime.Assets/Registry/AssetRegistry.cs` ✅
  - 组合 `GuidTable` + `DependencyTable` ✅
  - 提供统一 API：`RegisterAsset`, `UnregisterByGuid`, `ResolvePathByGuid`, `SetDependencies` 等 ✅
  - `ImportAsset(string path) -> GUID` — FNV-1a 合成 GUID ✅

**重写文件**：
- `Runtime/Neverness.Runtime.Assets/AssetDatabase.cs` ✅ — 已改为调用 C# `AssetRegistry`
- `Runtime/Neverness.Runtime.Assets/DependencyTracking.cs` ✅ — 已改为调用 C# `DependencyTable`

**依赖**：Phase 0（`Fnv1a64`、`GUID`）

---

### Phase 3：LRU 缓存 ✅ 已完成

**目标**：实现带内存预算和钉选的 LRU 缓存。

**新建文件**：
- `Runtime/Neverness.Runtime.Assets/AssetCache.cs` ✅
  - `AssetCache` — LRU 缓存 ✅
  - 使用 `LinkedList<GUID>` 做 LRU 排序（front = 最近使用） ✅
  - 使用 `Dictionary<ulong, CacheSlot>` 做 O(1) 查找（key = guid.low） ✅
  - `CacheSlot` = `{CacheEntry Entry, LinkedListNode<GUID> Node}` ✅
  - `CacheEntry` = `{GUID Guid, long MemorySize, ulong Handle, bool Pinned}` ✅
  - `Touch(GUID, long size, ulong handle)` — 插入或更新，移到 LRU 前端 ✅
  - `Pin(GUID)`, `Unpin(GUID)` — 钉选/取消钉选 ✅
  - `Evict(long requestedBytes, Action<GUID, ulong> onEvict)` — 从尾部驱逐，跳过 pinned ✅
  - `Clear(Action<GUID, ulong> onEvict)` — 清除所有非 pinned 条目 ✅
  - `MemoryBudget` 属性（默认 512MB） ✅
  - `CurrentUsage` 属性 ✅

**关键细节**：
- 线程安全（所有公共方法加 `lock`） ✅
- `Evict` 在 LRU 尾部开始，跳过 pinned 条目 ✅
- 内存预算通过 `CacheEntry.MemorySize` 累加追踪 ✅

**依赖**：Phase 0（`GUID`）

---

### Phase 4：异步加载管线（StreamingManager） ✅ 已完成

**目标**：用 `System.Threading.Channels` 实现生产者-消费者异步加载。

**新建文件**：
- `Runtime/Neverness.Runtime.Assets/Streaming/StreamingManager.cs` ✅
  - `StreamingManager` — 异步 IO/解码管线 ✅
  - `Channel<LoadRequest>` — 请求队列（bounded, capacity 256） ✅
  - `Channel<(LoadRequest, byte[]?, bool, string?)>` — IO 结果队列 ✅
  - `Channel<LoadResult>` — 完成队列 ✅
  - IO Workers：`SemaphoreSlim(ioConcurrency)` 限制并发（默认 4） ✅
  - Decode Workers：`SemaphoreSlim(decodeConcurrency)` 限制并发（默认 2） ✅
  - `SubmitRequestAsync(GUID, ulong typeId, LoadPriority, float distance, CancellationToken) -> ValueTask<ulong>` ✅
  - `DrainCompleted(Action<LoadResult>)` — 主线程 Tick 调用，消费完成队列 ✅
  - 优先级排序：内部 `PriorityQueue` 在消费端排序 ✅

- `Runtime/Neverness.Runtime.Assets/Streaming/LoadRequest.cs` ✅
  - `LoadRequest` — `{GUID, TypeId, Priority, CancellationToken, TaskCompletionSource<ulong>}` ✅
  - `LoadResult` — `{GUID, TypeId, byte[] Data, bool Success, string? ErrorMessage}` ✅
  - `LoadPriority` 枚举（已有，保留） ✅

**关键细节**：
- IO 线程：读取 .nnasset 文件（`File.ReadAllBytesAsync`） ✅
- 解码线程：预留（当前 header 验证在 AssetManager.ParseNnassetData 中完成） ✅
- 完成队列：主线程 `Tick()` → `DrainCompleted` 消费 ✅
- 取消支持：`CancellationToken` 贯穿整个管线 ✅
- 背压：bounded channel 在队列满时阻塞生产者 ✅

**依赖**：Phase 0（格式解析）、Phase 1（Handle 分配）

**说明**：与原计划的差异——`LoadResult` 类定义在 `LoadRequest.cs` 中（而非独立的 `LoadResult.cs`）；`DrainCompleted` 由 AssetManager.Tick() 调用而非独立的 Tick()。

---

### Phase 5：Pack 管理器 ✅ 已完成

**目标**：用 `MemoryMappedFile` 实现 .nnpack 包管理。

**新建文件**：
- `Runtime/Neverness.Runtime.Assets/Pack/PackManager.cs` ✅
  - `PackManager` — 单例，.nnpack 包管理 ✅
  - `MountPackage(string packPath)` — 打开 `MemoryMappedFile`，读取 Header + AssetTable ✅
  - `UnmountPackage(string packPath)` — Dispose mmf ✅
  - `IsAssetInPackage(GUID) -> bool` ✅
  - `ReadAssetFromPackage(GUID) -> ReadOnlySpan<byte>` — 零拷贝，通过 `Span<byte>` 直接访问 ✅
  - `ReadAssetFromPackageCopy(GUID) -> byte[]?` — 安全版本，跨异步边界用 ✅
  - `GetMountedPackages() -> IReadOnlyList<string>` ✅
  - `UnmountAll()` — 清理所有挂载 ✅

- `Runtime/Neverness.Runtime.Assets/Pack/PackMount.cs` ✅
  - `PackMount` — 单个挂载的 .nnpack ✅
  - `MemoryMappedFile Mmf` ✅
  - `MemoryMappedViewAccessor Accessor` ✅
  - `PackHeader Header` ✅
  - `PackAssetEntry[] AssetTable` ✅
  - `Dictionary<ulong, int> GuidToIndex` — guid.low → table index ✅
  - `ReadAssetData(int entryIndex) -> ReadOnlySpan<byte>` — 零拷贝 unsafe ✅
  - `ReadAssetDataCopy(int entryIndex) -> byte[]?` — 安全版本 ✅

**关键细节**：
- 挂载时只读 Header (64 bytes) + AssetTable (48 bytes × assetCount)，数据按需通过 `Span<byte>` 访问 ✅
- `ReadOnlySpan<byte>` 返回值要注意生命周期（不能逃逸到异步上下文） ✅
- 如果需要跨异步边界传递，复制为 `byte[]` ✅
- 卸载时 `Dispose` accessor 和 mmf ✅

**依赖**：Phase 0（`PackFormat`）

---

### Phase 6：核心资产管理器（AssetManager） ✅ 已完成

**目标**：实现核心单例，串联所有子系统。

**新建文件**：
- `Runtime/Neverness.Runtime.Assets/AssetManager.cs` ✅
  - `AssetManager` — 单例，核心资产管理器 ✅
  - 内部持有：`HandleTable`、`AssetCache`、`StreamingManager`、`PackManager`、`AssetRegistry` ✅
  - `Dictionary<ulong, AssetEntry> GuidToEntry` — guid.low → entry ✅
  - `Dictionary<ulong, AssetEntry> HandleToEntry` — handle → entry ✅

**生命周期方法**：
- `Initialize(string assetRoot)` — 注册默认类型，初始化子系统 ✅
- `Shutdown()` — 清理所有资源 ✅
- `Tick()` — 主线程调用，消费异步完成队列，处理 Hot Reload ✅

**同步加载**：
- `LoadAssetSync(GUID guid, ulong typeId) -> ulong` (handle) ✅
- 读取 .nnasset 文件（直接文件 IO 或 Pack） ✅
- 解析 header + dependencies + blobs ✅
- 分配 Handle，注册到缓存 ✅

**异步加载**：
- `LoadAssetAsync(GUID guid, ulong typeId, LoadPriority priority, CancellationToken ct) -> ValueTask<ulong>` ✅
- 提交到 `StreamingManager`，`Tick()` 消费完成结果 ✅

**卸载**：
- `UnloadAsset(ulong handle)` — 释放 Handle，从缓存移除 ✅
- `UnloadAssetByGuid(GUID guid)` — 按 GUID 卸载 ✅

**查询**：
- `IsLoaded(ulong handle) -> bool` ✅
- `IsLoading(ulong handle) -> bool` ✅
- `GetLoadedAsset(GUID) -> ulong?` ✅
- `GetGuidByHandle(ulong handle) -> GUID?` ✅

**引用计数**：
- `AddRef(ulong handle)`, `ReleaseRef(ulong handle)`, `GetRefCount(ulong handle)` ✅

**数据存取**：
- `GetAssetData(ulong handle) -> ReadOnlySpan<byte>` ✅
- `GetAssetDataSize(ulong handle) -> long` ✅
- `GetBlobCount(ulong handle) -> int` ✅
- `GetBlobData(ulong handle, int index) -> ReadOnlySpan<byte>` ✅
- `GetBlobSize(ulong handle, int index) -> long` ✅
- `GetBlobByType(ulong handle, uint blobType) -> ReadOnlySpan<byte>` ✅（额外扩展）

**包管理**：
- `MountPackage(string path)`, `UnmountPackage(string path)` ✅
- `IsAssetInPackage(GUID) -> bool` ✅

**Hot Reload**：
- `MarkForReload(GUID)`, `ReloadMarkedAssets()` ✅

**内部类型**：
- `AssetEntry` — `{GUID, TypeId, ulong Handle, int RefCount, AssetState State, List<GUID> Dependencies, byte[] Data, BlobDescriptor[] Blobs, int PayloadOffset, string? SourcePath, bool NeedsReload}` ✅
- `AssetState` 枚举：`Unloaded, Loading, Loaded, Failed, Streaming` ✅

**依赖**：Phase 1-5 全部

---

### Phase 7：重写已有薄包装类 ✅ 已完成

**目标**：将现有 C# 薄包装类改为调用 C# `AssetManager`，删除 Native 调用。

**修改文件**：
- `AssetHandle.cs` — ✅ **已重写**，所有方法调用 `AssetManager.Instance`；删除 `NativeApiProvider` / `AssetManagerApiTable` / `AssetHandleExtensions`
- `AssetDatabase.cs` — ✅ **已重写**（Phase 2 完成），改为调用 C# `AssetRegistry`
- `Addressables.cs` — ✅ **已重写**，加载/释放直接调用 `AssetManager.Instance`
- `DependencyTracking.cs` — ✅ **已重写**（Phase 2 完成），改为调用 C# `DependencyTable`
- `ImportPipeline.cs` (Runtime) — ✅ **已重写**，使用 C# `AssetRegistry.ImportAsset()` 合成 GUID

**额外修改**：
- `AssetsModuleImp.cs` — ✅ Bootstrap 改为 `AssetManager.Instance.Initialize()`
- `AudioAssetOpener.cs` — ✅ `NativeApiProvider.AssetManagerApi.GetBlobSize` → `AssetManager.Instance.GetBlobSize`
- `TextureAssetOpener.cs` — ✅ `AssetHandleExtensions.LoadSync` → `AssetHandle.LoadSync`
- `VideoAssetOpener.cs` — ✅ `AssetHandleExtensions.LoadSync` → `AssetHandle.LoadSync`
- `HotReloadCoordinator.cs` — ✅ `AssetHandleExtensions.MarkForReload/ReloadMarkedAssets` → `AssetHandle.MarkForReload/ReloadMarkedAssets`
- `AssetSystemTests.cs` — ✅ 删除 NativeApiProvider 测试，新增 AssetManager 测试

**已删除类型**：
- `NativeApiProvider` 静态类
- `AssetManagerApiTable` 类
- `AssetHandleExtensions` 静态类
- `AssetLoadCompletedCallback` 委托

**依赖**：Phase 6

---

### Phase 8：ABI 层废弃 + 清理 ✅ 已完成

**目标**：删除 C++/C# Asset ABI 层。

**已完成步骤**：
1. ✅ `TextureInterop.LoadTextureFromBlob` 改为使用 C# `AssetManager.Instance.GetBlobDataPtr`
2. ✅ 删除 `NativeApiProvider` 类、`AssetManagerApiTable` 类、`AssetHandleExtensions` 静态类
3. ✅ 删除 C# `NNAssetManagerApi`、`NNAssetRegistryApi` 结构体
4. ✅ 删除 C# `NNAssetHandle` 类型
5. ✅ 删除 C++ `AssetManagerAPI.h`、`AssetRegistryAPI.h`
6. ✅ 删除 C++ `NNAssetManagerApi.cpp`、`NNAssetRegistryApi.cpp`
7. ✅ 删除 C++ `AssetRegistryRuntimeApi.cpp`（NNRuntimeEngineServices 模块）
8. ✅ 删除 C++ Stub 文件（`AssetManagerApiStubs.cpp`、`AssetRegistryApiStubs.cpp`、`AssetRegistryStubDatabase.cpp`）
9. ✅ 更新 `EngineAPIRegistry.h`：移除 `assetRegistry`、`assetManager` 字段，LayoutVersion 23→24
10. ✅ 更新 `NNNativeEngineApi` C# 聚合体：移除 `AssetRegistry`、`AssetManager` 字段
11. ✅ 更新 `NNNativeEngineApiConstants.LayoutVersion`：23→24
12. ✅ 更新 `NativeEngineRuntimeApiTable.cpp`、`RuntimeApiBuilders.h`、CMakeLists.txt

**依赖**：Phase 7

---

## 5. 文件结构（当前状态）

```
Engine/Source/Managed/Runtime/Neverness.Runtime.Assets/
├── Formats/
│   ├── AssetFormat.cs          ✅ .nnasset 二进制格式读写
│   ├── PackFormat.cs           ✅ .nnpack 二进制格式定义
│   └── AssetTypeRegistry.cs    ✅ FNV-1a 类型注册表 + 常量
├── Registry/
│   ├── GuidTable.cs            ✅ GUID↔Path 双向映射
│   ├── DependencyTable.cs      ✅ 前向/反向依赖图 + 环检测
│   └── AssetRegistry.cs        ✅ 组合 GuidTable + DependencyTable
├── Streaming/
│   ├── StreamingManager.cs     ✅ Channels 生产者-消费者异步管线
│   └── LoadRequest.cs          ✅ 加载请求 + 加载结果（LoadResult 合并在此文件）
├── Pack/
│   ├── PackManager.cs          ✅ MemoryMappedFile 包管理
│   └── PackMount.cs            ✅ 单个挂载的 .nnpack
├── HandleTable.cs              ✅ generation 防 ABA Handle 分配表
├── AssetCache.cs               ✅ LRU 缓存 + 钉选 + 内存预算
├── AssetManager.cs             ✅ 核心单例资产管理器
├── AssetHandle.cs              ✅ (已重写) 泛型 Handle，调用 C# AssetManager
├── AssetDatabase.cs            ✅ (已重写) 调用 C# AssetRegistry
├── Addressables.cs             ✅ (已重写) 加载/释放直接调用 AssetManager
├── DependencyTracking.cs       ✅ (已重写) 调用 C# DependencyTable
├── ImportPipeline.cs           ✅ (已重写) 使用 C# AssetRegistry.ImportAsset
├── GUID.cs                     ✅ (保留) 128-bit GUID
├── NPath.cs                    ✅ (保留) OS 路径
├── NVirtualPath.cs             ✅ (保留) VFS 路径
└── Neverness.Runtime.Assets.csproj
```

**注意**：`AssetRef.cs`（IDisposable 引用计数智能指针）未创建，当前引用计数由 `HandleTable` + `AssetEntry.RefCount` 直接管理。

---

## 6. 测试计划

### 6.1 单元测试（每个 Phase）

| Phase | 测试内容 | 状态 |
|-------|----------|------|
| Phase 0 | `AssetHeader` 大小/偏移验证、`ReadFile`/`WriteFile` 往返、`Fnv1a64` 与 C++ 一致 | ⚠️ 待补 |
| Phase 1 | Handle 分配/释放/generation 防 ABA/引用计数 | ⚠️ 待补 |
| Phase 2 | GUID↔Path 双向映射、依赖增删查、环检测 | ⚠️ 待补 |
| Phase 3 | LRU 驱逐顺序、钉选保护、内存预算 | ⚠️ 待补 |
| Phase 4 | Channel 背压、优先级排序、取消支持、并发安全 | ⚠️ 待补 |
| Phase 5 | Pack 挂载/卸载、GUID 查找、Span 零拷贝读取 | ⚠️ 待补 |
| Phase 6 | 同步/异步加载完整流程、Hot Reload、包加载 | ⚠️ 待补 |

### 6.2 集成测试

- 编译一个测试用 `.nnasset` 文件，验证 C# 读取与 C++ 读取结果一致
- 编译一个测试用 `.nnpack` 文件，验证 C# 挂载/查询/读取
- 异步加载压力测试：并发 100 个加载请求，验证完成顺序和正确性
- Hot Reload 测试：修改文件 → 触发 Reload → 验证新数据

---

## 7. 风险与缓解

| 风险 | 影响 | 缓解 |
|------|------|------|
| `Span<byte>` 生命周期管理 | 零拷贝 Span 不能逃逸异步上下文 | 需要跨异步边界时复制为 `byte[]`；文档明确使用规范 |
| Channels 延迟抖动 | ThreadPool 调度可能不如专用线程可预测 | 实测后如果延迟有问题，退回专用 `Thread` + `BlockingCollection` |
| MemoryMappedFile 跨平台 | Linux/macOS 行为可能不同 | .NET 的 `MemoryMappedFile` 已跨平台抽象，但需测试 |
| ABI 层废弃时序 | 下游模块（Renderer2D 等）还未迁移 | Phase 8 等所有下游迁移完成后再执行；过渡期保留 ABI 兼容 |
| 线程安全 | C# `lock` 性能不如 C++ `std::mutex` | 使用 `ReaderWriterLockSlim` 优化读多写少场景 |

---

## 9. 双系统并行风险（已彻底消除）

**Phase 7 + Phase 8 全部完成后**：C# AssetManager 成为唯一权威源，C++/C# Asset ABI 层已彻底删除（LayoutVersion 24）。`TextureInterop` 已改为通过 C# `AssetManager.Instance.GetBlobDataPtr` 获取 blob 数据，不再依赖 Native AssetManager API。

---

## 8. 里程碑

| 里程碑 | Phase | 状态 | 实际产出 |
|--------|-------|------|----------|
| M0：格式定义 | 0 | ✅ 已完成 | `AssetFormat` / `PackFormat` / `AssetTypeRegistry` + `Fnv1a64` |
| M1：Handle 系统 | 1 | ⚠️ 部分完成 | `HandleTable` 已实现；`AssetHandle<T>` 待重写 |
| M2：Registry | 2 | ✅ 已完成 | `AssetRegistry` + `DependencyTable` + `GuidTable` + 重写 `AssetDatabase` / `DependencyTracking` |
| M3：缓存 | 3 | ✅ 已完成 | `AssetCache`（LRU + 钉选 + 512MB 预算） |
| M4：异步管线 | 4 | ✅ 已完成 | `StreamingManager`（Channels + IO Workers + 优先级排序） |
| M5：Pack 管理 | 5 | ✅ 已完成 | `PackManager` + `PackMount`（MemoryMappedFile 零拷贝） |
| M6：核心管理器 | 6 | ✅ 已完成 | `AssetManager` 完整功能（sync/async load, tick, hot reload, blob access） |
| M7：薄包装重写 | 7 | ✅ 已完成 | AssetHandle / AssetDatabase / Addressables / DependencyTracking / ImportPipeline 全部改为 C# |
| M8：ABI 废弃 | 8 | ✅ 已完成 | C#/C++ Asset ABI 全部删除；LayoutVersion 24；TextureInterop 改用 C# AssetManager |

**全部完成**：NNRuntimeAsset C# 迁移 Phase 0-8 已全部实施完毕。
