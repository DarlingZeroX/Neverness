# Neverness.Rendering.Core — API 参考手册

> 自动生成于 2026-06-26，基于源码 6 个文件。

---

## 目录

1. [渲染调度器：RenderingLoop](#1-renderingloop)
2. [视口表面注册表接口：IViewportSurfaceRegistry](#2-iviewportsurfaceregistry)
3. [视口表面注册表实现：ViewportSurfaceRegistryImpl](#3-viewportsurfaceregistryimpl)
4. [视口服务接口：IViewportService](#4-iviewportservice)
5. [视口服务实现：ViewportServiceImpl](#5-viewportserviceimpl)

---

## 1. RenderingLoop

**文件**: `RenderingLoop.cs` · **命名空间**: `Neverness.Rendering.Core`

主线程渲染调度器。管理渲染回调的注册和执行。纯静态类，线程安全。

### 设计原则

- 纯静态类，无状态依赖
- 线程安全：回调列表使用 lock 保护
- 异常隔离：单个回调异常不影响其他回调
- 跨模块共享：Editor 和 Runtime 都可使用

### 静态属性

| 属性 | 类型 | 说明 |
|------|------|------|
| `CallbackCount` | `int` | 已注册回调数量 |

### 静态方法

| 方法 | 说明 |
|------|------|
| `RegisterRenderCallback(Action callback)` | 注册主线程渲染回调。重复注册同一回调会被忽略。 |
| `UnregisterRenderCallback(Action callback)` | 注销主线程渲染回调。 |
| `TickRendering()` | 执行所有已注册的渲染回调。必须在主线程调用。异常处理：单个回调异常不会影响其他回调执行。 |
| `ClearAll()` | 清除所有已注册的渲染回调。通常在 Shutdown 时调用。 |

### 使用示例

```csharp
// 注册回调
RenderingLoop.RegisterRenderCallback(() =>
{
    // 渲染逻辑
    surfaceRegistry.FlushResizes();
    var commands = viewportService.CollectRenderCommands(w, h);
    if (commands != null)
        surfaceRegistry.RenderViewportCommands(surfaceId, commands);
});

// 主循环
while (running)
{
    SDL_PollEvent(...);
    RenderingLoop.TickRendering();
    SwapChain.Present();
}
```

---

## 2. IViewportSurfaceRegistry

**文件**: `IViewportSurfaceRegistry.cs` · **命名空间**: `Neverness.Rendering.Core`

视口表面注册表接口。管理多个 ViewportSurface 的生命周期。

### 设计原则

- 每个 Surface 持有独立 SwapChain
- Device/Context 全局共享，由 Renderer 管理
- Deferred Resize：MarkResize → FlushResizes（帧末统一执行）
- Surface Lost：HandleDestroyed → MarkLost → HandleCreated → Recreate

### 使用场景

- Scene Viewport（主视口）
- Material Preview（材质预览）
- Mesh Preview（模型预览）
- Texture Preview（纹理预览）
- Shader Preview（着色器预览）

### 属性

| 属性 | 类型 | 说明 |
|------|------|------|
| `Count` | `int` | 获取已注册表面数量 |

### 方法

| 方法 | 返回类型 | 说明 |
|------|----------|------|
| `Register(IntPtr nativeHandle, uint handleType, uint width, uint height)` | `ulong` | 注册一个新的视口表面。返回表面 ID。 |
| `Unregister(ulong surfaceId)` | `void` | 注销一个视口表面。 |
| `MarkResize(ulong surfaceId, uint width, uint height)` | `void` | 标记 Deferred Resize（不立即执行 ResizeBuffers，帧末统一 Flush）。 |
| `FlushResizes()` | `void` | 帧末统一执行所有 Deferred Resize。 |
| `Present(ulong surfaceId)` | `void` | Present SwapChain（提交渲染结果到屏幕）。 |
| `MarkSurfaceLost(ulong surfaceId)` | `void` | 标记表面丢失（HWND 被销毁/重建，如 Dock 浮动窗口）。 |
| `RecreateSurface(ulong surfaceId, IntPtr newNativeHandle, uint newHandleType)` | `bool` | 重建丢失的表面（新 HWND）。 |
| `IsSurfaceLost(ulong surfaceId)` | `bool` | 表面是否丢失。 |
| `IsRegistered(ulong surfaceId)` | `bool` | 表面是否已注册。 |
| `RenderViewportCommands(ulong surfaceId, byte[] commands)` | `bool` | 通过渲染命令缓冲区渲染视口。 |

### RenderViewportCommands 数据流

```
C# Scene → CollectRenderCommands → byte[] → RenderViewportCommands
→ C++ NNViewportSurfaceApi.RenderViewportCommands
→ 解析 Commands → SetCamera / SetRenderPassState / DrawSpriteBatch
→ Renderer2D（World Pass）
→ CopyTexture → SwapChain → Present
```

---

## 3. ViewportSurfaceRegistryImpl

**文件**: `ViewportSurfaceRegistryImpl.cs` · **命名空间**: `Neverness.Rendering.Core`

视口表面注册表实现。委托给 Native NNViewportSurfaceApi 函数指针。

### 类定义

```csharp
public sealed unsafe class ViewportSurfaceRegistryImpl : IViewportSurfaceRegistry
```

### 构造函数

无参构造。通过 `EngineNativeApiBootstrap.EngineApi` 访问 Native API。

### 实现说明

- `Register`: 调用 `api.ViewportSurface.CreateSurface`
- `Unregister`: 调用 `api.ViewportSurface.DestroySurface`
- `MarkResize`: 调用 `api.ViewportSurface.MarkResize`
- `FlushResizes`: 调用 `api.ViewportSurface.FlushResizes`
- `Present`: 调用 `api.ViewportSurface.Present`
- `RecreateSurface`: 调用 `api.ViewportSurface.RecreateSurface`
- `IsSurfaceLost`: 调用 `api.ViewportSurface.IsSurfaceLost`
- `RenderViewportCommands`: 调用 `api.ViewportSurface.RenderViewportCommands`（需要 layoutVersion >= 29）

---

## 4. IViewportService

**文件**: `IViewportService.cs` · **命名空间**: `Neverness.Rendering.Core`

视口服务接口。提供场景渲染和视口操作。Controller 通过服务定位器消费。

### 属性

| 属性 | 类型 | 说明 |
|------|------|------|
| `HasScene` | `bool` | 是否有有效的场景 |
| `AssetPathResolver` | `Func<NNGuid, string?>?` | 资产 GUID → VFSService 路径解析器（由上层注入） |

### 方法

| 方法 | 返回类型 | 说明 |
|------|----------|------|
| `GetLastRmluiTextureId()` | `ulong` | 获取最后渲染的 RmlUI 纹理 ID |
| `SetScene(SceneWorld? scene)` | `void` | 设置关联的场景 |
| `FocusEntity(IEntity entity)` | `void` | 聚焦到指定实体（摄像机移动） |
| `SetCameraPosition(float x, float y, float z)` | `void` | 设置摄像机位置 |
| `SetRmlUIViewportSize(uint width, uint height)` | `void` | 设置 RmlUI 视口尺寸 |
| `CollectRenderCommands(float width, float height)` | `byte[]?` | 从 ECS 收集渲染命令并序列化为 Flat Buffer |

### CollectRenderCommands 数据流

```
Friflo ECS
├── TransformComponent + CameraComponent
│   → 计算 ViewMatrix / ProjectionMatrix
│   → buffer.AddSetCamera(...)
├── SetRenderPassState
│   → buffer.AddSetRenderPassState(clearColor, flags)
├── TransformComponent + SpriteRendererComponent
│   → 收集精灵实例（WorldMatrix、Color、UV、TextureHandle）
│   → buffer.AddDrawSpriteBatch(sprites)
└── RmlUIDocumentComponent
    → AssetPathResolver(doc.DocumentAsset) → VFS 路径
    → buffer.AddSetRmlDocuments(entries)

buffer.Build() → byte[]
```

---

## 5. ViewportServiceImpl

**文件**: `ViewportServiceImpl.cs` · **命名空间**: `Neverness.Rendering.Core`

视口服务实现。封装 Native 渲染 API，对外暴露 IViewportService 接口。

### 类定义

```csharp
public sealed unsafe class ViewportServiceImpl : IViewportService
```

### 属性

| 属性 | 类型 | 说明 |
|------|------|------|
| `Scene` | `SceneWorld?` | 当前关联的场景 |
| `HasScene` | `bool` | 是否有有效的场景 |
| `AssetPathResolver` | `Func<NNGuid, string?>?` | 资产路径解析器 |

### 方法实现

#### SetScene

```csharp
public void SetScene(SceneWorld? scene)
```

设置关联的场景。清空或设置内部 `_scene` 引用。

#### GetLastRmluiTextureId

```csharp
public ulong GetLastRmluiTextureId()
```

调用 `api.ViewportRender.GetLastRmluiTexture` 获取最后渲染的 RmlUI 纹理 ID。

#### FocusEntity / SetCameraPosition

```csharp
public void FocusEntity(IEntity entity)
public void SetCameraPosition(float x, float y, float z)
```

当前 Native API 不支持，输出警告日志。需要 Native 端在 NNViewportRenderApi 中添加对应接口。

#### SetRmlUIViewportSize

```csharp
public void SetRmlUIViewportSize(uint width, uint height)
```

调用 `api.ViewportRender.SetRmlUIViewportSize` 设置 RmlUI 视口尺寸。

#### CollectRenderCommands

```csharp
public byte[]? CollectRenderCommands(float width, float height)
```

从 Friflo ECS 收集渲染命令并序列化为 Flat Buffer。

**返回**: 序列化的命令缓冲区（可直接传给 `RenderViewportCommands`），无场景时返回 null。

**调试日志**: 每 120 帧打印一次（约 2 秒 @60fps）。

**收集步骤**:

1. **相机数据**: 取第一个 `CameraComponent`，计算 ViewMatrix / ProjectionMatrix，调用 `buffer.AddSetCamera`
2. **渲染 Pass 状态**: 调用 `buffer.AddSetRenderPassState`（ClearColor + DepthTest）
3. **精灵数据**: 遍历 `SpriteRendererComponent`，收集可见精灵实例，调用 `buffer.AddDrawSpriteBatch`
4. **RmlUI 文档**: 遍历 `RmlUIDocumentComponent`，解析 GUID → VFS 路径，调用 `buffer.AddSetRmlDocuments`
5. **构建**: `buffer.Build()` 返回 `byte[]`

---

## 附录：类型引用

### 来自 Neverness.Runtime.Engine

| 类型 | 说明 |
|------|------|
| `EngineNativeApiBootstrap` | Native API 引导，提供 `EngineApi` 访问 |
| `NNGuid` | 资产 GUID 结构体 |
| `NNNativeHandleType` | 原生句柄类型枚举 |

### 来自 Neverness.Runtime.Scene

| 类型 | 说明 |
|------|------|
| `SceneWorld` | 场景世界，持有 Friflo ECS |
| `IEntity` | ECS 实体接口 |
| `TransformComponent` | 变换组件（Position、Rotation、Scale、WorldMatrix） |
| `CameraComponent` | 相机组件（ProjectionMatrix、NearPlane、FarPlane 等） |
| `SpriteRendererComponent` | 精灵渲染组件（TextureAsset、Color、UV、Flags 等） |
| `RmlUIDocumentComponent` | RmlUI 文档组件（DocumentAsset、SortOrder、ViewTarget） |

### 来自 Neverness.Rendering.Diligent.Commands

| 类型 | 说明 |
|------|------|
| `RenderCommandBuffer` | 命令缓冲区构建器 |
| `RenderCommandType` | 命令类型枚举 |
| `RenderPassFlags` | 渲染 Pass 标志位 |
| `SpriteInstance` | 精灵实例数据结构（120 bytes） |
| `RmlDocumentEntry` | RmlUI 文档条目（276 bytes） |

### 来自 Neverness.Rendering.RenderAssets

| 类型 | 说明 |
|------|------|
| `RenderAssetManager` | 渲染资产管理器（纹理加载） |
