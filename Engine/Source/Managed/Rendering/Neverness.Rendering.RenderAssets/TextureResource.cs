using Neverness.Rendering.Diligent;

namespace Neverness.Rendering.RenderAssets;

/// <summary>
/// Residency 状态（Streaming 预留）。
/// </summary>
public enum TextureResidency : byte
{
    NotLoaded = 0,
    Loading,
    Resident,
    Evicted,
    Error
}

/// <summary>
/// GPU 纹理资源缓存描述。
/// </summary>
public struct TextureCacheDesc
{
    public uint Width;
    public uint Height;
    public uint MipCount;
    public TextureFormat Format;
    public bool IsSRGB;

    public TextureCacheDesc()
    {
        Width = 0;
        Height = 0;
        MipCount = 1;
        Format = TextureFormat.RGBA8_UNorm;
        IsSRGB = false;
    }
}

/// <summary>
/// GPU 纹理资源。
/// 封装 RHI Texture，不暴露原生 Handle 类型。
/// </summary>
public sealed class TextureResource : IDisposable
{
    private TextureCacheDesc _desc;
    private TextureHandle? _rhiTexture;
    private TextureView? _rhiShaderResourceView;
    private ulong _lastUsedFrame;
    private TextureResidency _residency = TextureResidency.NotLoaded;
    private bool _disposed;

    /// <summary>纹理缓存描述。</summary>
    public ref TextureCacheDesc Desc => ref _desc;

    /// <summary>Residency 状态。</summary>
    public TextureResidency Residency
    {
        get => _residency;
        internal set => _residency = value;
    }

    /// <summary>最后使用的帧号。</summary>
    public ulong LastUsedFrame
    {
        get => _lastUsedFrame;
        internal set => _lastUsedFrame = value;
    }

    /// <summary>是否已驻留。</summary>
    public bool IsResident => _residency == TextureResidency.Resident;

    /// <summary>
    /// 获取内部 RHI Texture（仅内部使用）。
    /// </summary>
    internal TextureHandle? RHITexture
    {
        get => _rhiTexture;
        set => _rhiTexture = value;
    }

    /// <summary>
    /// 获取 ImGui 兼容的 Texture Handle。
    /// </summary>
    public ulong GetImGuiHandle()
    {
        if (_rhiShaderResourceView == null)
            return 0;

        // 使用 SRV 的原生指针作为 ImGui Handle
        // TextureView 内部持有 ITextureView，需要通过反射获取原生指针
        // 或者使用 TextureHandle 的 GetDefaultView 返回的 TextureView
        // 这里暂时返回 0，需要与 Neverness.Rendering.Diligent 配合
        return 0;
    }

    /// <summary>
    /// 设置 SRV（由工厂在创建时调用）。
    /// </summary>
    internal void SetShaderResourceView(TextureView srv)
    {
        _rhiShaderResourceView = srv;
    }

    /// <summary>
    /// 释放 GPU 资源。
    /// </summary>
    public void ReleaseGPU()
    {
        _rhiTexture?.Dispose();
        _rhiTexture = null;
        _rhiShaderResourceView = null;
        _residency = TextureResidency.NotLoaded;
    }

    public void Dispose()
    {
        if (_disposed) return;
        _disposed = true;
        ReleaseGPU();
    }
}
