# Neverness.Rendering.Core 模块下沉计划

## 背景

EditorApplicationRunner.cs 第208行的 `AvaloniaFrontendModule.TickRendering()` 负责主线程渲染调度，目前仅 Editor 可用。Runtime 游戏运行时也需要相同的渲染回调机制。需要将渲染循环相关代码从 Editor 下沉到独立的 Rendering 模块。

## 目标

1. 创建 `Neverness.Rendering.Core` 模块
2. 将渲染回调机制从 `AvaloniaFrontendModule` 移出
3. 将 `IViewportSurfaceRegistry` 接口和实现从 Editor 移出
4. 保持 Runtime 和 Editor 都能使用渲染循环

---

## 下沉范围

### 1. 渲染回调机制（从 AvaloniaFrontendModule）

**源文件**: `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Public/AvaloniaFrontendModule.cs`

**移出内容**:
```csharp
// 第305-337行
private static readonly List<Action> _renderCallbacks = new();

public static void RegisterRenderCallback(Action callback)
{
    if (!_renderCallbacks.Contains(callback))
        _renderCallbacks.Add(callback);
}

public static void UnregisterRenderCallback(Action callback)
{
    _renderCallbacks.Remove(callback);
}

public static void TickRendering()
{
    foreach (var callback in _renderCallbacks)
    {
        try { callback(); }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"[AvaloniaFrontendModule] 渲染回调异常: {ex.Message}");
        }
    }
}
```

**目标**: `Neverness.Rendering.Core/RenderingLoop.cs`

---

### 2. IViewportSurfaceRegistry 接口（从 Editor.Core）

**源文件**: `Engine/Source/Managed/Editor/Neverness.Editor.Core/Public/IViewportSurfaceRegistry.cs`

**移出内容**: 整个接口定义（第1-74行）

**目标**: `Neverness.Rendering.Core/IViewportSurfaceRegistry.cs`

**命名空间变更**: `Neverness.Editor.Core` → `Neverness.Rendering.Core`

---

### 3. ViewportSurfaceRegistryImpl 实现（从 Editor.Scene）

**源文件**: `Engine/Source/Managed/Editor/Neverness.Editor.Scene/Private/Service/ViewportSurfaceRegistryImpl.cs`

**移出内容**: 整个实现类（第1-171行）

**目标**: `Neverness.Rendering.Core/ViewportSurfaceRegistryImpl.cs`

**命名空间变更**: `Neverness.Editor.Scene.Private.Service` → `Neverness.Rendering.Core`

---

## 新模块结构

```
Engine/Source/Managed/Rendering/Neverness.Rendering.Core/
├── Neverness.Rendering.Core.csproj
├── RenderingLoop.cs                    // 渲染回调调度器（静态类）
├── IViewportSurfaceRegistry.cs         // 视口表面注册表接口
├── ViewportSurfaceRegistryImpl.cs      // 视口表面注册表实现
├── IViewportService.cs                 // 视口服务接口
└── ViewportServiceImpl.cs              // 视口服务实现（CollectRenderCommands）
```

---

## 依赖关系调整

### 新增依赖

| 模块 | 依赖 |
|------|------|
| `Neverness.Rendering.Core` | `Neverness.Runtime.Engine`（用于 EngineNativeApiBootstrap） |

### 修改依赖

| 模块 | 变更 |
|------|------|
| `Neverness.Editor.AvaloniaFrontend` | **新增** `Neverness.Rendering.Core` |
| `Neverness.Editor.Scene` | **新增** `Neverness.Rendering.Core`，**移除** `Neverness.Editor.Core` 对 IViewportSurfaceRegistry 的依赖 |
| `Neverness.Editor.Core` | **移除** `IViewportSurfaceRegistry.cs` |

### Runtime 使用

Runtime 只需要渲染回调机制，不需要 IViewportSurfaceRegistry。

**选项 A**（推荐）: Runtime.Application 依赖 Rendering.Core，使用 `RenderingLoop.TickRendering()`

**选项 B**: Runtime.Application 复制 RenderingLoop 代码（不推荐，违反 DRY）

