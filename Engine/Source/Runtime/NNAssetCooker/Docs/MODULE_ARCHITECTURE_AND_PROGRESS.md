# NNAssetCooker — 资产编译/打包编排器

> CMake 目标 **`NevernessRuntime-AssetCooker`**（STATIC）；C++ 命名空间 **`NN::Runtime::Asset`**。

## 1. 定位与边界

| 项目 | 说明 |
|------|------|
| **职责** | 将 Editor 导入的 `.nnasset` 文件按构建清单（`NNCookManifest`）编译、分组，通过 `NNPackBuilder` 输出 **`.nnpack`** 二进制包。提供 C ABI 桥接供 C# Editor 调用。 |
| **不负责** | 资产导入（`NNAssetImporter`）、运行时 `.nnpack` 加载/挂载（`NNAssetLoader`）、资产压缩/加密（格式预留 flag，尚未实现）。 |
| **CMake 目标** | `NevernessRuntime-AssetCooker`（`STATIC`） |
| **命名空间** | `NN::Runtime::Asset` |
| **依赖** | `NevernessRuntime-NativeEngineAPI`（PUBLIC：`AssetCookerAPI.h`、`EngineTypes.h`）；`NevernessRuntime-Asset`（PUBLIC：`NNPackFormat.h`）。 |
| **典型消费者** | `NNRuntimeEngineServices`（链接并调用 `NNBuildAssetCookerRuntimeApi`）；C# `AssetCooker.cs`（通过 C ABI 函数指针）；C# `PackageBuilder.cs`（纯 C# 独立实现同一格式）。 |

### 1.1 在资产管线中的位置

```
Editor AssetDatabase
    │
    ▼
AssetCooker.cs (C#)          ← Editor 触发构建
    │
    ▼  C ABI (NNAssetCookerAPI)
NNAssetCooker::Cook()         ← 本模块：读清单 → 读 .nnasset → NNPackBuilder
    │
    ▼
*.nnpack 二进制包             ← 运行时由 NNAssetLoader 加载
```

---

## 2. 构建与选项

- 在 [`Engine/Source/Runtime/CMakeLists.txt`](../CMakeLists.txt) 中注册 `add_subdirectory("NNAssetCooker")`。
- **PUBLIC** 包含目录：`NNAssetCooker/Include`。
- **PRIVATE** 包含目录：`Engine/Source/Runtime`、`Engine/Source/Core`。
- 无测试目标（当前无 GTest）。

```powershell
cmake --build Build/vs/x64-debug --target NevernessRuntime-AssetCooker
```

---

## 3. 目录结构

```
Engine/Source/Runtime/NNAssetCooker/
├── CMakeLists.txt
├── Include/
│   ├── NNAssetCooker.h          # 编排器类 + NNCookResult + 进度回调
│   ├── NNCookManifest.h         # 构建清单（资产条目 + 分组 + 路径）
│   └── NNPackBuilder.h          # .nnpack 二进制文件构建器
├── Private/
│   ├── NNAssetCooker.cpp        # Cook/CookGroup 实现
│   ├── NNCookManifest.cpp       # Add/Set/Clear 实现
│   ├── NNPackBuilder.cpp        # .nnpack 二进制写入全流程
│   └── NNAssetCookerApi.cpp     # C ABI 桥接（函数指针表填充 + handle 管理）
└── Docs/
    └── MODULE_ARCHITECTURE_AND_PROGRESS.md
```

---

## 4. 核心类型

### 4.1 NNCookAssetEntry（资产条目）

```cpp
struct NNCookAssetEntry
{
    NNGuid         guid{};              // 资产 GUID（128-bit）
    std::uint64_t  typeId{0};           // 资产类型 ID（Texture2D=1, Mesh=2, ...）
    std::string    sourcePath;          // .nnasset 源文件路径
    std::uint32_t  groupIndex{0};       // 所属构建分组索引
    bool           includeInBuild{true}; // 是否参与构建
};
```

### 4.2 NNCookGroup（构建分组）

```cpp
struct NNCookGroup
{
    std::string    name;        // 分组名称（成为 .nnpack 文件名）
    std::string    address;     // Addressable 地址
    std::uint32_t  strategy{0}; // 打包策略：0=PackTogether, 1=PackSeparately
    std::string    outputPath;  // 显式 .nnpack 输出路径（覆盖默认）
};
```

### 4.3 NNCookManifest（构建清单）

Editor 构建管线生成的"构建什么"规格说明，传递给 `NNAssetCooker::Cook()`。

| 方法 | 说明 |
|------|------|
| `AddAsset(entry)` | 添加待编译资产 |
| `AddGroup(group)` | 添加构建分组 |
| `SetOutputRoot(path)` | 设置输出根目录 |
| `SetLibraryRoot(path)` | 设置 Library 根目录（`Library/Imported/`） |
| `GetAssets()` / `GetGroups()` | const 访问器 |
| `GetOutputRoot()` / `GetLibraryRoot()` | 路径访问器 |
| `GetAssetCount()` / `GetGroupCount()` | 计数 |
| `Clear()` | 重置全部状态 |

