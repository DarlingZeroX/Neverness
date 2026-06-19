# Neverness.Rendering.Diligent — API 参考手册

> 自动生成于 2026-06-19，基于源码 18 个文件。

---

## 目录

1. [顶层：GraphicsDevice](#1-graphicsdevice)
2. [命令录制：RenderContext](#2-rendercontext)
3. [SwapChain：SwapChainPresenter](#3-swapchainpresenter)
4. [资源句柄](#4-资源句柄)
   - 4.1 [BufferHandle](#41-bufferhandle)
   - 4.2 [TextureHandle](#42-texturehandle)
   - 4.3 [TextureView](#43-textureview)
   - 4.4 [SamplerHandle](#44-samplerhandle)
5. [管线句柄](#5-管线句柄)
   - 5.1 [ShaderHandle](#51-shaderhandle)
   - 5.2 [PipelineStateHandle](#52-pipelinestatehandle)
   - 5.3 [ResourceSignatureHandle](#53-resourcesignaturehandle)
   - 5.4 [ShaderResourceBindingHandle](#54-shaderresourcebindinghandle)
6. [同步句柄（internal）](#6-同步句柄)
   - 6.1 [FenceHandle](#61-fencehandle)
   - 6.2 [QueryHandle](#62-queryhandle)
7. [渲染命令系统](#7-渲染命令系统)
   - 7.1 [RenderCommandBuffer](#71-rendercommandbuffer)
   - 7.2 [RenderCommandTypes](#72-rendercommandtypes)
8. [内部实现（internal）](#8-内部实现)
   - 8.1 [DiligentBackend](#81-diligentbackend)
   - 8.2 [BackendFactory](#82-backendfactory)
   - 8.3 [MarshalHelpers](#83-marshalhelpers)
9. [枚举与常量](#9-枚举与常量)

---

## 1. GraphicsDevice

**文件**: `GraphicsDevice.cs` · **命名空间**: `Neverness.Rendering.Diligent`

渲染设备的顶层封装。持有原生 `IRenderDevice`、`IDeviceContext`（Immediate）、`ISwapChain`。

全局单例模式：通过 `InitializePrimary()` / `Initialize()` 初始化，通过 `Instance` 访问。

### 静态单例

| 成员 | 类型 | 说明 |
|------|------|------|
| `Instance` | `GraphicsDevice` | 全局实例（未初始化时抛 `InvalidOperationException`） |
| `IsInitialized` | `bool` | 是否已初始化 |
| `Shutdown()` | `void` | 释放设备并清除单例，调用后可重新初始化 |

### 初始化方法

| 方法 | 说明 |
|------|------|
| `static GraphicsDevice? InitializePrimary()` | 从 NNDiligentAPI 初始化主窗口设备并设置单例（推荐用法） |
| `static GraphicsDevice Initialize(GraphicsBackend backend, in GraphicsDeviceCreateInfo createInfo = default)` | 独立创建设备并设置单例 |
| `static GraphicsDevice InitializeFromPointers(IntPtr devicePtr, IntPtr contextPtr, IntPtr swapChainPtr = default)` | 从原生指针初始化并设置单例（不接管所有权） |

### 资源创建

| 方法 | 返回类型 |
|------|----------|
| `CreateBuffer(in BufferDesc desc, ReadOnlySpan<byte> initialData = default)` | `BufferHandle` |
| `CreateBuffer<T>(in BufferDesc desc, ReadOnlySpan<T> initialData)` | `BufferHandle` |
| `CreateTexture(in TextureDesc desc, TextureData? initialData = null)` | `TextureHandle` |
| `CreateSampler(in SamplerDesc desc)` | `SamplerHandle` |

### 管线创建

| 方法 | 返回类型 |
|------|----------|
| `CreateShader(in ShaderCreateInfo shaderCI, out string compilerOutput)` | `ShaderHandle` |
| `CreateGraphicsPipelineState(in GraphicsPipelineStateCreateInfo createInfo)` | `PipelineStateHandle` |
| `CreateComputePipelineState(in ComputePipelineStateCreateInfo createInfo)` | `PipelineStateHandle` |
| `CreatePipelineResourceSignature(in PipelineResourceSignatureDesc desc)` | `ResourceSignatureHandle` |
| `CreateShaderResourceBinding(PipelineStateHandle pso, bool initStaticResources = true)` | `ShaderResourceBindingHandle` |

### 属性

| 属性 | 类型 | 说明 |
|------|------|------|
| `ImmediateContext` | `RenderContext` | 命令录制接口 |
| `SwapChain` | `SwapChainPresenter` | SwapChain 呈现接口（需先调用 CreateSwapChain） |
| `DeviceInfo` | `RenderDeviceInfo` | 设备信息 |
| `AdapterInfo` | `GraphicsAdapterInfo` | 适配器信息 |

### 方法

| 方法 | 说明 |
|------|------|
| `CreateSwapChain(GraphicsBackend backend, in SwapChainDesc desc, IntPtr hWnd)` | 初始化 SwapChain |
| `IdleGPU()` | 等待 GPU 空闲 |
| `Dispose()` | 释放设备及所有关联资源（实例方法） |
| `Shutdown()` | 释放全局单例并清除 Instance（静态方法） |

---

## 2. RenderContext

**文件**: `RenderContext.cs` · **命名空间**: `Neverness.Rendering.Diligent`

封装 `IDeviceContext` 的命令录制 API。只负责 Draw/Dispatch/Copy/RenderPass 等命令录制。Map/Unmap 在 BufferHandle 上，不在这里。

### 管线绑定

| 方法 | 说明 |
|------|------|
| `SetPipelineState(PipelineStateHandle pso)` | 设置管线状态 |
| `CommitShaderResources(ShaderResourceBindingHandle srb, ResourceStateTransitionMode mode)` | 提交 Shader 资源 |

### 顶点/索引缓冲

| 方法 | 说明 |
|------|------|
| `SetVertexBuffers(uint startSlot, BufferHandle[] buffers, ulong[] offsets, ResourceStateTransitionMode mode)` | 设置顶点缓冲 |
| `SetIndexBuffer(BufferHandle indexBuffer, ulong byteOffset, ResourceStateTransitionMode mode)` | 设置索引缓冲 |

### 渲染目标

| 方法 | 说明 |
|------|------|
| `SetRenderTargets(TextureView[] rtvs, TextureView dsv, ResourceStateTransitionMode mode)` | 设置渲染目标 |
| `SetViewports(Viewport[] viewports, uint rtWidth, uint rtHeight)` | 设置视口 |
| `SetScissorRects(Rect[] rects, uint rtWidth, uint rtHeight)` | 设置裁剪矩形 |

### RenderPass

| 方法 | 说明 |
|------|------|
| `BeginRenderPass(in BeginRenderPassAttribs attribs)` | 开始渲染 Pass |
| `NextSubpass()` | 下一个子 Pass |
| `EndRenderPass()` | 结束渲染 Pass |

### Draw

| 方法 | 说明 |
|------|------|
| `Draw(in DrawAttribs attribs)` | 非索引绘制 |
| `DrawIndexed(in DrawIndexedAttribs attribs)` | 索引绘制 |
| `DrawIndirect(in DrawIndirectAttribs attribs)` | 间接绘制 |
| `DrawIndexedIndirect(in DrawIndexedIndirectAttribs attribs)` | 间接索引绘制 |

### Compute

| 方法 | 说明 |
|------|------|
| `DispatchCompute(in DispatchComputeAttribs attribs)` | 分派计算着色器 |

### Copy / Transfer

| 方法 | 说明 |
|------|------|
| `CopyTexture(in CopyTextureAttribs attribs)` | 复制纹理 |
| `UpdateBuffer(BufferHandle buffer, ulong offset, ulong size, IntPtr data, ResourceStateTransitionMode mode)` | 更新缓冲区数据 |

### 状态管理

| 方法 | 说明 |
|------|------|
| `InvalidateState()` | 使当前状态失效 |
| `Begin(uint immediateContextId)` | 开始命令录制 |

### Internal（为 RenderGraph 预留）

| 方法 | 说明 |
|------|------|
| `TransitionResourceStates(StateTransitionDesc[] barriers)` | 资源状态转换 |
| `EnqueueSignal(FenceHandle fence, ulong value)` | 入队 Fence 信号 |
| `DeviceWaitForFence(FenceHandle fence, ulong value)` | 设备等待 Fence |
| `WaitForIdle()` | 等待空闲 |

---

## 3. SwapChainPresenter

**文件**: `SwapChainPresenter.cs` · **命名空间**: `Neverness.Rendering.Diligent`

封装 `ISwapChain` 的呈现和窗口管理。

| 成员 | 类型 | 说明 |
|------|------|------|
| `Desc` | `SwapChainDesc` | SwapChain 描述 |
| `GetCurrentBackBufferRTV()` | `TextureView` | 获取当前后缓冲 RTV |
| `GetDepthBufferDSV()` | `TextureView` | 获取深度缓冲 DSV |
| `Present(uint syncInterval = 0)` | `void` | 呈现当前帧 |
| `Resize(uint newWidth, uint newHeight, SurfaceTransform newTransform = SurfaceTransform.Optimal)` | `void` | 调整大小 |
| `Dispose()` | `void` | 释放 |

---

## 4. 资源句柄

### 4.1 BufferHandle

**文件**: `Resources/BufferHandle.cs`

Buffer 的轻量句柄。实现 `IDisposable`，析构时调用原生 Release。

| 成员 | 说明 |
|------|------|
| `Desc` | `BufferDesc` — Buffer 描述 |
| `Map(MapType mapType, MapFlags mapFlags)` | ⚠️ **NotImplemented** — 需要通过 RenderContext 执行 |
| `Unmap()` | ⚠️ **NotImplemented** — 需要通过 RenderContext 执行 |
| `WriteData<T>(ReadOnlySpan<T> data)` | ⚠️ **NotImplemented** — 需要通过 RenderContext 执行 |
| `Dispose()` | 释放原生 Buffer |

> **注意**: Map/Unmap/WriteData 当前抛出 `NotImplementedException`，等 RenderContext 对接后实现。

### 4.2 TextureHandle

**文件**: `Resources/TextureHandle.cs`

Texture 的句柄，拥有所有 TextureView 的生命周期。Dispose TextureHandle 时，所有从它获取的 TextureView 同时失效。

| 成员 | 说明 |
|------|------|
| `Desc` | `TextureDesc` — Texture 描述 |
| `GetDefaultView(TextureViewType viewType)` | `TextureView` — 获取默认 View（SRV/RTV/DSV/UAV），内部缓存 |
| `Dispose()` | 释放 Texture 及所有缓存的 View |

### 4.3 TextureView

**文件**: `Resources/TextureView.cs`

TextureView 的轻量引用。**不实现 IDisposable** —— 生命周期由父 TextureHandle 管理。调用方只管用，不管销毁。

| 成员 | 说明 |
|------|------|
| `Desc` | `TextureViewDesc` — View 描述 |
| `NativeObject` | `ITextureView`（internal） |

### 4.4 SamplerHandle

**文件**: `Resources/SamplerHandle.cs`

| 成员 | 说明 |
|------|------|
| `Desc` | `SamplerDesc` — Sampler 描述 |
| `Dispose()` | 释放原生 Sampler |

---

## 5. 管线句柄

### 5.1 ShaderHandle

**文件**: `Pipeline/ShaderHandle.cs`

| 成员 | 说明 |
|------|------|
| `GetBytecode()` | `ReadOnlySpan<byte>` — 获取 Shader 字节码 |
| `Dispose()` | 释放原生 Shader |

### 5.2 PipelineStateHandle

**文件**: `Pipeline/PipelineStateHandle.cs`

| 成员 | 说明 |
|------|------|
| `CreateShaderResourceBinding(bool initStaticResources = true)` | `ShaderResourceBindingHandle` — 创建 SRB |
| `Dispose()` | 释放原生 PSO |

### 5.3 ResourceSignatureHandle

**文件**: `Pipeline/ResourceSignatureHandle.cs`

| 成员 | 说明 |
|------|------|
| `Dispose()` | 释放原生 ResourceSignature |

### 5.4 ShaderResourceBindingHandle

**文件**: `Pipeline/ShaderResourceBindingHandle.cs`

| 成员 | 说明 |
|------|------|
| `GetVariable(ShaderType shaderType, string name)` | `IShaderResourceVariable` — 获取 Shader 资源变量 |
| `Dispose()` | 释放原生 SRB |

---

## 6. 同步句柄

> 当前均为 `internal`，等 RenderGraph 执行层设计后决定对外暴露方式。

### 6.1 FenceHandle

**文件**: `Synchronization/FenceHandle.cs` · **访问级别**: `internal`

| 成员 | 说明 |
|------|------|
| `Desc` | `FenceDesc` |
| `NativeObject` | `IFence` |
| `Dispose()` | 释放 |

### 6.2 QueryHandle

**文件**: `Synchronization/QueryHandle.cs` · **访问级别**: `internal`

| 成员 | 说明 |
|------|------|
| `Desc` | `QueryDesc` |
| `NativeObject` | `IQuery` |
| `Dispose()` | 释放 |

---

## 7. 渲染命令系统

> C# 端序列化 RenderCommands 为 Flat Buffer，传给 C++ 端的 RenderViewportCommands。

### 7.1 RenderCommandBuffer

**文件**: `Commands/RenderCommandBuffer.cs`

命令缓冲区构建器。一次性使用：`Build()` 后不可再 Add。

```csharp
var buffer = new RenderCommandBuffer();
buffer.AddSetCamera(viewMatrix, projMatrix, width, height, near, far);
buffer.AddSetRenderPassState(clearColor, RenderPassFlags.ClearColor);
buffer.AddDrawSpriteBatch(sprites);
byte[] data = buffer.Build();
```

| 方法 | 说明 |
|------|------|
| `AddSetCamera(ReadOnlySpan<float> viewMatrix, ReadOnlySpan<float> projectionMatrix, float viewportWidth, float viewportHeight, float nearPlane, float farPlane, float orthoWidth = 0f, float orthoHeight = 0f)` | 添加 SetCamera 命令（160 bytes） |
| `AddSetRenderPassState(ReadOnlySpan<float> clearColor, RenderPassFlags flags)` | 添加 SetRenderPassState 命令（40 bytes） |
| `AddDrawSpriteBatch(ReadOnlySpan<SpriteInstance> sprites)` | 添加 DrawSpriteBatch 命令（可变大小） |
| `AddSetRmlDocuments(ReadOnlySpan<RmlDocumentEntry> entries)` | 添加 SetRmlDocuments 命令（可变大小） |
| `Build()` | `byte[]` — 构建最终缓冲区（含 16 bytes BufferHeader） |
| `CurrentDataSize` | `int` — 当前命令数据大小（不含 Header） |
| `CommandCount` | `uint` — 当前命令数量 |

### 7.2 RenderCommandTypes

**文件**: `Commands/RenderCommandTypes.cs`

#### 枚举

| 枚举 | 说明 |
|------|------|
| `RenderCommandType` | 命令类型：`SetCamera(0x01)`, `SetRenderPassState(0x02)`, `DrawSpriteBatch(0x10)`, `SetRmlDocuments(0x20)` |
| `RenderPassFlags` | 渲染 Pass 标志：`ClearColor(0x01)`, `DepthTest(0x02)`, `DepthWrite(0x04)` |
| `SpriteBlendMode` | 精灵混合模式：`Alpha(0)`, `Additive(1)`, `Multiply(2)`, `Opaque(3)`, `Premultiplied(4)` |
| `SpriteFlags` | 精灵标志：`FlipX(0x01)`, `FlipY(0x02)` |

#### 数据结构（全部 blittable，与 C++ 逐字节对齐）

| 结构体 | 大小 | 说明 |
|--------|------|------|
| `RenderCommandBufferHeader` | 16 bytes | 缓冲区头部：Magic(0x524E4443) + CommandCount + TotalBytes + Reserved |
| `RenderCommandHeader` | 8 bytes | 命令头部：Type + Size |
| `SetCameraData` | 144 bytes | View/Projection 矩阵 + 视口 + 近远平面 + 正交尺寸 |
| `RenderPassStateData` | 32 bytes | ClearColor + Flags + StencilRef + Reserved |
| `SpriteInstance` | 120 bytes | Transform(64) + TextureHandle(8) + Color(16) + UvRect(16) + Layer/SortOrder/BlendMode/Flags(16) |
| `DrawSpriteBatchHeader` | 16 bytes | SpriteCount + Reserved×3 |
| `RmlDocumentEntry` | 276 bytes | AssetPath(256) + SortOrder(4) + ViewTarget(4) + EntityHandle(4) + ViewportId(4) |
| `RmlDocumentsHeader` | 16 bytes | DocumentCount + Reserved×3 |

#### 常量

| 常量 | 值 | 说明 |
|------|----|------|
| `BufferMagic` | `0x524E4443` | "RNDC" |
| `BufferHeaderSize` | 16 | 缓冲区头部大小 |
| `CommandHeaderSize` | 8 | 命令头部大小 |
| `SetCameraTotalSize` | 160 | SetCamera 命令总大小 |
| `SetRenderPassStateTotalSize` | 40 | SetRenderPassState 命令总大小 |
| `DrawSpriteBatchHeaderSize` | 24 | DrawSpriteBatch 头部大小 |
| `SpriteInstanceSize` | 120 | 单个 Sprite 实例大小 |
| `RmlDocumentEntrySize` | 272 | 单个 RmlDocument 条目大小 |
| `SetRmlDocumentsHeaderSize` | 24 | SetRmlDocuments 头部大小 |
| `RmlDocumentPathSize` | 256 | assetPath 缓冲区大小 |

---

## 8. 内部实现

> 以下类标记为 `internal`，上层代码不应直接接触。

### 8.1 DiligentBackend

**文件**: `Internal/DiligentBackend.cs` · **访问级别**: `internal sealed class`

持有原生 Diligent 对象的内部容器。

| 成员 | 说明 |
|------|------|
| `NativeDevice` | `IRenderDevice` |
| `NativeImmediateContext` | `IDeviceContext` |
| `NativeSwapChain` | `ISwapChain?`（可空） |
| `DiligentBackend(IntPtr devicePtr, IntPtr contextPtr, IntPtr swapChainPtr = default)` | 从原生指针构造（不接管所有权） |
| `static DiligentBackend Create(GraphicsBackend backend, in GraphicsDeviceCreateInfo createInfo)` | 创建设备和上下文 |
| `CreateSwapChain(GraphicsBackend backend, in SwapChainDesc desc, IntPtr hWnd)` | 创建 SwapChain |
| `Dispose()` | 释放顺序：SwapChain → Context → Device |

### 8.2 BackendFactory

**文件**: `Internal/BackendFactory.cs` · **访问级别**: `internal static class`

| 方法 | 说明 |
|------|------|
| `static (IRenderDevice device, IDeviceContext[] contexts) CreateDevice(GraphicsBackend backend, in GraphicsDeviceCreateInfo createInfo)` | 创建设备和上下文（不含 SwapChain） |
| `static ISwapChain CreateSwapChain(GraphicsBackend backend, IRenderDevice device, IDeviceContext immediateContext, in SwapChainDesc swapChainDesc, IntPtr hWnd)` | 创建 SwapChain |

支持 D3D12 和 Vulkan 两种后端。

### 8.3 MarshalHelpers

**文件**: `Internal/MarshalHelpers.cs` · **访问级别**: `internal static class`

| 方法 | 说明 |
|------|------|
| `static unsafe IntPtr ToUnmanagedPtr<T>(ReadOnlySpan<T> span)` | 将托管 span 转换为非托管指针（stackalloc 场景） |

---

## 9. 枚举与常量

### GraphicsBackend

```csharp
public enum GraphicsBackend { D3D12, Vulkan }
```

### RenderCommandType

| 值 | 名称 | 说明 |
|----|------|------|
| `0x01` | `SetCamera` | 设置相机 |
| `0x02` | `SetRenderPassState` | 设置渲染 Pass 状态 |
| `0x10` | `DrawSpriteBatch` | 批量精灵绘制 |
| `0x20` | `SetRmlDocuments` | 设置 RmlUI 文档列表 |

### RenderPassFlags

| 值 | 名称 |
|----|------|
| `0x01` | `ClearColor` |
| `0x02` | `DepthTest` |
| `0x04` | `DepthWrite` |

### SpriteBlendMode

| 值 | 名称 |
|----|------|
| `0` | `Alpha` |
| `1` | `Additive` |
| `2` | `Multiply` |
| `3` | `Opaque` |
| `4` | `Premultiplied` |

### SpriteFlags

| 值 | 名称 |
|----|------|
| `0x01` | `FlipX` |
| `0x02` | `FlipY` |