---

## 实施步骤

### Phase 1: 创建 Neverness.Rendering.Core 模块

1. 创建目录 `Engine/Source/Managed/Rendering/Neverness.Rendering.Core/`
2. 创建 `Neverness.Rendering.Core.csproj`
3. 创建 `RenderingLoop.cs`（渲染回调调度器）
4. 创建 `IViewportSurfaceRegistry.cs`（从 Editor.Core 复制）
5. 创建 `ViewportSurfaceRegistryImpl.cs`（从 Editor.Scene 复制）

### Phase 2: 更新 Editor 模块引用

1. 修改 `AvaloniaFrontendModule.cs`:
   - 移除 `_renderCallbacks`、`RegisterRenderCallback`、`UnregisterRenderCallback`、`TickRendering`
   - 改为调用 `Neverness.Rendering.Core.RenderingLoop.XXX`

2. 修改 `ViewportAvaloniaView.cs`:
   - `using Neverness.Editor.Core` → `using Neverness.Rendering.Core`
   - `AvaloniaFrontendModule.RegisterRenderCallback` → `RenderingLoop.RegisterRenderCallback`

3. 修改 `EditorApplicationRunner.cs`:
   - `AvaloniaFrontendModule.TickRendering()` → `RenderingLoop.TickRendering()`

4. 修改 `Editor.Core.csproj`:
   - 移除 `IViewportSurfaceRegistry.cs`

5. 修改 `Editor.Scene.csproj`:
   - 新增 `Neverness.Rendering.Core` 依赖
   - 移除 `ViewportSurfaceRegistryImpl.cs`

### Phase 3: 更新 Runtime 模块引用

1. 修改 `Runtime.Application.csproj`:
   - 新增 `Neverness.Rendering.Core` 依赖

2. 修改 `ApplicationHost.cs`（如果需要）:
   - 在主循环中调用 `RenderingLoop.TickRendering()`

### Phase 4: 验证

1. 编译所有受影响的模块
2. 运行 Editor 验证渲染正常
3. 运行 Runtime 验证渲染回调机制可用

---

## 关键设计决策

### 1. RenderingLoop 作为静态类

```csharp
namespace Neverness.Rendering.Core;

/// <summary>
/// 主线程渲染调度器——管理渲染回调的注册和执行。
///
/// 设计原则：
/// - 纯静态类，无状态依赖
/// - 线程安全：回调列表使用 lock 保护
/// - 异常隔离：单个回调异常不影响其他回调
/// - 跨模块共享：Editor 和 Runtime 都可使用
/// </summary>
public static class RenderingLoop
{
    private static readonly List<Action> s_callbacks = new();
    private static readonly object s_lock = new();

    public static void RegisterRenderCallback(Action callback)
    {
        lock (s_lock)
        {
            if (!s_callbacks.Contains(callback))
                s_callbacks.Add(callback);
        }
    }

    public static void UnregisterRenderCallback(Action callback)
    {
        lock (s_lock)
        {
            s_callbacks.Remove(callback);
        }
    }

    public static void TickRendering()
    {
        Action[] snapshot;
        lock (s_lock)
        {
            snapshot = s_callbacks.ToArray();
        }

        foreach (var callback in snapshot)
        {
            try { callback(); }
            catch (Exception ex)
            {
                Console.Error.WriteLine($"[RenderingLoop] 渲染回调异常: {ex.Message}");
            }
        }
    }
}
```

### 2. IViewportSurfaceRegistry 保持不变

接口定义已经足够通用，不需要修改。

### 3. ViewportSurfaceRegistryImpl 命名空间变更

从 `Neverness.Editor.Scene.Private.Service` 移到 `Neverness.Rendering.Core`，实现保持不变。

---

## 风险和注意事项

1. **命名空间变更**: 所有使用 `IViewportSurfaceRegistry` 的地方都需要更新 `using`
2. **服务注册**: `SceneModuleImp` 中注册 `IViewportSurfaceRegistry` 的代码需要更新类型引用
3. **Runtime 渲染时序**: Runtime 主循环需要在正确时机调用 `RenderingLoop.TickRendering()`
4. **线程安全**: 当前 `AvaloniaFrontendModule._renderCallbacks` 无锁保护，下沉后应添加

