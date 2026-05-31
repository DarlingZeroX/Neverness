# TextureAsset 持久化标识 — 两层架构

## 问题

`NNSpriteRendererComponent.TextureAsset` 之前存储了各种临时值（GL texture ID、cache key），导致：

- 序列化后反序列化值失效（GL ID / cache key 每次会话不同）
- Inspector 显示"未加载"（cache key → GL ID 解析失败）
- Renderer 绑定错误纹理（`glBindTexture(GL_TEXTURE_2D, cacheKey)`）

## 架构设计

Scene Component 存**资产引用**，Renderer 吃**已解析的 GPU 资源 ID**，分两层：

```
┌─────────────────────────────────────────────┐
│ Scene Component (Authoring/Serialization)   │
│  TextureAsset = GUID.Low (持久化标识)         │
│  TextureRuntimeId = GL texture ID (瞬态)     │
└──────────────────┬──────────────────────────┘
                   │ SpriteRenderSystem 懒解析
                   ▼
┌─────────────────────────────────────────────┐
│ Renderer (GPU Hot Path)                     │
│  cmd.TextureHandle = GL texture ID          │
│  → glBindTexture(GL_TEXTURE_2D, handle)     │
│  → 零开销，无 lookup                         │
└─────────────────────────────────────────────┘
```

## 数据流

### Inspector 拖拽

```
ContentBrowser 拖拽 → payload = GUID.High + GUID.Low
Inspector 接收 → data.TextureAsset = droppedGuid.Low
               → LoadTextureFromGuidLow(guidLow) → cacheKey → 预览
```

### 渲染（Collect 首帧懒解析）

```
SpriteRenderSystem::Collect()
  if TextureRuntimeId == 0 && TextureAsset != 0:
    GetCacheKeyByGuidLow(TextureAsset)  // O(1) 查询
    if not cached:
      LoadAssetByGuidLow(guidLow) → handle
      LoadTextureFromAsset(handle, guidLow) → cacheKey + 注册映射
    GetGLTextureId(cacheKey) → GL texture ID
    TextureRuntimeId = GL ID

  cmd.TextureHandle = TextureRuntimeId  // 已解析，直接使用
```

### 渲染（后续帧）

```
Collect() → TextureRuntimeId != 0 → 直接赋值 cmd.TextureHandle
Renderer → glBindTexture(GL_TEXTURE_2D, TextureHandle)
```

### 序列化/反序列化

```
序列化: TextureAsset (= GUID.Low) → .nnscene 文件
反序列化: 读取 GUID.Low → TextureAsset
         TextureRuntimeId = 0 (未解析)
首帧渲染: Collect() 懒解析 → TextureRuntimeId = GL ID
```

## 修改清单

### C++ 侧

| 文件 | 变更 |
|------|------|
| `NNAssetManager.h/.cpp` | 新增 `LoadAssetByGuidLow(guidLow, typeId)` |
| `NNRenderAssetManager.h/.cpp` | `LoadTextureFromAsset` 增加 `guidLow` 参数；新增 `m_GuidToCacheKeyMap` 索引；新增 `GetCacheKeyByGuidLow()` 和 `GetGLTextureId()` |
| `RenderAssetAPI.h` | `loadTextureFromAsset` 增加 `guidLow` 参数 |
| `NNSpriteRendererComponent.h` | 新增 `TextureRuntimeId`（uint32_t，不序列化）；`TextureAsset` 注释改为"GUID.Low" |
| `SpriteRenderSystem.cpp` | Collect 中实现懒解析：GUID.Low → cacheKey → GL ID → TextureRuntimeId |
| `Renderer2D.h/.cpp` | 删除 `TextureResolveFunc` 和 `SetTextureResolver`；Submit 直接使用 `cmd.TextureHandle` 作为 GL ID |
| `SceneRenderer.h/.cpp` | 删除 `SetTextureResolver` |
| `ViewportRenderRuntimeApi.cpp` | 删除 `ResolveTextureCacheKey` 和 resolver 注册 |
| `RenderAssetRuntimeApi.cpp` | `rt_renderAsset_loadTextureFromAsset` 增加 `guidLow` 参数 |
| `CMakeLists.txt` (Renderer2D) | 链接 `NevernessRuntime-RenderAssets` + `NevernessRuntime-Asset` |

### C# 侧

| 文件 | 变更 |
|------|------|
| `NNNativeEngineApiTypes.cs` | `LoadTextureFromAsset` 委托增加 `guidLow` 参数 |
| `TextureInterop.cs` | `LoadTextureFromAsset` 增加 `guidLow` 参数；新增 `LoadTextureFromGuidLow(guidLow)` |
| `SpriteRendererInspector.cs` | 拖拽存储 `GUID.Low`；预览通过 `LoadTextureFromGuidLow` + `GetImGuiTextureHandle` |

## 设计原则

- **Scene ≠ Renderer**：Component 存资产引用（Authoring Data），Renderer 吃 GPU 句柄
- **懒解析**：首次访问时解析，后续帧零开销（TextureRuntimeId 已缓存）
- **O(1) 索引**：`m_GuidToCacheKeyMap` 避免每帧重复加载
- **可扩展**：未来可演进为 RenderProxy 模式（SpriteRenderSystem → NNSpriteRenderProxy → Renderer）
