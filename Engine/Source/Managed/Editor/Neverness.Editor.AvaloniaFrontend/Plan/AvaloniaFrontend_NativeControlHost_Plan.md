# NativeControlHost + Diligent SwapChain 实施计划

> 状态：Phase 0-4.5 已完成，SwapChain 渲染管线工作正常
> 日期：2026-06-14

---

## Context

### 目标
在 Avalonia 12.0.4 中使用 `NativeControlHost` 嵌入原生窗口，让 Diligent 通过 SwapChain 直接渲染到该窗口。

### 当前状态
- `NativeControlHostSurface.cs`、`ViewportHostService.cs`、`IViewportSurface.cs` 已有骨架但都是占位实现
- ImGui 版本使用离屏渲染（`RenderSceneToTexture` → OpenGL Texture ID → `ImGui.Image`）
- Avalonia 版需要 NativeControlHost + SwapChain 直接渲染

### 关键约束
- **禁止反射和内部 PlatformImpl API**
- **Device/Context 全局共享**
- **允许多个 ViewportSurface（Scene、Material、Mesh 等）同时存在**
- **每个 Surface 持有独立 SwapChain**
- **资源统一由 Renderer 管理**

---

## Phase 0：NativeControlHost 实验验证 ✅ 已完成

**目标**：验证 NativeControlHost 在 Avalonia 12.0.4 下的实际行为

**实验结论**：
1. ✅ NativeControlHost 自动创建 `AvaloniaDumbWindow-<GUID>` 子窗口（需要 app.manifest）
2. ✅ `TopLevel.TryGetPlatformHandle()` 返回主窗口 HWND，不是子窗口
3. ✅ 子窗口类名带 GUID 后缀，`FindWindowEx` 精确匹配失败
4. ✅ `FindWindowEx(mainHwnd, 0, null, null)` 可取第一个子窗口（但多 Host 不安全）
5. ✅ `CreateNativeControlCore` 是 `protected virtual`，是官方扩展点

**关键发现**：
- Avalonia 内部流程：`UpdateHost()` → `CreateNewAttachment()` → 创建 DumbWindow → 回调 `CreateNativeControlCore(parent)`
- `parent` 参数就是 DumbWindow 的 `IPlatformHandle`，`parent.Handle` 就是 HWND
- `NativeControlHost.NativeControlHandle` 是 internal，无法直接访问
- `INativeControlHostImpl` 标记为 `[Unstable]`

**文件变更**：
1. **新建** `app.manifest` — Windows DPI 感知 + 兼容性声明
2. **新建** `AvaloniaViewportHostExperiment.cs` — 实验验证类
3. **修改** `NevernessEditor.csproj` — 添加 `<ApplicationManifest>`

---

## Phase 1：ViewportHostControl 子类化 ✅ 已完成

**目标**：通过重写 `CreateNativeControlCore` 自定义子窗口创建

**设计**：
```
ViewportHostControl : NativeControlHost
├── CreateNativeControlCore(parent) → CreateWindowEx("STATIC") → 保存 HWND
├── DestroyNativeControlCore(control) → DestroyWindow → 清理
├── HandleCreated 事件 → NativeControlHostSurface 监听
└── HandleDestroyed 事件 → NativeControlHostSurface 监听
```

**文件变更**：
1. **新建** `ViewportHostControl.cs` — 子类化 NativeControlHost，重写 CreateNativeControlCore
2. **修改** `NativeControlHostSurface.cs` — 使用 ViewportHostControl 替代 FindWindowEx
3. **修改** `ViewportHostService.cs` — GetControl 返回 ViewportHostControl

**验证方式**：编辑器启动，控制台输出 `[ViewportHostControl] 子窗口已创建: HWND=0x...`

---

## Phase 2：Native ViewportSurface API

**目标**：新增独立的 `NNViewportSurfaceApi`，与 `NNViewportRenderApi` 分离

