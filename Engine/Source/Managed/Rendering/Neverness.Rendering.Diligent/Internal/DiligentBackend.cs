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
    /// 从已有的原生指针创建后端（不接管所有权，调用方负责生命周期）。
    /// 用于从 C++ 端获取主窗口的 Diligent 设备。
    /// </summary>
    /// <param name="devicePtr">IRenderDevice* 指针</param>
    /// <param name="contextPtr">IDeviceContext* 指针</param>
    /// <param name="swapChainPtr">ISwapChain* 指针（可选）</param>
    public DiligentBackend(IntPtr devicePtr, IntPtr contextPtr, IntPtr swapChainPtr = default)
    {
        if (devicePtr == IntPtr.Zero)
            throw new ArgumentNullException(nameof(devicePtr), "IRenderDevice 指针不能为空");
        if (contextPtr == IntPtr.Zero)
            throw new ArgumentNullException(nameof(contextPtr), "IDeviceContext 指针不能为空");

        NativeDevice = new IRenderDevice(devicePtr);
        NativeImmediateContext = new IDeviceContext(contextPtr);
        NativeSwapChain = swapChainPtr != IntPtr.Zero ? new ISwapChain(swapChainPtr) : null;
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
        // 注意：从指针构造的后端不接管所有权，不释放原生对象
        NativeSwapChain?.Dispose();
        NativeImmediateContext?.Dispose();
        NativeDevice?.Dispose();
    }
}