### 4.4 NNCookResult（构建结果）

```cpp
struct NNCookResult
{
    bool          success{false};        // 全部成功时为 true（failedAssets==0）
    std::uint32_t totalAssets{0};        // 清单中资产总数
    std::uint32_t cookedAssets{0};       // 成功编译数
    std::uint32_t failedAssets{0};       // 失败数
    std::uint32_t generatedPacks{0};     // 生成的 .nnpack 数
    std::string   errorMessage;          // 错误描述
    double        elapsedSeconds{0.0};   // 墙钟耗时
};
```

### 4.5 NNCookProgressCallback（进度回调）

```cpp
using NNCookProgressCallback = std::function<void(
    std::uint32_t current,   // 当前进度
    std::uint32_t total,     // 总数
    const char*   phase      // 阶段描述（"Reading", "Packing", ...）
)>;
```

---

## 5. 核心 API

### 5.1 NNAssetCooker（编排器）

主入口，**无状态**（无成员变量），线程安全（不同实例可并发 Cook）。

| 方法 | 说明 |
|------|------|
| `Cook(manifest, progress)` | 执行完整构建流程。无分组时输出 `default.nnpack`；有分组时逐组调用 `CookGroup`。返回 `NNCookResult`。 |
| `CookGroup(group, manifest, progress, groupIndex, totalGroups)` | 单分组打包：筛选 groupIndex 匹配的资产 → 读取 .nnasset 数据 → NNPackBuilder::Build()。 |

**Cook 流程：**

```
Cook(manifest)
├── 无分组？
│   ├── 是 → 所有资产 → NNPackBuilder → default.nnpack
│   └── 否 → 遍历每个 Group
│       └── CookGroup(group)
│           ├── 筛选 groupIndex 匹配的资产
│           ├── 解析源路径（Library root + 相对路径）
│           ├── 读取 .nnasset 二进制数据
│           ├── NNPackBuilder.AddAsset(guid, typeId, data)
│           └── NNPackBuilder.Build() → <group.name>.nnpack
├── 统计 cookedAssets / failedAssets / generatedPacks
└── 返回 NNCookResult（含 elapsedSeconds）
```

### 5.2 NNPackBuilder（包构建器）

低级 `.nnpack` 二进制文件写入器。

| 方法 | 说明 |
|------|------|
| `SetOutputPath(path)` | 设置输出文件路径 |
| `AddAsset(guid, typeId, data)` | 入队一个资产（原始字节） |
| `SetPackageName(name)` | 包名（写入 Manifest 段） |
| `Build()` | 写入完整 `.nnpack` 文件，返回成功/失败 |
| `GetAssetCount()` | 队列中资产数 |
| `Clear()` | 重置队列 |

---

## 6. .nnpack 二进制格式

格式定义于 `NNRuntimeAsset/Include/NNPackFormat.h`，当前**版本 1**。

### 6.1 文件布局

```
┌─────────────────────────────────────────┐
│ NNPackHeader（64 字节）                  │  ← magic='NNPK', version=1
├─────────────────────────────────────────┤
│ Asset Table（N × 条目）                  │  ← 64 字节对齐
├─────────────────────────────────────────┤
│ Manifest（UTF-8 包名）                   │  ← 64 字节对齐
├─────────────────────────────────────────┤
│ Asset Data（各 .nnasset 原始字节）        │  ← 每资产 64 字节对齐
└─────────────────────────────────────────┘
```

### 6.2 NNPackHeader（64 字节）

| 偏移 | 字段 | 类型 | 说明 |
|------|------|------|------|
| 0 | `magic` | `uint32` | `0x4E4E504B`（`'NNPK'`） |
| 4 | `version` | `uint32` | 格式版本，当前 `1` |
| 8 | `assetCount` | `uint32` | 资产数量 |
| 12 | `flags` | `uint32` | 标志位：`COMPRESSED=0x01`, `ENCRYPTED=0x02`, `STREAMING=0x04` |
| 16 | `tableOffset` | `uint64` | Asset Table 偏移（= 64） |
| 24 | `tableSize` | `uint64` | Asset Table 总字节数 |
| 32 | `manifestOffset` | `uint64` | Manifest 段偏移 |
| 40 | `manifestSize` | `uint64` | Manifest 段字节数 |
| 48 | `dataOffset` | `uint64` | Asset Data 段偏移 |
| 56 | `totalDataSize` | `uint64` | Asset Data 总字节数 |

### 6.3 NNPackAssetEntry（每资产条目）

