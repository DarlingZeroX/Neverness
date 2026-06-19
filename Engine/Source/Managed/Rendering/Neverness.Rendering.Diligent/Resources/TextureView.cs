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

    /// <summary>
    /// 获取原生 ITextureView* 指针（IntPtr）。
    /// 用于 RenderAssetManager 的 GetImGuiHandle()，
    /// 与 C++ 端 reinterpret_cast&lt;uint64_t&gt;(ITextureView*) 等价。
    /// </summary>
    public IntPtr NativePointer => NativeObject?.NativePointer ?? IntPtr.Zero;

    internal TextureView(ITextureView nativeView)
    {
        NativeObject = nativeView;
    }
}
