using Diligent;

namespace Neverness.Rendering.Diligent;

/// <summary>
/// TextureView 的轻量引用。
/// 不实现 IDisposable —— 生命周期由父 TextureHandle 管理。
/// 调用方只管用，不管销毁。
/// </summary>
public sealed class TextureView
{
    internal ITextureView NativeObject { get; }

    public TextureViewDesc Desc => NativeObject.GetDesc();

    internal TextureView(ITextureView nativeView)
    {
        NativeObject = nativeView;
    }
}
