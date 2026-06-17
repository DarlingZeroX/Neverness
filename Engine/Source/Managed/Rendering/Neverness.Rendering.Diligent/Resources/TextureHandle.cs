using Diligent;
using System.Collections.Concurrent;

namespace Neverness.Rendering.Diligent;

/// <summary>
/// Texture 的句柄，拥有所有 TextureView 的生命周期。
/// Dispose TextureHandle 时，所有从它获取的 TextureView 同时失效。
/// </summary>
public sealed class TextureHandle : IDisposable
{
    internal ITexture NativeObject { get; }

    private readonly ConcurrentDictionary<TextureViewType, TextureView> _defaultViews = new();

    public TextureDesc Desc => NativeObject.GetDesc();

    internal TextureHandle(ITexture nativeTexture)
    {
        NativeObject = nativeTexture;
    }

    /// <summary>
    /// 获取默认 TextureView（SRV / RTV / DSV / UAV）。
    /// 返回的 TextureView 由本 Texture 拥有，调用方不得 Dispose。
    /// 内部缓存：同一 viewType 多次调用返回同一实例。
    /// </summary>
    public TextureView GetDefaultView(TextureViewType viewType)
    {
        return _defaultViews.GetOrAdd(viewType, type =>
        {
            var nativeView = NativeObject.GetDefaultView(type);
            return new TextureView(nativeView);
        });
    }

    public void Dispose()
    {
        // 清理所有缓存的 View
        _defaultViews.Clear();
        NativeObject?.Dispose();
    }
}
