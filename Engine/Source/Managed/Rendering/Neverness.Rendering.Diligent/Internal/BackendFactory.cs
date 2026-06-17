using Diligent;

namespace Neverness.Rendering.Diligent.Internal;

/// <summary>
/// 根据后端类型创建 Diligent 原生设备、上下文和交换链。
/// </summary>
internal static class BackendFactory
{
    /// <summary>
    /// 创建设备和上下文（不创建 SwapChain）。
    /// SwapChain 需要窗口句柄，后续单独创建。
    /// </summary>
    public static (IRenderDevice device, IDeviceContext[] contexts)
        CreateDevice(GraphicsBackend backend, in GraphicsDeviceCreateInfo createInfo)
    {
        return backend switch
        {
            GraphicsBackend.D3D12 => CreateD3D12(createInfo),
            GraphicsBackend.Vulkan => CreateVulkan(createInfo),
            _ => throw new NotSupportedException($"不支持的图形后端: {backend}")
        };
    }

    /// <summary>
    /// 创建 SwapChain（需要设备和窗口句柄）。
    /// </summary>
    public static ISwapChain CreateSwapChain(GraphicsBackend backend, IRenderDevice device,
        IDeviceContext immediateContext, in SwapChainDesc swapChainDesc, IntPtr hWnd)
    {
        return backend switch
        {
            GraphicsBackend.D3D12 => CreateSwapChainD3D12(device, immediateContext, swapChainDesc, hWnd),
            GraphicsBackend.Vulkan => CreateSwapChainVk(device, immediateContext, swapChainDesc, hWnd),
            _ => throw new NotSupportedException($"不支持的图形后端: {backend}")
        };
    }

    private static (IRenderDevice, IDeviceContext[]) CreateD3D12(in GraphicsDeviceCreateInfo createInfo)
    {
        var factory = Native.CreateEngineFactory<IEngineFactoryD3D12>()
            ?? throw new InvalidOperationException("无法创建 D3D12 EngineFactory");

        var d3d12CI = new EngineD3D12CreateInfo
        {
            // TODO: 从 GraphicsDeviceCreateInfo 映射
        };

        factory.CreateDeviceAndContextsD3D12(d3d12CI, out var device, out var contexts);
        return (device, contexts);
    }

    private static (IRenderDevice, IDeviceContext[]) CreateVulkan(in GraphicsDeviceCreateInfo createInfo)
    {
        var factory = Native.CreateEngineFactory<IEngineFactoryVk>()
            ?? throw new InvalidOperationException("无法创建 Vulkan EngineFactory");

        var vkCI = new EngineVkCreateInfo
        {
            // TODO: 从 GraphicsDeviceCreateInfo 映射
        };

        factory.CreateDeviceAndContextsVk(vkCI, out var device, out var contexts);
        return (device, contexts);
    }

    private static ISwapChain CreateSwapChainD3D12(IRenderDevice device,
        IDeviceContext immediateContext, in SwapChainDesc swapChainDesc, IntPtr hWnd)
    {
        var factory = device.GetEngineFactory();
        var d3d12Factory = (IEngineFactoryD3D12)factory;
        var window = new Win32NativeWindow { Wnd = hWnd };
        return d3d12Factory.CreateSwapChainD3D12(device, immediateContext, swapChainDesc,
            new FullScreenModeDesc(), window);
    }

    private static ISwapChain CreateSwapChainVk(IRenderDevice device,
        IDeviceContext immediateContext, in SwapChainDesc swapChainDesc, IntPtr hWnd)
    {
        var factory = device.GetEngineFactory();
        var vkFactory = (IEngineFactoryVk)factory;
        var window = new Win32NativeWindow { Wnd = hWnd };
        return vkFactory.CreateSwapChainVk(device, immediateContext, swapChainDesc, window);
    }
}
