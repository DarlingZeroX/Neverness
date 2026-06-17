# Neverness.Rendering.Diligent 模块计划

## 1. 目标

在 `Engine/Source/Managed/Rendering/` 下新建 `Neverness.Rendering.Diligent` 模块，封装底层 `Diligent-GraphicsEngine.NET`（SharpGen 自动生成的 COM 绑定），为上层 RenderGraph / SRP 提供干净的渲染 API 入口。

### 核心约束

- **薄封装层**：保留 Diligent 原生概念（RenderDevice / PipelineState / Buffer 等），不做高层抽象
- **隔离边界**：所有 `Diligent.XXX` 类型必须限制在本程序集内部，上层只能看到本模块暴露的接口/类型
- **为未来留门**：接口设计预留未来提升为 `IRenderDevice` / `IRenderContext` 抽象的空间，届时 Diligent 实现退到 `internal`，上层无需改动
- **后端**：仅支持 D3D12 + Vulkan

---

## 2. 项目结构

```
Engine/Source/Managed/Rendering/Neverness.Rendering.Diligent/
├── Neverness.Rendering.Diligent.csproj
├── GraphicsDevice.cs              // 封装 IRenderDevice，所有 Create 方法直接在设备上
├── RenderContext.cs               // 封装 IDeviceContext 的命令录制 API
├── SwapChainPresenter.cs          // 封装 ISwapChain 的 Present / Resize
├── Resources/
│   ├── BufferHandle.cs            // 封装 IBuffer，提供 Map/Unmap + IDisposable
│   ├── TextureHandle.cs           // 封装 ITexture，拥有所有 TextureView 的生命周期
│   ├── TextureView.cs             // 封装 ITextureView，轻量引用类型，不实现 IDisposable
│   ├── SamplerHandle.cs           // 封装 ISampler
│   └── UploadHelper.cs            // 便利方法：Ring Buffer / Persistent Mapping 辅助（Phase 2+）
├── Pipeline/
│   ├── PipelineStateHandle.cs     // 封装 IPipelineState
│   ├── ShaderHandle.cs            // 封装 IShader
│   ├── ResourceSignatureHandle.cs // 封装 IPipelineResourceSignature
│   └── ShaderResourceBindingHandle.cs // 封装 IShaderResourceBinding
├── Synchronization/
│   ├── FenceHandle.cs             // 封装 IFence（当前 internal）
│   └── QueryHandle.cs             // 封装 IQuery（当前 internal）
├── Internal/
│   ├── DiligentBackend.cs         // 持有原生 Diligent 对象，internal 可见性
│   ├── BackendFactory.cs          // 封装 IEngineFactory 的创建（D3D12 / Vulkan）
│   └── MarshalHelpers.cs          // 结构体 marshalling 辅助
└── Docs/
    └── README.md
```

**相比旧版的变化：**
- 删除 `ResourceFactory.cs` — Create 方法直接放在 GraphicsDevice 上
- 删除 `PipelineFactory.cs` — 同上
- `TextureViewHandle.cs` → `TextureView.cs` — 不再是 IDisposable，由 Texture 拥有
- 新增 `UploadHelper.cs` — 为 Ring Buffer / Persistent Mapping 预留

---

## 3. 核心类型设计

### 3.1 GraphicsDevice（对外入口 + 所有 Create 方法）