**关键设计**：
- **职责分离**：`NNViewportSurfaceApi` 管理 SwapChain 生命周期（Renderer 基础设施），`NNViewportRenderApi` 只负责离屏渲染（Editor 功能）
- **跨平台**：`CreateSurface` 接收 `nativeHandle + NNNativeHandleType`
- **Deferred Resize**：`MarkResize` → `FlushResizes`（帧末统一执行，避免 Dock 拖动时 ResizeBuffers 风暴）
- **Surface Lost**：`IsSurfaceLost` + `RecreateSurface`（Dock 浮动后 HWND 重建）
- **cameraId 是临时设计**：TODO 改为 ViewportContext（Material/Texture/Shader Preview 无 Camera）

**新增 Native API（C++ 端）**：
```cpp
// ViewportSurfaceAPI.h（新文件，与 ViewportRenderAPI.h 分离）
enum class NNNativeHandleType : uint32_t { Win32HWND=0, X11Window=1, WaylandSurface=2, NSView=3 };

typedef uint64_t NNViewportSurfaceHandle;

struct NNViewportSurfaceAPI {
    NNViewportSurfaceHandle (*CreateSurface)(void* nativeHandle, NNNativeHandleType type, uint32_t w, uint32_t h);
    void (*DestroySurface)(NNViewportSurfaceHandle surface);
    void (*MarkResize)(NNViewportSurfaceHandle surface, uint32_t w, uint32_t h);
    void (*FlushResizes)();
    void (*Present)(NNViewportSurfaceHandle surface);
    uint8_t (*IsSurfaceLost)(NNViewportSurfaceHandle surface);
    uint8_t (*RecreateSurface)(NNViewportSurfaceHandle surface, void* newHandle, NNNativeHandleType type);
};
```

**C# 端**（✅ 已写入 `NNNativeEngineApiTypes.cs`）：
```csharp
public struct NNViewportSurfaceApi { CreateSurface, DestroySurface, MarkResize, FlushResizes, Present, IsSurfaceLost, RecreateSurface }
public enum NNNativeHandleType : uint { Win32HWND=0, X11Window=1, WaylandSurface=2, NSView=3 }
```

**文件变更**：
1. **新建** `Engine/Source/Runtime/NNNativeEngineAPI/Include/ViewportSurfaceAPI.h`
   - `NNNativeHandleType` 枚举
   - `NNViewportSurfaceAPI` 结构体（7 个函数指针）

2. ✅ **已修改** `Engine/Source/Managed/Runtime/Neverness.Runtime.Engine/NNNativeEngineApiTypes.cs`
   - `NNViewportSurfaceApi` 结构体
   - `NNNativeHandleType` 枚举
   - `NNNativeEngineApi.ViewportSurface` 字段

3. **新建** `Engine/Source/Rendering/NNRuntimeDiligent/Source/Device/NNDiligentViewportSurface.cpp`
   - 管理单个 ViewportSurface 的 SwapChain
   - Surface Lost 检测和重建

4. **新建** `Engine/Source/Runtime/NNRuntimeEngineServices/Private/ViewportSurface/ViewportSurfaceRuntimeApi.cpp`
   - 实现 7 个函数
   - Deferred Resize Queue 管理

**依赖关系**：Phase 0 验证通过

---

## Phase 3：IViewportService 扩展 + IViewportSurfaceRegistry

**目标**：扩展 IViewportService 支持 SwapChain 模式，创建跨前端共享的 Surface 注册表

**设计**：
```
IViewportSurfaceRegistry (Editor.Core，跨前端共享)
├── Register(nativeHandle, handleType, width, height) → surfaceId
├── Unregister(surfaceId)
├── Resize(surfaceId, width, height) → Deferred
├── CommitResize() → 帧末执行
├── RenderViewport(surfaceId, cameraId)
└── ImGuiFrontend / AvaloniaFrontend 共用

IViewportService (扩展)
├── RenderSceneToTexture() → 离屏模式（ImGui 用，保留）
└── CreateViewportSurface() → SwapChain 模式（委托给 Registry）
```

