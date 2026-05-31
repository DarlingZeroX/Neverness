# Texture 加载失败：跨模块单例 + 格式版本缓存问题

## 问题现象

打开纹理资产时 GPU 上传失败：

```
[RenderAssetManager] LoadTextureFromAsset: handle=1 缺少 TypeInfo blob
[TextureAssetOpener] GPU 上传失败: /assets/屏幕截图1.png
```

## 根因分析

本问题由两个独立缺陷叠加导致：

### 缺陷 1：.nnasset 格式版本未递增（缓存过期）

**触发条件：** TextureImporter 新增了 TypeInfo blob（type=9），但 `NN_ASSET_VERSION` 仍为 1。

**故障链：**

1. 旧版 TextureImporter 导入纹理 → 生成 `.nnasset`（仅有 DATA + MIP_LEVEL blobs，无 TypeInfo）
2. 更新代码后，TextureImporter 现在写入 TypeInfo blob
3. `ImportPipeline` 的 `HasChanged` 检测发现源 PNG 未修改 → 跳过重新导入
4. 旧版 `.nnasset` 被加载，缺少 TypeInfo blob → GPU 上传失败

**核心问题：** `HasChanged` 仅检查源文件时间戳，不检查已导入资产的格式版本。

### 缺陷 2：NNAssetManager 跨模块双单例（根因）

即使修复了版本缓存问题，重新导入生成了正确的 `.nnasset`，`LoadTextureFromAsset` 仍然失败。

**触发条件：** `NNRuntimeAsset` 以静态库形式链接到多个模块（可执行文件 + `NNRuntimeRenderer2D` DLL）。

**故障链：**

1. Windows 静态库中的 `static` 变量会被复制到每个链接它的模块中
2. `NNAssetManager::Instance()` 使用函数内静态变量实现单例：
   ```cpp
   NNAssetManager& NNAssetManager::Instance() noexcept {
       static NNAssetManager instance;
       return instance;
   }
   ```
3. C# 调用 `LoadSync` → 经 C API（`NNRuntimeAsset` 模块的单例）→ 成功分配 handle=1，存入 `handleToEntry_`
4. `LoadTextureFromAsset`（在 `NNRuntimeRenderer2D` 模块中）调用 `NNAssetManager::Instance()` → 获取了**另一个**单例实例 → `handleToEntry_` 为空 → 找不到 TypeInfo blob

**证据（`this` 指针不同）：**

```
[AssetManager] LoadAssetInternal: 插入后验证 this=00007FF81599F600 ...
[AssetManager] GetBlobCount: this=00007FF81660CCF0 handle=1 NOT FOUND ...
```

两个不同的 `NNAssetManager` 实例，各自维护独立的 `handleToEntry_`。

## 解决方案

### 1. 格式版本递增 + 过期资产检测

- `NNAssetFormat.h`：`NN_ASSET_VERSION` 从 1 升至 2
- `ImportPipeline.cs`：写入版本号同步更新为 2
- `ImportPipeline.cs`：新增 `IsAssetFormatCurrent()` 方法，在 `HasChanged` 跳过逻辑前检查已有 `.nnasset` 的版本号，版本不匹配时强制重新导入

### 2. 新增 LoadTextureFromBlob API（绕过跨模块单例）

**设计思路：** 不在 `LoadTextureFromAsset` 内部通过单例查询 blob 数据，改为由调用方（C# 侧）通过 C API 获取 blob 指针后直接传入。

**新增函数签名：**

```c
// C API (RenderAssetAPI.h)
uint64_t loadTextureFromBlob(
    const void* typeInfoData, uint64_t typeInfoSize,
    const void* pixelData, uint64_t pixelDataSize,
    uint64_t guidLow);
```

**调用链变更：**

```
旧流程：
  C# → LoadTextureFromAsset(handle, guid)
     → [NNRuntimeRenderer2D] NNAssetManager::Instance().GetBlobByType(handle)  ← 错误单例
     → GPU Upload

新流程：
  C# → AssetManager API: getBlobCount(handle) / getBlobData(handle, i)  ← 正确单例（同模块）
     → 找到 TypeInfo (size==24) 和 DATA blob
     → LoadTextureFromBlob(typeInfoPtr, typeInfoSize, pixelPtr, pixelSize, guid)
     → [NNRuntimeRenderer2D] 直接使用传入的数据 → GPU Upload
```

### 3. 清理诊断日志

- 移除了 `LoadTextureFromAsset` 中的 hex dump 和冗余日志
- 保留 `LoadTextureFromAsset` 实现以备 C++ 内部使用
- 新增的 `LoadTextureFromBlob` 包含合理性校验（宽高、format）

## 涉及文件

| 文件 | 变更 |
|------|------|
| `NNRuntimeAsset/Include/NNAssetFormat.h` | 版本 1→2 |
| `Editor/Neverness.Editor.Assets/ImportPipeline.cs` | 版本写入 + `IsAssetFormatCurrent` 检测 |
| `NNNativeEngineAPI/Include/RenderAssetAPI.h` | 新增 `loadTextureFromBlob` 函数指针 |
| `NNRuntimeRenderAssets/Include/NNRenderAssetManager.h` | 新增 `LoadTextureFromBlob` 声明 |
| `NNRuntimeRenderAssets/NNRenderAssetManager.cpp` | 新增 `LoadTextureFromBlob` 实现 |
| `NNRuntimeEngineServices/RenderAssetRuntimeApi.cpp` | C API 桥接 + 接线 |
| `NNRuntimeNativeEngineAPIStub/RenderAssetApiStubs.cpp` | Stub 实现 |
| `Neverness.Runtime.Engine/NNNativeEngineApiTypes.cs` | C# API 结构体新增字段 |
| `Neverness.Runtime.Engine/TextureInterop.cs` | 新增 `LoadTextureFromBlob` 封装 |
| `Editor/Neverness.Editor.Assets/TextureAssetOpener.cs` | 改用 `LoadTextureFromBlob` |
| `Editor/Neverness.Editor.Scene/SpriteRendererInspector.cs` | 同上 |

## 经验教训

1. **静态库单例陷阱：** Windows 上静态库中的 `static` 变量会被复制到每个链接模块。跨模块访问单例必须通过共享库（DLL）导出，或通过同模块内的 C API 间接访问。
2. **资产格式版本管理：** 修改二进制格式（新增/删除/改布局 blob）时，必须同时：
   - 递增格式版本号（C++ 和 C# 同步）
   - 在导入管线中添加版本检测，旧版本强制重新导入
3. **Handle 跨模块无效：** 资产 handle 仅在创建它的单例实例内有效。跨模块传递 handle 无法查询资产数据。