```csharp
namespace Neverness.Rendering.Diligent;

/// <summary>
/// 渲染设备的顶层封装。
/// 持有原生 IRenderDevice、IDeviceContext（Immediate）、ISwapChain。
/// 所有资源/管线创建方法直接在设备上，调用方式：
///   var vb = device.CreateBuffer(...);
///   var pso = device.CreateGraphicsPipelineState(...);
/// </summary>
public sealed class GraphicsDevice : IDisposable
{
    // ── 工厂方法 ──
    public static GraphicsDevice Create(GraphicsBackend backend, in GraphicsDeviceCreateInfo createInfo);

    // ── 资源创建 ──
    public BufferHandle CreateBuffer(in BufferDesc desc, ReadOnlySpan<byte> initialData = default);
    public BufferHandle CreateBuffer<T>(in BufferDesc desc, ReadOnlySpan<T> initialData) where T : unmanaged;
    public TextureHandle CreateTexture(in TextureDesc desc, TextureData? initialData = null);
    public SamplerHandle CreateSampler(in SamplerDesc desc);

    // ── 管线创建 ──
    public ShaderHandle CreateShader(in ShaderCreateInfo shaderCI, out string compilerOutput);
    public PipelineStateHandle CreateGraphicsPipelineState(in GraphicsPipelineStateCreateInfo createInfo);
    public PipelineStateHandle CreateComputePipelineState(in ComputePipelineStateCreateInfo createInfo);
    public ResourceSignatureHandle CreatePipelineResourceSignature(in PipelineResourceSignatureDesc desc);
    public ShaderResourceBindingHandle CreateShaderResourceBinding(PipelineStateHandle pso, bool initStaticResources = true);

    // ── 命令录制 ──
    public RenderContext ImmediateContext { get; }

    // ── SwapChain ──
    public SwapChainPresenter SwapChain { get; }

    // ── 设备信息 ──
    public RenderDeviceInfo DeviceInfo { get; }
    public GraphicsAdapterInfo AdapterInfo { get; }

    // ── 生命周期 ──
    public void IdleGPU();
    public void Dispose();
}
```

### 3.2 RenderContext（命令录制）

```csharp
/// <summary>
/// 封装 IDeviceContext 的命令录制 API。
/// 只负责 Draw / Dispatch / Copy / RenderPass 等命令录制。
/// Map/Unmap 在 BufferHandle 上，不在这里。
/// </summary>
public sealed class RenderContext
{
    // ── 管线绑定 ──
    public void SetPipelineState(PipelineStateHandle pso);
    public void CommitShaderResources(ShaderResourceBindingHandle srb, ResourceStateTransitionMode mode);

    // ── 顶点/索引缓冲 ──
    public void SetVertexBuffers(uint startSlot, BufferHandle[] buffers, ulong[] offsets, ResourceStateTransitionMode mode);
    public void SetIndexBuffer(BufferHandle indexBuffer, ulong byteOffset, ResourceStateTransitionMode mode);

    // ── 渲染目标 ──
    public void SetRenderTargets(TextureView[] rtvs, TextureView dsv, ResourceStateTransitionMode mode);
    public void SetViewports(Viewport[] viewports, uint rtWidth, uint rtHeight);
    public void SetScissorRects(Rect[] rects, uint rtWidth, uint rtHeight);

    // ── RenderPass ──
    public void BeginRenderPass(in BeginRenderPassAttribs attribs);
    public void NextSubpass();
    public void EndRenderPass();

    // ── Draw ──
    public void Draw(in DrawAttribs attribs);
    public void DrawIndexed(in DrawIndexedAttribs attribs);
    public void DrawIndirect(in DrawIndirectAttribs attribs);
    public void DrawIndexedIndirect(in DrawIndexedIndirectAttribs attribs);

    // ── Compute ──
    public void DispatchCompute(in DispatchComputeAttribs attribs);

    // ── Copy / Transfer ──
    public void CopyTexture(in CopyTextureAttribs attribs);
    public void UpdateBuffer(BufferHandle buffer, ulong offset, ulong size, IntPtr data, ResourceStateTransitionMode mode);

    // ── 状态管理 ──
    public void InvalidateState();
    public void Begin(uint immediateContextId);

    // ── internal: Fence / Barrier / Sync（为 RenderGraph 执行层预留，暂不对外暴露） ──
    internal void TransitionResourceStates(ResourceStateTransitionBarrier[] barriers);
    internal void EnqueueSignal(FenceHandle fence, ulong value);
    internal ulong GetCompletedFenceValue(FenceHandle fence);
    internal bool WaitForFence(FenceHandle fence, ulong value, bool waitForCompletion);
    internal void WaitForIdle();
}
```

### 3.3 BufferHandle（资源句柄 + Map/Unmap）

```csharp
/// <summary>
/// Buffer 的轻量句柄。
/// 实现 IDisposable，析构时调用原生 Release。
/// 提供 Map/Unmap，支持 Persistent Mapping 和 Ring Buffer 模式。
/// </summary>
public sealed class BufferHandle : IDisposable
{
    public BufferDesc Desc { get; }

    /// <summary>映射缓冲区到 CPU 可访问内存。</summary>
    public IntPtr Map(MapType mapType, MapFlags mapFlags);

    /// <summary>取消映射。</summary>
    public void Unmap();

    /// <summary>便利方法：映射后写入数据。</summary>
    public unsafe void WriteData<T>(ReadOnlySpan<T> data) where T : unmanaged;

    public void Dispose();
}
```

