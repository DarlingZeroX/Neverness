using System.Runtime.InteropServices;
using Diligent;
using Neverness.Rendering.Diligent.Internal;

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
/// </summary>
public sealed class GraphicsDevice : IDisposable
{
    private readonly DiligentBackend _backend;
    private bool _disposed;

    private GraphicsDevice(DiligentBackend backend)
    {
        _backend = backend;
    }

    // ═══════════════════════════════════════════
    //  工厂方法
    // ═══════════════════════════════════════════

    /// <summary>
    /// 创建图形设备。
    /// </summary>
    public static GraphicsDevice Create(GraphicsBackend backend, in GraphicsDeviceCreateInfo createInfo = default)
    {
        var nativeBackend = DiligentBackend.Create(backend, createInfo);
        return new GraphicsDevice(nativeBackend);
    }

    /// <summary>
    /// 从已有的原生指针创建图形设备（不接管所有权，调用方负责生命周期）。
    /// 用于从 C++ 端获取主窗口的 Diligent 设备。
    /// </summary>
    /// <param name="devicePtr">IRenderDevice* 指针</param>
    /// <param name="contextPtr">IDeviceContext* 指针</param>
    /// <param name="swapChainPtr">ISwapChain* 指针（可选）</param>
    public static GraphicsDevice FromNativePointers(IntPtr devicePtr, IntPtr contextPtr, IntPtr swapChainPtr = default)
    {
        var nativeBackend = new DiligentBackend(devicePtr, contextPtr, swapChainPtr);
        return new GraphicsDevice(nativeBackend);
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