**文件变更**：
1. ✅ **已新建** `Engine/Source/Managed/Editor/Neverness.Editor.Core/Public/IViewportSurfaceRegistry.cs`
   - 注册/注销/Deferred Resize/RenderViewport 接口
   - `NativeHandleType` 常量类（与 NNNativeHandleType 对齐）

2. **新建** `Engine/Source/Managed/Editor/Neverness.Editor.Scene/Private/Service/ViewportSurfaceRegistryImpl.cs`
   - 实现 IViewportSurfaceRegistry
   - 委托给 Native API

3. **修改** `Engine/Source/Managed/Editor/Neverness.Editor.Core/Public/IViewportService.cs`
   - 新增 CreateViewportSurface/DestroyViewportSurface/RenderViewport
   - 委托给 IViewportSurfaceRegistry

**依赖关系**：Phase 2

---

## Phase 4：Scene Viewport 集成

**目标**：将 NativeControlHost + SwapChain 集成到 ViewportAvaloniaView

**关键设计**：
- **ResizeObserver**：监听 `BoundsProperty` 变化 → `MarkResize` → `CompositionTarget.Rendering` 帧末 `FlushResizes`
- **Surface Lost**：`HandleDestroyed` → `MarkSurfaceLost` → `HandleCreated` → `RecreateSurface`

**数据流**：
```
ViewportAvaloniaView.Bind()
  → ViewportHostService.CreateSurface(w, h)
    → new NativeControlHostSurface(w, h)
      → new ViewportHostControl()
  → service.GetControl() → 添加到 _viewportPanel
  → HandleCreated 事件 → 获取原生句柄 + handleType
  → IViewportSurfaceRegistry.Register(handle, type, w, h) → surfaceId
  → BoundsProperty 变化 → MarkResize(surfaceId, w, h)
  → CompositionTarget.Rendering → FlushResizes()
  → EditorViewportController.RenderScene()
    → IViewportSurfaceRegistry.RenderViewport(surfaceId, cameraId)
    → IViewportSurfaceRegistry.Present(surfaceId)

Dock 浮动窗口场景：
  → HandleDestroyed → MarkSurfaceLost(surfaceId)
  → HandleCreated (新 HWND) → RecreateSurface(surfaceId, newHandle, type)
  → 继续正常渲染
```

**文件变更**：
1. ✅ **已修改** `ViewportAvaloniaView.cs` — 使用 ViewportHostService 正式集成
2. ✅ **已修改** `ViewportHostControl.cs` — 直接使用 parent（DumbWindow），不 CreateWindowEx
3. ✅ **已修改** `NativeControlHostSurface.cs` — 存储 handleDescriptor
4. **待实现** `ViewportResizeObserver` — BoundsProperty + CompositionTarget.Rendering Deferred Resize
5. **待实现** Surface Lost 处理逻辑（在 ViewportAvaloniaView 中）

**依赖关系**：Phase 1 + Phase 3

---

## Phase 4.5：SwapChain 渲染管线 ✅ 已完成

**目标**：实现完整的 SwapChain 渲染路径（FBO → CopyTexture → SwapChain → Present）

**实现**：
- `ViewportSurfaceAPI.h`：新增 `RenderViewport(surfaceId, sceneHandle, w, h)` 函数指针
- `NNDiligentViewportSurface`：新增 `GetDevice()` / `GetContext()` 访问器
- `ViewportSurfaceRuntimeApi.cpp`：RenderSceneToTexture → CopyTexture → Present
- `ViewportAvaloniaView`：主线程渲染回调 + Bounds 变化更新 ViewModel

**修复的问题**：
1. SwapChain 0x0 尺寸 → 回退到面板尺寸
2. Diligent "outstanding commands" → 线程问题，主线程渲染回调
3. Flush 在 CopyTexture 前 → 移除 Flush
4. 多余的 EndRenderPass → 根因是线程，不是 RenderPass
5. Viewport 不跟随窗口大小 → OnBoundsChanged 更新 ViewModel

