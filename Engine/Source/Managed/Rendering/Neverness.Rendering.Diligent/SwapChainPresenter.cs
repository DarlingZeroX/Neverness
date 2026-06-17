using Diligent;

namespace Neverness.Rendering.Diligent;

/// <summary>
/// 封装 ISwapChain 的呈现和窗口管理。
/// </summary>
public sealed class SwapChainPresenter : IDisposable
{
    internal ISwapChain NativeObject { get; }

    public SwapChainDesc Desc => NativeObject.GetDesc();

    internal SwapChainPresenter(ISwapChain nativeSwapChain)
    {
        NativeObject = nativeSwapChain;
    }

    /// <summary>获取当前后缓冲的 RTV。</summary>
    public TextureView GetCurrentBackBufferRTV()
    {
        var nativeView = NativeObject.GetCurrentBackBufferRTV();
        return new TextureView(nativeView);
    }

    /// <summary>获取深度缓冲的 DSV。</summary>
    public TextureView GetDepthBufferDSV()
    {
        var nativeView = NativeObject.GetDepthBufferDSV();
        return new TextureView(nativeView);
    }

    /// <summary>呈现当前帧。</summary>
    public void Present(uint syncInterval = 0)
    {
        NativeObject.Present(syncInterval);
    }

    /// <summary>调整交换链大小。</summary>
    public void Resize(uint newWidth, uint newHeight, SurfaceTransform newTransform = SurfaceTransform.Optimal)
    {
        NativeObject.Resize(newWidth, newHeight, newTransform);
    }

    public void Dispose()
    {
        NativeObject?.Dispose();
    }
}