**Map/Unmap 设计理由：**
- D3D12/Vulkan 世界里，Persistent Mapping + Ring Buffer 是标准模式
- `buffer.Map()` 比 `context.MapBuffer(buffer)` 更自然，语义上是"访问这个资源"
- 上层做 Upload 时：`buffer.WriteData(vertices)` 一行搞定
- 为未来 `UploadBuffer` / `RingBuffer` 高层抽象留门

### 3.4 TextureHandle + TextureView（所有权分离）

```csharp
/// <summary>
/// Texture 的句柄，拥有所有 TextureView 的生命周期。
/// Dispose TextureHandle 时，所有从它获取的 TextureView 同时失效。
/// </summary>
public sealed class TextureHandle : IDisposable
{
    public TextureDesc Desc { get; }

    /// <summary>
    /// 获取默认 TextureView（SRV / RTV / DSV / UAV）。
    /// 返回的 TextureView 由本 Texture 拥有，调用方不得 Dispose。
    /// 内部缓存：同一 viewType 多次调用返回同一实例。
    /// </summary>
    public TextureView GetDefaultView(TextureViewType viewType);

    public void Dispose();
}

/// <summary>
/// TextureView 的轻量引用。
/// 不实现 IDisposable —— 生命周期由父 TextureHandle 管理。
/// 调用方只管用，不管销毁。
/// </summary>
public sealed class TextureView
{
    public TextureViewDesc Desc { get; }
    // 注意：没有 Dispose() 方法
}
```

**TextureView 不是 IDisposable 的理由：**
- Diligent 的 `ITexture::GetDefaultView()` 返回的是内部缓存对象
- 如果上层 `tex.GetDefaultView(SRV).Dispose()` → 释放了 Texture 内部的 View → 后续访问炸掉
- View 的生命周期绑定到 Texture，Texture 销毁时 View 自然失效
- 上层代码：`context.SetRenderTargets(new[] { tex.GetDefaultView(RTV) }, tex.GetDefaultView(DSV), ...)` — 不用操心释放

### 3.5 SamplerHandle

```csharp
public sealed class SamplerHandle : IDisposable
{
    public SamplerDesc Desc { get; }
    public void Dispose();
}
```

### 3.6 SwapChainPresenter

```csharp
/// <summary>
/// 封装 ISwapChain 的呈现和窗口管理。
/// </summary>
public sealed class SwapChainPresenter : IDisposable
{
    public SwapChainDesc Desc { get; }
    public TextureView GetCurrentBackBufferRTV();  // 返回 TextureView（不 Dispose）
    public TextureView GetDepthBufferDSV();         // 返回 TextureView（不 Dispose）
    public void Present(uint syncInterval = 0);
    public void Resize(uint newWidth, uint newHeight, SurfaceTransform newTransform = SurfaceTransform.Optimal);
    public void Dispose();
}
```

---

## 4. 内部实现（Internal / 不对外暴露）

### 4.1 DiligentBackend

```csharp
internal sealed class DiligentBackend : IDisposable
{
    public Diligent.IRenderDevice NativeDevice { get; }
    public Diligent.IDeviceContext NativeImmediateContext { get; }
    public Diligent.ISwapChain NativeSwapChain { get; }

    public static DiligentBackend Create(GraphicsBackend backend, in GraphicsDeviceCreateInfo createInfo);
    public void Dispose();
}
```

### 4.2 BackendFactory

```csharp
internal static class BackendFactory
{
    public static (Diligent.IRenderDevice device, Diligent.IDeviceContext[] contexts, Diligent.ISwapChain swapChain)
        CreateDeviceAndSwapChain(GraphicsBackend backend, in GraphicsDeviceCreateInfo createInfo);
}
```

---

## 5. 依赖关系

```
Neverness.Rendering.Diligent
  └── Diligent-GraphicsEngine.NET   (项目引用，SharpGen 绑定)
        └── Diligent-SharpGen.Runtime (NuGet)
```

### csproj 草案