| 字段 | 类型 | 说明 |
|------|------|------|
| `guidHigh` | `uint64` | GUID 高 64 位 |
| `guidLow` | `uint64` | GUID 低 64 位 |
| `typeId` | `uint64` | 资产类型 ID |
| `offset` | `uint64` | 在 Asset Data 段中的字节偏移 |
| `size` | `uint64` | 原始大小 |
| `compressedSize` | `uint64` | 压缩后大小（0=未压缩） |
| `flags` | `uint32` | 单资产标志位 |
| `_pad` | `uint32` | 对齐填充 |

### 6.4 对齐规则

所有段起始偏移和每个资产数据块均通过 `NNPackAlign()` 对齐到 **64 字节**边界。不足部分零填充。

---

## 7. C ABI 层

### 7.1 NNAssetCookerAPI（函数指针表）

定义于 `NNNativeEngineAPI/Include/AssetCookerAPI.h`，嵌入 `NNNativeEngineAPI.assetCooker` 字段（layoutVersion 17 起）。

```c
typedef struct NNAssetCookerAPI {
    uint64_t (NN_ENGINE_ABI_STDCALL *createManifest)(void);
    void     (NN_ENGINE_ABI_STDCALL *destroyManifest)(uint64_t manifestHandle);
    void     (NN_ENGINE_ABI_STDCALL *setOutputRoot)(uint64_t manifestHandle, const char* pathUtf8);
    void     (NN_ENGINE_ABI_STDCALL *setLibraryRoot)(uint64_t manifestHandle, const char* pathUtf8);
    void     (NN_ENGINE_ABI_STDCALL *addAsset)(uint64_t manifestHandle, NNGuid guid,
                 uint64_t typeId, const char* sourcePathUtf8, uint32_t groupIndex);
    void     (NN_ENGINE_ABI_STDCALL *addGroup)(uint64_t manifestHandle, const char* nameUtf8,
                 const char* addressUtf8, uint32_t strategy, const char* outputPathUtf8);
    NNCookResultData (NN_ENGINE_ABI_STDCALL *cook)(uint64_t manifestHandle);
} NNAssetCookerAPI;
```

### 7.2 NNCookResultData（POD 结果）

```c
typedef struct NNCookResultData {
    int      success;
    uint32_t totalAssets;
    uint32_t cookedAssets;
    uint32_t failedAssets;
    uint32_t generatedPacks;
    double   elapsedSeconds;
} NNCookResultData;
```

### 7.3 Handle 管理

`NNAssetCookerApi.cpp` 维护全局 manifest 存储：

```cpp
static std::unordered_map<uint64_t, std::unique_ptr<NNCookManifest>> g_manifests;
static std::mutex g_manifestMutex;
static uint64_t g_nextHandle{1};
```

- `createManifest()` → 分配单调递增 handle（从 1 开始）+ mutex 保护
- `cook(handle)` → 查表取 manifest → 创建临时 `NNAssetCooker` → `Cook()` → 映射为 `NNCookResultData`
- `destroyManifest(handle)` → 从 map 移除，unique_ptr 自动释放

### 7.4 Stub 实现

`NNRuntimeNativeEngineAPIStub/Private/AssetCooker/AssetCookerApiStubs.cpp` 提供 `NNBuildAssetCookerApiStubs()`，将所有函数指针填充为 no-op。未加载真实模块时使用。

---

## 8. 接线链路

### 8.1 Native 侧

```
EngineAPIRegistry.h
    └── NNNativeEngineAPI.assetCooker（NNAssetCookerAPI）
            │
            ├── 真实路径：
            │   NNRuntimeEngineServices/NativeEngineRuntimeApiTable.cpp
            │       └── NNBuildAssetCookerRuntimeApi(&api->assetCooker)
            │               └── NNAssetCookerApi.cpp 填充 7 个函数指针
            │
            └── Stub 路径：
                NNRuntimeNativeEngineAPIStub/AssetCookerApiStubs.cpp
                    └── NNBuildAssetCookerApiStubs(&api->assetCooker)
                            └── 全部 no-op
```

### 8.2 C# 侧

```
AssetCooker.cs（Neverness.Editor.Assets）
    ├── Cook(BuildProfile profile)
    │   ├── 创建 manifest（createManifest）
    │   ├── 设置路径（setOutputRoot / setLibraryRoot）
    │   ├── 遍历 EditorAssetDatabase → addAsset
    │   ├── 遍历 BuildProfile.GroupManager → addGroup
    │   └── 调用 cook → 返回 CookResult
    │
    ├── CookerApiTable：delegate* unmanaged[Stdcall] 函数指针
    └── NativeCookResultData：[StructLayout(Sequential)] 镜像 C 结构体

PackageBuilder.cs（Neverness.Editor.Assets）
    └── 纯 C# 实现 .nnpack 格式（不依赖 C++ 模块）
        ├── BuildFromDirectory()
        ├── BuildFromGroup()
        └── Build()：完全镜像 NNPackBuilder::Build() 布局
```

