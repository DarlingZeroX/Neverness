namespace Neverness.Rendering.RenderAssets;

/// <summary>
/// 渲染资源工厂接口。
/// 负责 GPU 资源的创建和更新，AssetManager 通过此接口与渲染后端解耦。
/// </summary>
public interface IRenderResourceFactory
{
    /// <summary>
    /// 创建 GPU 纹理。
    /// </summary>
    /// <param name="width">纹理宽度</param>
    /// <param name="height">纹理高度</param>
    /// <param name="format">纹理格式</param>
    /// <param name="pixels">像素数据</param>
    /// <param name="isSRGB">是否为 sRGB 纹理</param>
    /// <returns>纹理资源（RHI Texture + SRV 已填充），失败返回 null</returns>
    TextureResource? CreateTexture(uint width, uint height, TextureFormat format,
                                   ReadOnlySpan<byte> pixels, bool isSRGB);

    /// <summary>
    /// 更新已存在纹理的像素数据。
    /// </summary>
    /// <param name="resource">目标纹理资源</param>
    /// <param name="pixels">新的像素数据</param>
    /// <returns>是否成功</returns>
    bool UpdateTexturePixels(TextureResource resource, ReadOnlySpan<byte> pixels);
}