**依赖关系**：Phase 4

---

## Phase 5：Material Preview

**目标**：实现 Material 预览视口，验证多 Surface 共存

**设计**：
- 独立 ViewportHostControl + 独立 SwapChain
- 渲染 Sphere + Material + PreviewCamera
- 无 Scene，只有 cameraId
- 验证 Device/Context 共享、资源同步

**文件变更**：
1. **新建** `Views/MaterialPreviewView.cs`
2. **新建** `Inspectors/MaterialPreviewPanel.cs`

**依赖关系**：Phase 4.5

---

## Phase 6：Mesh / Texture / Shader Preview

**目标**：扩展更多预览视口

**文件变更**：
1. **新建** `MeshPreviewView.cs` — 模型预览
2. **新建** `TexturePreviewView.cs` — 纹理预览
3. **新建** `ShaderPreviewView.cs` — 着色器预览

**依赖关系**：Phase 5

---

## 执行顺序

```
Phase 0 (实验验证) ✅
    │
    v
Phase 1 (ViewportHostControl 子类化) ✅
    │
    v
Phase 2 (Native SwapChain API) ✅
    │
    v
Phase 3 (IViewportService + IViewportSurfaceRegistry) ✅
    │
    v
Phase 4 (Scene Viewport 集成) ✅
    │
    v
Phase 4.5 (SwapChain 渲染) ✅
    │
    v
Phase 5 (Material Preview)
    │
    v
Phase 6 (Mesh / Texture / Shader Preview)
```

---

## 实施进度跟踪

| Phase | 状态 | 完成日期 | 说明 |
|-------|------|----------|------|
| Phase 0：实验验证 | ✅ 已完成 | 2026-06-14 | NativeControlHost 行为验证、app.manifest、FindWindowEx 实验 |
| Phase 1：ViewportHostControl | ✅ 已完成 | 2026-06-14 | 直接使用 parent（DumbWindow），不 CreateWindowEx |
| Phase 2：Native SwapChain API | ✅ 已完成 | 2026-06-14 | ViewportSurfaceAPI.h + NNDiligentViewportSurface + ViewportSurfaceRuntimeApi + LayoutVersion 23 |
| Phase 3：IViewportService + Registry | ✅ 已完成 | 2026-06-14 | ViewportSurfaceRegistryImpl + SceneModuleImp 注册 |
| Phase 4：Scene Viewport | ✅ 已完成 | 2026-06-14 | Deferred Resize + SurfaceRegistry + SwapChain 渲染 + 主线程渲染回调 |
| Phase 4.5：SwapChain 渲染 | ✅ 已完成 | 2026-06-14 | RenderViewport（FBO → CopyTexture → SwapChain → Present）+ 5 个问题修复 |
| Phase 5：Material Preview | ⬜ 未开始 | - | 多 Surface 验证 |
| Phase 6：Mesh/Texture/Shader | ⬜ 未开始 | - | |

---

## 风险和缓解

| 风险 | 影响 | 缓解措施 |
|------|------|----------|
| NativeControlHost 行为与预期不符 | 高 | Phase 0 已验证通过 |
| NativeControlHost 重建句柄（Dock 浮动） | 高 | Surface Lost + RecreateSurface 自动重建 SwapChain |
| 多 SwapChain 共享 Device 性能问题 | 中 | 先实现单个 SwapChain，Phase 4.5 验证多 Surface |
| Resize 时 SwapChain 重建延迟 | 中 | Deferred Resize + FlushResizes 帧末统一执行 |
| HWND 获取失败 | 高 | Phase 0 已验证，CreateNativeControlCore 方案可靠 |
| Avalonia 版本升级 API 变化 | 中 | ViewportHostControl 隔离变化，只依赖 protected virtual |

---

## 参考资源

