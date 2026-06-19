using System.Runtime.InteropServices;
using Diligent;
using Neverness.Rendering.Diligent.Internal;
using Neverness.Runtime.Engine;

namespace Neverness.Rendering.Diligent;

/// <summary>
/// 图形后端类型。
/// </summary>
public enum GraphicsBackend
{
    D3D12,
    Vulkan
}

/// <summary>
/// 图形设备创建信息（占位，后续扩展）。
/// </summary>
public struct GraphicsDeviceCreateInfo
{
    // TODO: 窗口句柄、适配器选择、调试标志等
}

/// <summary>
/// 渲染设备的顶层封装。
/// 持有原生 IRenderDevice、IDeviceContext（Immediate）、ISwapChain。
/// 所有资源/管线创建方法直接在设备上，调用方式：
///   var vb = device.CreateBuffer(...);
///   var pso = device.CreateGraphicsPipelineState(...);
///
/// 全局单例模式：通过 InitializePrimary() 初始化，通过 Instance 访问。
/// </summary>
public sealed class GraphicsDevice : IDisposable
{
    private readonly DiligentBackend _backend;
    private bool _disposed;

    // ═══════════════════════════════════════════
    //  静态单例
    // ═══════════════════════════════════════════

    private static GraphicsDevice? _instance;
    private static readonly object _instanceLock = new();

    /// <summary>
    /// 获取全局 GraphicsDevice 实例。
    /// 必须先调用 InitializePrimary() 或 Initialize()，否则抛出 InvalidOperationException。
    /// </summary>
    public static GraphicsDevice Instance
    {
        get
        {
            if (_instance == null)
                throw new InvalidOperationException(
                    "GraphicsDevice 尚未初始化，请先调用 GraphicsDevice.InitializePrimary() 或 GraphicsDevice.Initialize()");
            return _instance;
        }
    }

    /// <summary>是否已初始化。</summary>
    public static bool IsInitialized => _instance != null;

    private GraphicsDevice(DiligentBackend backend)
    {
        _backend = backend;
    }

    // ═══════════════════════════════════════════
    //  工厂方法
    // ═══════════════════════════════════════════

    /// <summary>
    /// 创建图形设备并设置为全局单例。
    /// </summary>
    public static GraphicsDevice Initialize(GraphicsBackend backend, in GraphicsDeviceCreateInfo createInfo = default)
    {
        lock (_instanceLock)
        {
            if (_instance != null)
                throw new InvalidOperationException("GraphicsDevice 已初始化，不可重复创建");

            var nativeBackend = DiligentBackend.Create(backend, createInfo);
            _instance = new GraphicsDevice(nativeBackend);
            return _instance;
        }
    }

    /// <summary>
    /// 从已有的原生指针创建图形设备并设置为全局单例（不接管所有权，调用方负责生命周期）。
    /// 用于从 C++ 端获取主窗口的 Diligent 设备。
    /// </summary>
    /// <param name="devicePtr">IRenderDevice* 指针</param>
    /// <param name="contextPtr">IDeviceContext* 指针</param>
    /// <param name="swapChainPtr">ISwapChain* 指针（可选）</param>
    public static GraphicsDevice InitializeFromPointers(IntPtr devicePtr, IntPtr contextPtr, IntPtr swapChainPtr = default)
    {
        lock (_instanceLock)
        {
            if (_instance != null)
                throw new InvalidOperationException("GraphicsDevice 已初始化，不可重复创建");

            var nativeBackend = new DiligentBackend(devicePtr, contextPtr, swapChainPtr);
            _instance = new GraphicsDevice(nativeBackend);
            return _instance;
        }
    }

    /// <summary>
    /// 从 NNDiligentAPI 初始化主窗口的图形设备（推荐用法）。
    /// 统一全局管理：所有需要 Diligent 设备的模块应通过此方法获取，而非自行从 NativeAPI 取指针。
    /// </summary>
    /// <returns>初始化成功返回 GraphicsDevice 实例，失败返回 null（错误输出到 stderr）。</returns>
    public static unsafe GraphicsDevice? InitializePrimary()
    {
        ref readonly var api = ref EngineNativeApiCache.EngineApi;
        if (api.LayoutVersion == 0)
        {
            Console.Error.WriteLine("[GraphicsDevice] InitializePrimary: RuntimeTable 未初始化");
            return null;
        }

        var devicePtr = api.Diligent.GetPrimaryDevice();
        var contextPtr = api.Diligent.GetPrimaryContext();
        var swapChainPtr = api.Diligent.GetPrimarySwapChain();

        if (devicePtr == null || contextPtr == null)
        {
            Console.Error.WriteLine("[GraphicsDevice] InitializePrimary: Diligent 设备未初始化");
            return null;
        }

        return InitializeFromPointers(
            new IntPtr(devicePtr),
            new IntPtr(contextPtr),
            swapChainPtr != null ? new IntPtr(swapChainPtr) : IntPtr.Zero);
    }

    // ═══════════════════════════════════════════
    //  资源创建
    // ═══════════════════════════════════════════

    public BufferHandle CreateBuffer(in BufferDesc desc, ReadOnlySpan<byte> initialData = default)
    {
        ThrowIfDisposed();
        BufferData? data = null;
        if (initialData.Length > 0)
        {
            unsafe
            {
                fixed (byte* ptr = initialData)
                {
                    var bd = new BufferData
                    {
                        Data = new IntPtr(ptr),
                        DataSize = (ulong)initialData.Length
                    };
                    data = bd;
                }
            }
        }
        var nativeBuffer = _backend.NativeDevice.CreateBuffer(desc, data);
        return new BufferHandle(nativeBuffer);
    }

