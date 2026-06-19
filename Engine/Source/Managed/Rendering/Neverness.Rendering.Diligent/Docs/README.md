# Neverness.Rendering.Diligent

对 `Diligent-GraphicsEngine.NET`（SharpGen 自动生成的 COM 绑定）的薄封装层，为上层 RenderGraph / SRP 提供干净的渲染 API 入口。

## 核心设计

- **静态单例**：`GraphicsDevice.InitializePrimary()` 设置全局实例，`GraphicsDevice.Instance` 访问
- **所有 Create 方法在 GraphicsDevice 上**：`device.CreateBuffer(...)` / `device.CreateGraphicsPipelineState(...)`
- **TextureView 不是 IDisposable**：由 TextureHandle 拥有生命周期，上层只管用不管销毁
- **Map/Unmap 在 BufferHandle 上**：支持 Persistent Mapping 和 Ring Buffer 模式
- **Fence/Barrier 暂不对外**：同步 API 留在 RenderContext 的 internal 方法中，等 RenderGraph 执行层设计后决定暴露方式

## 使用示例

```csharp
using Neverness.Rendering.Diligent;

// 初始化（启动时调用一次）
GraphicsDevice.InitializePrimary();  // 从 C++ 端 NNDiligentAPI 获取主窗口设备

// 任意位置访问全局设备
var device = GraphicsDevice.Instance;

// 创建资源
var vb = device.CreateBuffer(new BufferDesc
{
    Name = "VertexBuffer",
    Usage = Usage.Immutable,
    BindFlags = BindFlags.VertexBuffer,
    Size = (ulong)(vertexData.Length * Unsafe.SizeOf<Vertex>())
}, vertexData.AsSpan());

// 创建 Shader
var vs = device.CreateShader(new ShaderCreateInfo
{
    FilePath = "shader.hlsl",
    EntryPoint = "main",
    ShaderType = ShaderType.Vertex
}, out var compilerOutput);

// 创建管线
var pso = device.CreateGraphicsPipelineState(new GraphicsPipelineStateCreateInfo { ... });

// 录制命令
var ctx = device.ImmediateContext;
ctx.SetPipelineState(pso);
ctx.Draw(new DrawAttribs { NumVertices = 3 });

// 呈现
device.SwapChain.Present();

// 关闭（退出时调用）
GraphicsDevice.Shutdown();
```

## 项目结构

```
Neverness.Rendering.Diligent/
├── GraphicsDevice.cs              // 顶层设备，所有 Create 方法
├── RenderContext.cs               // 命令录制（Draw/Dispatch/Copy/RenderPass）
├── SwapChainPresenter.cs          // SwapChain 呈现
├── Resources/
│   ├── BufferHandle.cs            // Buffer 句柄（Map/Unmap/WriteData）
│   ├── TextureHandle.cs           // Texture 句柄（拥有 TextureView）
│   ├── TextureView.cs             // TextureView（不 Dispose，由 Texture 管理）
│   └── SamplerHandle.cs           // Sampler 句柄
├── Pipeline/
│   ├── ShaderHandle.cs            // Shader 句柄
│   ├── PipelineStateHandle.cs     // PipelineState 句柄
│   ├── ResourceSignatureHandle.cs // ResourceSignature 句柄
│   └── ShaderResourceBindingHandle.cs // SRB 句柄
├── Commands/
│   ├── RenderCommandBuffer.cs     // 命令缓冲区构建器（C#→C++ Flat Buffer 序列化）
│   └── RenderCommandTypes.cs      // 命令类型、数据结构、常量（与 C++ ABI 对齐）
├── Synchronization/
│   ├── FenceHandle.cs             // Fence 句柄（internal）
│   └── QueryHandle.cs             // Query 句柄（internal）
└── Internal/
    ├── DiligentBackend.cs         // 持有原生 Diligent 对象（internal）
    ├── BackendFactory.cs          // 工厂创建（D3D12/Vulkan）
    └── MarshalHelpers.cs          // marshalling 辅助
```

## 文档索引

| 文档 | 说明 |
|------|------|
| [README.md](README.md) | 核心设计、使用示例、项目结构、边界隔离规则 |
| [API-Reference.md](API-Reference.md) | 完整 API 参考手册（18 个文件、全部公开类型和方法） |

## 边界隔离规则

- 上层禁止 `using Diligent`，只能 `using Neverness.Rendering.Diligent`
- `BufferHandle` / `TextureHandle` 等句柄类型对外，`Handle` 后缀明确引用语义
- `TextureView` 不是 IDisposable，由父 TextureHandle 拥有
- `DiligentBackend` / `BackendFactory` 等持有原生对象的类标记为 `internal`
- Desc 结构体和枚举直接 re-export
