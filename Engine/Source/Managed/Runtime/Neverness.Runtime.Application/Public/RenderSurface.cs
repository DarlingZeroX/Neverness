// Neverness.Runtime.Application — 渲染表面封装。
// 分离 Window 和 SwapChain，支持多窗口编辑器。

namespace Neverness.Runtime.Application.Public;

/// <summary>
/// 渲染表面，持有 Diligent Surface 和 SwapChain。
/// 每个窗口可有独立的 RenderSurface。
/// </summary>
public sealed class RenderSurface : IDisposable
{
    private bool _disposed;

    /// <summary>Diligent ISurface* 原生指针。</summary>
    public IntPtr Surface { get; }

    /// <summary>Diligent ISwapChain* 原生指针。</summary>
    public IntPtr SwapChain { get; }

    /// <summary>Diligent IDeviceContext* 原生指针。</summary>
    public IntPtr Context { get; internal set; }

    /// <summary>关联的窗口句柄。</summary>
    public WindowHandle WindowHandle { get; }

    /// <summary>是否有效。</summary>
    public bool IsValid => !_disposed && Surface != IntPtr.Zero;

    internal RenderSurface(IntPtr surface, IntPtr swapChain, WindowHandle windowHandle)
    {
        Surface = surface;
        SwapChain = swapChain;
        WindowHandle = windowHandle;
    }

    /// <summary>调整 SwapChain 大小。</summary>
    public void Resize(int width, int height)
    {
        if (_disposed || SwapChain == IntPtr.Zero)
        {
            return;
        }

        // 通过 DiligentEngine.NET 的 SwapChain.Resize 调用
        // 具体实现依赖 Diligent 绑定层
    }

    /// <summary>释放资源。</summary>
    public void Dispose()
    {
        if (_disposed)
        {
            return;
        }

        _disposed = true;

        // SwapChain 和 Surface 的释放由 Diligent 引用计数管理
        GC.SuppressFinalize(this);
    }
}
