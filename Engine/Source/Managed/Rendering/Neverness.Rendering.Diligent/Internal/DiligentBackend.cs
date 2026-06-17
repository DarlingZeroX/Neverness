using Diligent;

namespace Neverness.Rendering.Diligent.Internal;

/// <summary>
/// 持有原生 Diligent 对象的内部容器。
/// 上层代码不应直接接触此类。
/// </summary>
internal sealed class DiligentBackend : IDisposable
{
    public IRenderDevice NativeDevice { get; }
    public IDeviceContext NativeImmediateContext { get; }
    public ISwapChain? NativeSwapChain { get; private set; }

    private DiligentBackend(IRenderDevice device, IDeviceContext immediateContext)
    {
        NativeDevice = device;
        NativeImmediateContext = immediateContext;
    }

    /// <summary>
    /// 创建原生设备和上下文（不含 SwapChain）。
    /// </summary>
    public static DiligentBackend Create(GraphicsBackend backend, in GraphicsDeviceCreateInfo createInfo)
    {
        var (device, contexts) = BackendFactory.CreateDevice(backend, createInfo);
        return new DiligentBackend(device, contexts[0]);
    }

    /// <summary>
    /// 创建 SwapChain（需要窗口句柄）。
    /// </summary>
    public void CreateSwapChain(GraphicsBackend backend, in SwapChainDesc desc, IntPtr hWnd)
    {
        NativeSwapChain = BackendFactory.CreateSwapChain(backend, NativeDevice, NativeImmediateContext, desc, hWnd);
    }

    public void Dispose()
    {
        // 释放顺序：SwapChain → Context → Device
        NativeSwapChain?.Dispose();
        NativeImmediateContext?.Dispose();
        NativeDevice?.Dispose();
    }
}
