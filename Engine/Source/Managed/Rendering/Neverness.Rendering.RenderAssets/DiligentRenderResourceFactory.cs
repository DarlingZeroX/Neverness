using Diligent;
using Neverness.Rendering.Diligent;
using TextureFormatDiligent = Diligent.TextureFormat;

namespace Neverness.Rendering.RenderAssets;

/// <summary>
/// Diligent 渲染资源工厂。
/// 通过 GraphicsDevice 创建 GPU 纹理。
/// </summary>
public sealed class DiligentRenderResourceFactory : IRenderResourceFactory
{
    private readonly GraphicsDevice _device;

    /// <summary>
    /// 创建 Diligent 渲染资源工厂。
    /// </summary>
    /// <param name="device">图形设备</param>
    public DiligentRenderResourceFactory(GraphicsDevice device)
    {
        _device = device ?? throw new ArgumentNullException(nameof(device));
    }

    /// <inheritdoc/>
    public TextureResource? CreateTexture(uint width, uint height, TextureFormat format,
                                          ReadOnlySpan<byte> pixels, bool isSRGB)
    {
        if (width == 0 || height == 0 || pixels.IsEmpty)
        {
            Console.Error.WriteLine("[DiligentRenderResourceFactory] CreateTexture: 参数无效");
            return null;
        }

        // 构造 Diligent TextureDesc
        var texDesc = new TextureDesc
        {
            Name = "NNRenderAsset_Texture",
            Type = ResourceDimension.Tex2d,
            Width = width,
            Height = height,
            MipLevels = 1,
            Format = ToDiligentFormat(format, isSRGB),
            Usage = Usage.Default,
            BindFlags = BindFlags.ShaderResource
        };

        // 准备初始数据
        unsafe
        {
            fixed (byte* ptr = pixels)
            {
                var subresource = new TextureSubResData
                {
                    Data = new IntPtr(ptr),
                    Stride = width * TextureFormatHelper.GetBytesPerPixel(format)
                };

                var subresources = new[] { subresource };
                var textureData = new TextureData
                {
                    SubResources = subresources
                };

                // 创建纹理
                var textureHandle = _device.CreateTexture(texDesc, textureData);
                if (textureHandle == null)
                {
                    Console.Error.WriteLine("[DiligentRenderResourceFactory] CreateTexture: 设备创建失败");
                    return null;
                }

                // 获取 SRV
                var srv = textureHandle.GetDefaultView(TextureViewType.ShaderResource);
                if (srv == null)
                {
                    Console.Error.WriteLine("[DiligentRenderResourceFactory] CreateTexture: 获取 SRV 失败");
                    textureHandle.Dispose();
                    return null;
                }

                // 构造 TextureResource
                var resource = new TextureResource();
                resource.Desc.Width = width;
                resource.Desc.Height = height;
                resource.Desc.MipCount = 1;
                resource.Desc.Format = format;
                resource.Desc.IsSRGB = isSRGB;
                resource.RHITexture = textureHandle;
                resource.SetShaderResourceView(srv);
                resource.Residency = TextureResidency.Resident;

                return resource;
            }
        }
    }

    /// <inheritdoc/>
    public bool UpdateTexturePixels(TextureResource resource, ReadOnlySpan<byte> pixels)
    {
        // TODO: 需要 IDeviceContext 来调用 UpdateTexture
        Console.Error.WriteLine("[DiligentRenderResourceFactory] UpdateTexturePixels: 暂未实现");
        return false;
    }

    /// <summary>
    /// 纹理格式映射：TextureFormat → Diligent.TextureFormat。
    /// </summary>
    private static TextureFormatDiligent ToDiligentFormat(TextureFormat format, bool isSRGB)
    {
        return format switch
        {
            TextureFormat.RGBA8_UNorm => isSRGB
                ? TextureFormatDiligent.RGBA8_UNorm_sRGB
                : TextureFormatDiligent.RGBA8_UNorm,
            TextureFormat.R32_Float => TextureFormatDiligent.R32_Float,
            TextureFormat.RG32_Float => TextureFormatDiligent.RG32_Float,
            TextureFormat.RGBA32_Float => TextureFormatDiligent.RGBA32_Float,
            TextureFormat.BC1_UNorm => isSRGB
                ? TextureFormatDiligent.BC1_UNorm_sRGB
                : TextureFormatDiligent.BC1_UNorm,
            TextureFormat.BC3_UNorm => isSRGB
                ? TextureFormatDiligent.BC3_UNorm_sRGB
                : TextureFormatDiligent.BC3_UNorm,
            TextureFormat.BC7_UNorm => isSRGB
                ? TextureFormatDiligent.BC7_UNorm_sRGB
                : TextureFormatDiligent.BC7_UNorm,
            _ => isSRGB
                ? TextureFormatDiligent.RGBA8_UNorm_sRGB
                : TextureFormatDiligent.RGBA8_UNorm
        };
    }
}
