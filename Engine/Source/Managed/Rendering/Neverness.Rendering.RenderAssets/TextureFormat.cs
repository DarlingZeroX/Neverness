namespace Neverness.Rendering.RenderAssets;

/// <summary>
/// 引擎级纹理格式。
/// 不映射任何 API 特定格式，由渲染后端负责映射。
/// </summary>
public enum TextureFormat : uint
{
    Unknown = 0,

    // 8-bit unorm
    R8_UNorm,
    RG8_UNorm,
    RGB8_UNorm,
    RGBA8_UNorm,

    // 16-bit float
    R16_Float,
    RG16_Float,
    RGB16_Float,
    RGBA16_Float,

    // 32-bit float
    R32_Float,
    RG32_Float,
    RGB32_Float,
    RGBA32_Float,

    // 压缩格式（预留）
    BC1_UNorm,
    BC3_UNorm,
    BC7_UNorm,
    ETC2_RGB8,
    ETC2_RGBA8,
    ASTC_4x4,

    Count
}

/// <summary>
/// 纹理格式辅助方法。
/// </summary>
public static class TextureFormatHelper
{
    /// <summary>
    /// 获取每个纹素的字节数（未压缩格式）。
    /// </summary>
    public static uint GetBytesPerPixel(TextureFormat format)
    {
        return format switch
        {
            TextureFormat.R8_UNorm => 1,
            TextureFormat.RG8_UNorm => 2,
            TextureFormat.RGB8_UNorm => 3,
            TextureFormat.RGBA8_UNorm => 4,
            TextureFormat.R16_Float => 2,
            TextureFormat.RG16_Float => 4,
            TextureFormat.RGB16_Float => 6,
            TextureFormat.RGBA16_Float => 8,
            TextureFormat.R32_Float => 4,
            TextureFormat.RG32_Float => 8,
            TextureFormat.RGB32_Float => 12,
            TextureFormat.RGBA32_Float => 16,
            _ => 0
        };
    }

    /// <summary>
    /// 判断是否为压缩格式。
    /// </summary>
    public static bool IsCompressed(TextureFormat format)
    {
        return format switch
        {
            TextureFormat.BC1_UNorm or
            TextureFormat.BC3_UNorm or
            TextureFormat.BC7_UNorm or
            TextureFormat.ETC2_RGB8 or
            TextureFormat.ETC2_RGBA8 or
            TextureFormat.ASTC_4x4 => true,
            _ => false
        };
    }
}