    public BufferHandle CreateBuffer<T>(in BufferDesc desc, ReadOnlySpan<T> initialData) where T : unmanaged
    {
        ThrowIfDisposed();
        var nativeBuffer = _backend.NativeDevice.CreateBuffer(desc, initialData);
        return new BufferHandle(nativeBuffer);
    }

    public TextureHandle CreateTexture(in TextureDesc desc, TextureData? initialData = null)
    {
        ThrowIfDisposed();
        var nativeTexture = _backend.NativeDevice.CreateTexture(desc, initialData);
        return new TextureHandle(nativeTexture);
    }

    public SamplerHandle CreateSampler(in SamplerDesc desc)
    {
        ThrowIfDisposed();
        var nativeSampler = _backend.NativeDevice.CreateSampler(desc);
        return new SamplerHandle(nativeSampler);
    }

    // ═══════════════════════════════════════════
    //  管线创建
    // ═══════════════════════════════════════════

    public ShaderHandle CreateShader(in ShaderCreateInfo shaderCI, out string compilerOutput)
    {
        ThrowIfDisposed();
        var nativeShader = _backend.NativeDevice.CreateShader(shaderCI, out var compilerOutputBlob);
        compilerOutput = compilerOutputBlob != null
            ? Marshal.PtrToStringUTF8(compilerOutputBlob.GetConstDataPtr(0),
                (int)compilerOutputBlob.GetSize()) ?? string.Empty
            : string.Empty;
        return new ShaderHandle(nativeShader);
    }

    public PipelineStateHandle CreateGraphicsPipelineState(in GraphicsPipelineStateCreateInfo createInfo)
    {
        ThrowIfDisposed();
        var nativePSO = _backend.NativeDevice.CreateGraphicsPipelineState(createInfo);
        return new PipelineStateHandle(nativePSO);
    }

    public PipelineStateHandle CreateComputePipelineState(in ComputePipelineStateCreateInfo createInfo)
    {
        ThrowIfDisposed();
        var nativePSO = _backend.NativeDevice.CreateComputePipelineState(createInfo);
        return new PipelineStateHandle(nativePSO);
    }

    public ResourceSignatureHandle CreatePipelineResourceSignature(in PipelineResourceSignatureDesc desc)
    {
        ThrowIfDisposed();
        var nativeSig = _backend.NativeDevice.CreatePipelineResourceSignature(desc);
        return new ResourceSignatureHandle(nativeSig);
    }

    public ShaderResourceBindingHandle CreateShaderResourceBinding(PipelineStateHandle pso, bool initStaticResources = true)
    {
        ThrowIfDisposed();
        var nativeSRB = pso.NativeObject.CreateShaderResourceBinding(initStaticResources);
        return new ShaderResourceBindingHandle(nativeSRB);
    }

    // ═══════════════════════════════════════════
    //  命令录制 / SwapChain
    // ═══════════════════════════════════════════

    /// <summary>获取 Immediate Context 的命令录制接口。</summary>
    public RenderContext ImmediateContext => new(_backend.NativeImmediateContext);

    /// <summary>
    /// 将纹理从 COPY_DEST 转换为 SHADER_RESOURCE 状态。
    /// CreateTexture 带初始数据后必须调用此方法，否则纹理无法用于着色器采样。
    /// 对应 C++ 端 NNDiligentDevice::CreateTexture 中的 TransitionResourceStates 调用。
    /// </summary>
    public void TransitionTextureToShaderResource(TextureHandle texture)
    {
        ThrowIfDisposed();
        var transition = new StateTransitionDesc
        {
            Resource = texture.NativeObject,
            OldState = ResourceState.CopyDest,
            NewState = ResourceState.ShaderResource,
            Flags = StateTransitionFlags.UpdateState
        };
        _backend.NativeImmediateContext.TransitionResourceStates(new[] { transition });
    }

    /// <summary>
    /// 初始化 SwapChain（需要窗口句柄）。
    /// 必须在使用 SwapChain 属性前调用。
    /// </summary>
    public void CreateSwapChain(GraphicsBackend backend, in SwapChainDesc desc, IntPtr hWnd)
    {
        ThrowIfDisposed();
        _backend.CreateSwapChain(backend, desc, hWnd);
    }

    /// <summary>获取 SwapChain 呈现接口（需先调用 CreateSwapChain）。</summary>
    public SwapChainPresenter SwapChain
    {
        get
        {
            ThrowIfDisposed();
            if (_backend.NativeSwapChain == null)
                throw new InvalidOperationException("SwapChain 尚未创建，请先调用 CreateSwapChain()");
            return new SwapChainPresenter(_backend.NativeSwapChain);
        }
    }

    // ═══════════════════════════════════════════
    //  设备信息
    // ═══════════════════════════════════════════

    public RenderDeviceInfo DeviceInfo => _backend.NativeDevice.GetDeviceInfo();
    public GraphicsAdapterInfo AdapterInfo => _backend.NativeDevice.GetAdapterInfo();

    // ═══════════════════════════════════════════
    //  生命周期
    // ═══════════════════════════════════════════

    /// <summary>等待 GPU 空闲。</summary>
    public void IdleGPU()
    {
        ThrowIfDisposed();
        _backend.NativeDevice.IdleGPU();
    }

    /// <summary>
    /// 关闭全局单例：释放设备资源并清除 Instance。
    /// 调用后可重新调用 InitializePrimary() 创建新实例。
    /// </summary>
    public static void Shutdown()
    {
        lock (_instanceLock)
        {
            if (_instance != null)
            {
                _instance.Dispose();
                _instance = null;
            }
        }
    }

    public void Dispose()
    {
        if (_disposed) return;
        _disposed = true;
        _backend.Dispose();
    }

    private void ThrowIfDisposed()
    {
        ObjectDisposedException.ThrowIf(_disposed, this);
    }
}