---

## 9. 依赖关系

```
NNAssetCooker 依赖：
├── NevernessRuntime-NativeEngineAPI（PUBLIC）
│   ├── AssetCookerAPI.h    — ABI 定义
│   └── EngineTypes.h       — NNGuid 类型
└── NevernessRuntime-Asset（PUBLIC）
    └── NNPackFormat.h      — 格式常量 + 结构体

依赖 NNAssetCooker 的模块：
├── NNRuntimeEngineServices          — 链接 + 调用 NNBuildAssetCookerRuntimeApi()
├── NNRuntimeNativeEngineAPIStub     — 提供 no-op stub
├── EngineAPIRegistry.h              — 嵌入 assetCooker 字段
└── C# AssetCooker.cs / PackageBuilder.cs — Editor 构建消费
```

---

## 10. 关键设计决策

| 决策 | 理由 |
|------|------|
| **Handle-based ABI**（`uint64_t` 句柄而非指针） | C#/C++ 跨 ABI 安全，不受 GC 压缩影响 |
| **STATIC 库**（非 SHARED） | 链入 `NNRuntimeEngineServices`，无需独立 DLL |
| **NNAssetCooker 无状态** | 并发安全：不同线程可同时 Cook 不同清单 |
| **64 字节对齐** | 适配 SIMD/GPU 读取、SSD 页对齐、mmap 友好 |
| **格式预留 COMPRESSED/ENCRYPTED flag** | 前向兼容，后续实现无需改格式版本 |
| **纯 C# PackageBuilder 备份路径** | C++ 模块未加载时仍可构建（Editor 降级路径） |
| **Library root 路径解析** | 支持 `Library/Imported/XX/` hex 前缀约定 |

---

## 11. 开发进展

| 日期 | 进展 |
|------|------|
| 2026-05-23 | **Phase 1** 合入：`NevernessRuntime-AssetCooker` STATIC 库创建、`NNAssetCooker`/`NNCookManifest`/`NNPackBuilder` 三件套、C ABI 桥接（7 个函数指针 + handle 管理）、`NNNativeEngineAPI` layoutVersion 17 新增 `assetCooker` 字段、Stub 实现。 |
| 2026-05-23 | **C# 侧** 合入：`AssetCooker.cs`（C ABI 封装）+ `PackageBuilder.cs`（纯 C# .nnpack 写入）+ `NNCookResultData`/`NNAssetCookerApi` blittable 类型。 |

---

## 12. 未完成项

- **压缩支持**：`NNPackBuilder::Build()` 当前始终 `compressedSize=0`（未压缩），需接入 zstd/lz4。
- **加密支持**：`flags` 始终为 0，`NN_PACK_FLAG_ENCRYPTED` 预留但未实现。
- **增量构建**：当前为全量重建，未做资产变更检测。
- **并行打包**：`CookGroup` 串行执行，未利用多核并行。
- **C# AssetCooker.cs**：`GetCookerApi()` 当前返回空表（TODO：从 `NativeApiProvider` 接线）。
- **Streaming 包**：`NN_PACK_FLAG_STREAMING` 预留，未实现流式加载分包。
- **GTest 测试**：无单元测试覆盖。

---

## 13. 未来规划

1. **压缩集成**：`NNPackBuilder` 支持 zstd 压缩，`compressedSize` 字段启用，header flags 设置 `COMPRESSED`。
2. **增量构建**：基于 `.nnasset` 文件 hash + 上次构建缓存，仅重编变更资产。
3. **并行 CookGroup**：线程池并行打包多个 group，进度回调合并。
4. **Streaming 包**：按 LOD / 距离阈值拆分 streaming pack，运行时按需挂载。
5. **加密支持**：AES-256 对称加密，密钥由运行时注入。
6. **构建报告**：详细的 per-asset 构建日志 + XML/JSON 报告输出。

---

## 14. 相关文档

- [资产管线总架构](../../../Docs/1.Neverness%20Engine%20Industrial-grade%20Modern%20Asset%20Runtime%20and%20Asset%20Pipeline%20Architecture.md) — §20 定义 .nnpack 格式与包挂载，§6.3 列出 NNAssetCooker 模块定位
- [纹理资产管线](../../../Docs/5.Texture-Asset-Pipeline-Architecture.md) — 纹理 pak 构建集成
- [音视频资产管线](../../../Docs/15.Audio_Video_Asset_Pipeline_Architecture.md) — §5.4 媒体资产 pak 集成
- `NNPackFormat.h`（`NNRuntimeAsset/Include/`）— .nnpack 格式定义
- `AssetCookerAPI.h`（`NNNativeEngineAPI/Include/`）— C ABI 函数指针表定义