---

## 问题确认

1. **Runtime 主循环位置**: Runtime 的 `ApplicationHost.cs` 是否已有主循环？如果有，在哪里调用 `TickRendering` 最合适？
2. **服务注册**: `IViewportSurfaceRegistry` 是否应该注册为全局服务？目前在 `SceneModuleImp` 中注册。
3. **命名空间**: 是否接受 `Neverness.Rendering.Core` 作为新命名空间？

---

## 实施状态

**Phase 1: 创建 Neverness.Rendering.Core 模块** ✅ 已完成

- [x] 创建目录 `Engine/Source/Managed/Rendering/Neverness.Rendering.Core/`
- [x] 创建 `Neverness.Rendering.Core.csproj`
- [x] 创建 `RenderingLoop.cs`（渲染回调调度器，添加了线程安全锁）
- [x] 创建 `IViewportSurfaceRegistry.cs`（从 Editor.Core 复制）
- [x] 创建 `ViewportSurfaceRegistryImpl.cs`（从 Editor.Scene 复制）

**Phase 2: 更新 Editor 模块引用** ✅ 已完成

- [x] 修改 `AvaloniaFrontendModule.cs`: 移除 `_renderCallbacks`、`RegisterRenderCallback`、`UnregisterRenderCallback`、`TickRendering`，添加 `using Neverness.Rendering.Core;`
- [x] 修改 `ViewportAvaloniaView.cs`: `AvaloniaFrontendModule.RegisterRenderCallback` → `RenderingLoop.RegisterRenderCallback`
- [x] 修改 `EditorApplicationRunner.cs`: `AvaloniaFrontendModule.TickRendering()` → `RenderingLoop.TickRendering()`
- [x] 修改 `Editor.Scene.csproj`: 新增 `Neverness.Rendering.Core` 依赖
- [x] 修改 `SceneModuleImp.cs`: `using Neverness.Editor.Scene.Private.Service` → `using Neverness.Rendering.Core`
- [x] 修改 `AvaloniaFrontend.csproj`: 新增 `Neverness.Rendering.Core` 依赖
- [x] 删除 `Editor.Core/Public/IViewportSurfaceRegistry.cs`
- [x] 删除 `Editor.Scene/Private/Service/ViewportSurfaceRegistryImpl.cs`

**Phase 3: 更新 Runtime 模块引用** ✅ 已完成

- [x] 修改 `Runtime.Application.csproj`: 新增 `Neverness.Rendering.Core` 依赖

**Phase 4: CollectRenderCommands 下沉** ✅ 已完成

- [x] 更新 `Neverness.Rendering.Core.csproj` 添加依赖（Rendering.Diligent、Rendering.RenderAssets、Runtime.Scene）
- [x] 创建 `IViewportService.cs`（从 Editor.Core 移入）
- [x] 创建 `ViewportServiceImpl.cs`（从 Editor.Scene 移入）
- [x] 删除 `Editor.Core/Public/IViewportService.cs`
- [x] 删除 `Editor.Scene/Private/Service/ViewportServiceImpl.cs`
- [x] 更新 `EditorViewportController.cs` 添加 `using Neverness.Rendering.Core;`
- [x] 更新 `EditorCompositionRoot.cs` 添加 `using Neverness.Rendering.Core;`
- [x] 更新 `Editor.Core.csproj` 添加 `Neverness.Rendering.Core` 依赖
- [x] 验证无旧命名空间引用

**Phase 5: 文档更新** ✅ 已完成

- [x] 创建 `Neverness.Rendering.Core/Docs/README.md`（核心设计、使用示例、模块结构、数据流）
- [x] 创建 `Neverness.Rendering.Core/Docs/API-Reference.md`（完整 API 参考手册）

**Phase 6: 验证** ⏳ 待编译验证

- [ ] 编译所有受影响的模块
- [ ] 运行 Editor 验证渲染正常
- [ ] 运行 Runtime 验证渲染回调机制可用
