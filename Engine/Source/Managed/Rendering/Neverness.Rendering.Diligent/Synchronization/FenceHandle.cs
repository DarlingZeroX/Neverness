using Diligent;

namespace Neverness.Rendering.Diligent;

/// <summary>
/// Fence 的句柄（当前 internal，等 RenderGraph 执行层设计后决定对外暴露方式）。
/// </summary>
internal sealed class FenceHandle : IDisposable
{
    public IFence NativeObject { get; }

    public FenceDesc Desc => NativeObject.GetDesc();

    internal FenceHandle(IFence nativeFence)
    {
        NativeObject = nativeFence;
    }

    public void Dispose()
    {
        NativeObject?.Dispose();
    }
}
