# Neverness.Rendering.Core

渲染核心模块——提供主线程渲染调度、视口表面管理、视口服务接口。

从 Editor 模块下沉，使 Runtime 和 Editor 共享渲染基础设施。

## 核心设计

- **RenderingLoop**: 线程安全的渲染回调调度器，Editor 和 Runtime 共享
- **IViewportSurfaceRegistry**: 视口表面生命周期管理（Register/Resize/SurfaceLost/Recreate）
- **IViewportService**: 视口服务接口（CollectRenderCommands、场景绑定）
- **依赖隔离**: 不依赖 Avalonia/ImGui，纯渲染逻辑

## 模块结构

```
Neverness.Rendering.Core/
├── RenderingLoop.cs                    // 主线程渲染调度器（静态类）
├── IViewportSurfaceRegistry.cs         // 视口表面注册表接口
├── ViewportSurfaceRegistryImpl.cs      // 视口表面注册表实现
├── IViewportService.cs                 // 视口服务接口
└── ViewportServiceImpl.cs              // 视口服务实现（CollectRenderCommands）
```

## 依赖关系

```
Neverness.Rendering.Core
├── Neverness.Runtime.Engine            // Native API 访问
├── Neverness.Runtime.Scene             // Friflo ECS（SceneWorld、Components）
├── Neverness.Rendering.Diligent        // RenderCommandBuffer、RenderCommandTypes
└── Neverness.Rendering.RenderAssets    // RenderAssetManager（纹理加载）
```

## 使用示例

### 1. RenderingLoop — 主线程渲染回调

```csharp
using Neverness.Rendering.Core;

// 注册渲染回调（View 初始化时）
RenderingLoop.RegisterRenderCallback(OnMainThreadRender);

// 注销渲染回调（View 销毁时）
RenderingLoop.UnregisterRenderCallback(OnMainThreadRender);

// 主循环中调用（每帧）
while (running)
{
    SDL_PollEvent(...);
    RenderingLoop.TickRendering();  // ← 执行所有已注册回调
    SwapChain.Present();
}

// 清理（退出时）
RenderingLoop.ClearAll();
```

### 2. IViewportSurfaceRegistry — 视口表面管理

```csharp
using Neverness.Rendering.Core;

// 创建注册表（SceneModule 安装时）
var registry = new ViewportSurfaceRegistryImpl();

// 注册视口表面（View 创建时）
ulong surfaceId = registry.Register(nativeHandle, handleType, width, height);

// Deferred Resize（窗口 resize 时）
registry.MarkResize(surfaceId, newWidth, newHeight);

// 帧末统一执行 Resize（渲染回调中）
registry.FlushResizes();

// 渲染视口（渲染回调中）
registry.RenderViewportCommands(surfaceId, commands);

// 注销（View 销毁时）
registry.Unregister(surfaceId);
```

### 3. IViewportService — 视口服务

```csharp
using Neverness.Rendering.Core;

// 注册服务（SceneModule 安装时）
var viewportService = new ViewportServiceImpl();
context.RegisterService<IViewportService>(viewportService);

// 注入资产路径解析器（Editor 启动时）
viewportService.AssetPathResolver = guid =>
{
    if (EditorAssetDatabase.TryGetPath(guid, out var path))
        return path.FullPath;
    return null;
};

// 设置场景（场景切换时）
viewportService.SetScene(sceneWorld);

// 收集渲染命令（渲染回调中）
byte[]? commands = viewportService.CollectRenderCommands(width, height);
if (commands != null)
{
    surfaceRegistry.RenderViewportCommands(surfaceId, commands);
}
```

## 渲染管线数据流

```
Editor 主循环:
  ApplicationHost.PumpEvents()
  → RenderingLoop.TickRendering()
    → ViewportAvaloniaView.OnMainThreadRender()
      → FlushResizes()
      → CollectRenderCommands(width, height)
        → Friflo ECS (TransformComponent + CameraComponent + SpriteRendererComponent)
        → RenderCommandBuffer.AddSetCamera()
        → RenderCommandBuffer.AddSetRenderPassState()
        → RenderCommandBuffer.AddDrawSpriteBatch()
        → RenderCommandBuffer.AddSetRmlDocuments()
        → RenderCommandBuffer.Build() → byte[]
      → ViewportSurfaceRegistry.RenderViewportCommands(surfaceId, commands)
        → C++ NNViewportSurfaceApi.RenderViewportCommands()
          → 解析 Commands → Renderer2D → CopyTexture → SwapChain → Present
```

## 边界隔离规则

- **不依赖 UI 框架**: 不 import Avalonia/ImGui，纯渲染逻辑
- **不依赖 Editor 模块**: Runtime 可独立使用
- **Native API 通过 Engine**: 使用 `EngineNativeApiBootstrap.EngineApi` 访问 C++ 函数指针
- **ECS 通过 Runtime.Scene**: 使用 Friflo ECS 查询组件数据

## 文档索引

| 文档 | 说明 |
|------|------|
| [README.md](README.md) | 核心设计、使用示例、模块结构、数据流 |
| [API-Reference.md](API-Reference.md) | 完整 API 参考手册 |