- **Avalonia NativeControlHost**：https://docs.avaloniaui.net/docs/reference/controls/native-control-host
- **Diligent SwapChain**：https://github.com/DiligentGraphics/DiligentEngine
- **NNWindowApi**：`NNNativeEngineApiTypes.cs` 已有完整窗口创建 API
- **NNDiligentDevice**：`NNDiligentDevice.cpp` 已有 SwapChain 创建逻辑

---

## Phase 4/4.5 问题修复记录

### 问题 1：SwapChain 创建失败（0x0 尺寸）

**现象**：`CreateSurface` 返回 0，日志显示 `CreateSurface 参数无效`

**根因**：`OnSurfaceCreated` 触发时，ViewModel 的 `ViewportWidth/Height` 还是 0（未初始化）

**修复**：回退到面板实际尺寸，Native 端加最小尺寸保护
- `ViewportAvaloniaView.OnSurfaceCreated`：`vmW > 0 ? vmW : panelW > 0 ? panelW : 800`
- `ViewportSurfaceRuntimeApi.cpp`：`if (width < 1) width = 1;`

### 问题 2：Diligent "outstanding commands" + "active render pass" 错误

**现象**：每次渲染都报 `There are outstanding commands` 和 `Finishing frame inside an active render pass`

**根因**：**线程问题**。Avalonia 在独立线程运行（`new Thread`），`DispatcherTimer` 在 Avalonia 线程触发。Diligent immediate context 在主线程创建，从 Avalonia 线程调用导致 D3D12 render pass 状态损坏。

**修复**：
1. `AvaloniaFrontendModule` 新增 `RegisterRenderCallback` / `TickRendering` 机制
2. `EditorApplicationRunner` 主循环中调用 `AvaloniaFrontendModule.TickRendering()`
3. `ViewportAvaloniaView` 的 `DispatcherTimer` 改为 `AvaloniaFrontendModule.RegisterRenderCallback`

**关键教训**：Diligent 的 immediate context 不是线程安全的，所有 Diligent 调用必须在创建 context 的同一线程执行。

### 问题 3：CopyTexture 前 Flush 导致 "Failed to close the command list"

**现象**：`Present()` 时报 `Debug assertion failed in CommandContext::Close()`

**根因**：`Flush()` 在 `CopyTexture` 之前调用，关闭了命令列表。`Present()` 再次尝试关闭已关闭的命令列表。

**修复**：移除 `Flush()`，`CopyTexture` + `Present` 在同一命令列表中一起提交。

### 问题 4：误判为 RenderPass 未结束（添加多余的 EndRenderPass）

**现象**：在 `RenderSceneToTexture` 返回后添加 `ctx->EndRenderPass()`，导致 `TransitionSubpassAttachments` 断言失败

**根因**：错误地认为 RmlUI 的 `EndOffscreenFrame`（`UnbindRenderTargets`）没有结束 RenderPass。实际上 D3D12 的 `SetRenderTargets(0, nullptr, nullptr)` 已正确结束 render pass 状态。真正的根本原因是线程问题（问题 2）。

**修复**：移除多余的 `EndRenderPass` 调用。修复线程问题后，RmlUI 的 render pass 生命周期正常。

**关键教训**：不要在没有确认根因的情况下添加修复。应该先验证线程模型是否正确，再考虑 render pass 生命周期。

### 修复后的工作流程

```
主线程（EditorApplicationRunner 主循环）:
  RuntimeMainLoop.Tick(deltaTime)
  AvaloniaFrontendModule.TickRendering()     ← 新增
    → ViewportAvaloniaView.OnMainThreadRender()
      → FlushResizes()                       ← Deferred Resize
      → Registry.RenderViewport(surfaceId, sceneHandle, w, h)
        → RenderSceneToTexture(sceneHandle, w, h)  ← 主线程执行
          → SceneRenderer::Render → EndRenderPass
          → RmlUI::RenderOverlayOnScene → EndOffscreenFrame
        → CopyTexture(FBO → SwapChain)
        → Present()

Avalonia 线程:
  UI 事件处理、数据绑定
  Bounds 变化 → MarkResize（只标记，不调 Diligent）
```
