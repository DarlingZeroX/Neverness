# NNRuntimeRenderAssets C# 迁移计划

## 概述

将 C++ `NNRuntimeRenderAssets` 模块复刻到 C#，放在 `Engine/Source/Managed/Rendering/Neverness.Rendering.RenderAssets`。

## 需求

- 纯 C# 实现，不与 C++ 端 ABI 兼容
- 通过 `NNDiligentAPI.GetPrimaryDevice()` 获取 `void*`，使用 `Neverness.Rendering.Diligent.GraphicsDevice`
- 完整 LRU 缓存（驱逐策略、内存限制）

## C++ 模块结构分析

```
NNRuntimeRenderAssets/
├── NNRenderAssetManager      # 单例，CPU Asset → GPU Resource 管理
├── NNTextureResource          # GPU 纹理资源封装（void* RHI Texture + SRV）
├── NNTextureSourceAsset       # CPU 侧纹理源资产（mip 链）
├── NNTextureCache             # LRU 纹理缓存
├── NNTextureFormat            # 纹理格式枚举
├── IRenderResourceFactory     # 渲染资源工厂接口
└── DiligentRenderResourceFactory  # Diligent 实现
```

## C# 端架构

```
Neverness.Rendering.RenderAssets/
├── RenderAssetManager.cs          # 单例，管理 CPU→GPU 生命周期
├── TextureResource.cs             # GPU 纹理资源（持有 TextureHandle + TextureView）
├── TextureSourceAsset.cs          # CPU 侧纹理源（mip 链）
├── TextureCache.cs                # LRU 缓存
├── TextureFormat.cs               # 纹理格式枚举
├── IRenderResourceFactory.cs      # 渲染资源工厂接口
└── DiligentRenderResourceFactory.cs  # Diligent 实现
```

## 依赖关系

```
Neverness.Rendering.RenderAssets
  ├─ Neverness.Rendering.Diligent   # GraphicsDevice, TextureHandle, TextureView
  └─ Neverness.Runtime.Engine       # NNDiligentAPI (获取主窗口设备指针)
```

## 文件清单

### 1. `TextureFormat.cs`

对应 `NNTextureFormat.h`：

```csharp
public enum TextureFormat : uint
{
    Unknown = 0,
    R8_UNorm, RG8_UNorm, RGB8_UNorm, RGBA8_UNorm,
    R16_Float, RG16_Float, RGB16_Float, RGBA16_Float,
    R32_Float, RG32_Float, RGB32_Float, RGBA32_Float,
    BC1_UNorm, BC3_UNorm, BC7_UNorm,
    ETC2_RGB8, ETC2_RGBA8, ASTC_4x4,
    Count
}
```

### 2. `TextureSourceAsset.cs`

对应 `NNTextureSourceAsset.h`：
- `MipLevel` 结构体（Width, Height, Pixels）
- `SetFromDecodedImage()` 初始化
- `Serialize()` / `Deserialize()` 二进制序列化

### 3. `TextureResource.cs`

对应 `NNTextureResource.h`：
- 持有 `TextureHandle` + `TextureView`（SRV）
- `GetImGuiHandle()` → `TextureView.NativeObject` 指针
- `Residency` 状态

### 4. `TextureCache.cs`

对应 `NNTextureCache.h`：
- LRU 驱逐策略
- `Dictionary<ulong, Entry>` + `LinkedList<ulong>` 实现
- `Insert()`, `Get()`, `Remove()`, `EvictToTarget()`

### 5. `IRenderResourceFactory.cs`

```csharp
public interface IRenderResourceFactory
{
    TextureResource CreateTexture(uint width, uint height, TextureFormat format,
                                  ReadOnlySpan<byte> pixels, bool isSRGB);
    bool UpdateTexturePixels(TextureResource resource, ReadOnlySpan<byte> pixels);
}
```

### 6. `DiligentRenderResourceFactory.cs`

- 通过 `GraphicsDevice.CreateTexture()` 创建 Diligent 纹理
- 获取 `TextureViewType.ShaderResourceView` 作为 SRV
- 格式映射：`TextureFormat` → `Diligent.TextureFormat`

### 7. `RenderAssetManager.cs`

对应 `NNRenderAssetManager.h`：
- 单例模式
- `Initialize(GraphicsDevice device)` - 创建 DiligentRenderResourceFactory
- `CreateTextureFromSource()`, `CreateTextureFromPixels()`, `LoadTextureFromAsset()`
- `GetTextureResource()`, `ReleaseTexture()`, `ReloadTexture()`
- `GetImGuiTextureHandle()`, `UpdateTexturePixels()`
- LRU 驱逐：`EvictLRU()`, `SetCurrentFrame()`
- GUID 索引：`GetCacheKeyByGuidLow()`

## 与 C++ 端的差异

| 方面 | C++ 端 | C# 端 |
|------|--------|-------|
| 设备获取 | WindowRegistry → VGWindow → NNDiligentDevice | NNDiligentAPI.GetPrimaryDevice() → GraphicsDevice |
| 纹理句柄 | void* (ITextureView*) | TextureView 对象 |
| 内存管理 | shared_ptr<void> + deleter | IDisposable + GC |
| 线程安全 | std::mutex | lock 语句 |
| 序列化 | memcpy | BinaryReader/Writer |

## 实施顺序

1. `TextureFormat.cs` - 格式枚举
2. `TextureSourceAsset.cs` - CPU 侧源资产
3. `TextureResource.cs` - GPU 纹理资源
4. `TextureCache.cs` - LRU 缓存
5. `IRenderResourceFactory.cs` - 工厂接口
6. `DiligentRenderResourceFactory.cs` - Diligent 实现
7. `RenderAssetManager.cs` - 管理器单例
8. 项目文件 `Neverness.Rendering.RenderAssets.csproj`
9. 编译验证

## 风险与注意事项

1. **Diligent 绑定依赖**：需要引用 `Diligent.GraphicsEngine.NET` NuGet 包
2. **设备指针转换**：`void*` → `IRenderDevice` 需要通过 `DiligentBackend` 内部机制
3. **线程安全**：C# 的 `lock` 比 C++ 的 `std::mutex` 更简洁，但需注意死锁
4. **GC 压力**：大量小对象（MipLevel）可能增加 GC 负担，考虑使用 ArrayPool