```xml
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <AssemblyName>Neverness.Rendering.Diligent</AssemblyName>
    <TargetFramework>net10.0</TargetFramework>
    <RootNamespace>Neverness.Rendering.Diligent</RootNamespace>
    <Nullable>enable</Nullable>
    <ImplicitUsings>enable</ImplicitUsings>
    <AllowUnsafeBlocks>true</AllowUnsafeBlocks>
  </PropertyGroup>
  <ItemGroup>
    <ProjectReference Include="../Diligent-GraphicsEngine.NET/Diligent-GraphicsEngine.NET.csproj" />
  </ItemGroup>
</Project>
```

---

## 6. 边界隔离规则

| 规则 | 说明 |
|------|------|
| **上层禁止 using Diligent** | RenderGraph / SRP 只能 `using Neverness.Rendering.Diligent` |
| **句柄类型对外** | `BufferHandle` / `TextureHandle` / `PipelineStateHandle` 等，`Handle` 后缀明确表达"引用语义，不拥有数据"，与 `NNEntityHandle` 命名风格对齐 |
| **TextureView 不是句柄** | `TextureView` 不实现 `IDisposable`，由父 `TextureHandle` 拥有生命周期。上层只管用，不管销毁 |
| **Diligent 类型 internal** | `DiligentBackend`、`BackendFactory` 等持有原生对象的类标记为 `internal` |
| **Desc / 枚举 re-export** | `BufferDesc` / `TextureDesc` / `TextureFormat` / `ResourceState` 等纯数据结构和枚举，直接 re-export。Desc 是纯数据无行为，换后端时批量搜索替换即可 |
| **Fence / Barrier 暂不对外** | 同步 API 先留在 `RenderContext` 的 `internal` 方法中，等 RenderGraph 执行层设计清晰后再决定哪些需要暴露 |
| **RayTracing 暂不覆盖** | BLAS / TLAS / SBT 接口复杂度高，等 Graphics+Compute 主线跑通后单独加命名空间 |
| **Create 方法在设备上** | 不设 ResourceFactory / PipelineFactory，所有 Create 直接在 `GraphicsDevice` 上，调用方式：`device.CreateBuffer(...)` |
| **Map/Unmap 在 Buffer 上** | 不在 RenderContext 上做 MapBuffer，而是 `buffer.Map()` / `buffer.Unmap()`，支持 Persistent Mapping 和 Ring Buffer 模式 |

---

## 7. 实施阶段

### Phase 0：项目脚手架
- 创建 `Neverness.Rendering.Diligent.csproj`
- 建立目录结构
- 引用 `Diligent-GraphicsEngine.NET`
- 确保编译通过

### Phase 1：GraphicsDevice + BackendFactory
- 实现 `DiligentBackend`（internal，持有原生对象）
- 实现 `BackendFactory`（D3D12 / Vulkan 工厂创建）
- 实现 `GraphicsDevice.Create()` 入口
- 实现 `GraphicsDevice.Dispose()` 生命周期管理

### Phase 2：资源句柄（Buffer / Texture / TextureView / Sampler）
- 实现 `BufferHandle`（含 Map / Unmap / WriteData / Dispose）
- 实现 `TextureHandle`（含 GetDefaultView 缓存 / Dispose）
- 实现 `TextureView`（轻量引用，无 IDisposable）
- 实现 `SamplerHandle`
- 实现 `GraphicsDevice` 上的 CreateBuffer / CreateTexture / CreateSampler

### Phase 3：管线句柄
- 实现 `ShaderHandle` / `PipelineStateHandle` / `ResourceSignatureHandle` / `ShaderResourceBindingHandle`
- 实现 `GraphicsDevice` 上的 CreateShader / CreateGraphicsPipelineState / CreateComputePipelineState / CreateSRB

### Phase 4：RenderContext（命令录制）
- 实现 `RenderContext` 封装 `IDeviceContext`
- Draw / Dispatch / Copy 等命令录制（公开）
- RenderPass 系列 API（公开）
- Fence / Barrier / Sync API（internal，为 RenderGraph 执行层预留）

### Phase 5：SwapChainPresenter
- 实现 `SwapChainPresenter` 封装 `ISwapChain`
- Present / Resize / GetBackBuffer

### Phase 6：枚举 re-export + 文档
- 常用枚举和 Desc 结构体的类型转发
- README.md 编写
