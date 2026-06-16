using Neverness.Runtime.Assets;
using StbImageSharp;

namespace Neverness.Editor.Assets;

/// <summary>
/// 纹理导入器。
///
/// 支持格式：.png, .jpg, .jpeg, .tga, .bmp, .dds, .hdr
///
/// 导入设置（.meta importSettings）：
///   compression    — BC1 | BC3 | BC5 | BC7 | None（默认 BC7）
///   generateMipmaps — true | false（默认 true）
///   maxSize         — 256 | 512 | 1024 | 2048 | 4096（默认 4096）
///   srgb            — true | false（默认 true）
///   wrapMode        — Repeat | Clamp（默认 Repeat）
///   filterMode      — Bilinear | Trilinear | Point（默认 Trilinear）
/// </summary>
[AssetImporter(".png", ".jpg", ".jpeg", ".tga", ".bmp", ".dds", ".hdr")]
public class TextureImporter : ISettingsAwareImporter
{
    public string[] SupportedExtensions => new[] { ".png", ".jpg", ".jpeg", ".tga", ".bmp", ".dds", ".hdr" };
    public string DisplayName => "Texture Importer";

    public ImportResult Import(AssetImportContext context)
    {
        try
        {
            var result = ImportResult.Ok(context.AssetGuid, AssetTypeId.Texture2D);

            // 读取源文件
            var sourceData = context.ReadAllBytes();

            // 使用 StbImageSharp 解码为 RGBA8
            var (width, height, pixelData) = DecodeImage(context.Extension, sourceData);

            if (pixelData == null || width == 0 || height == 0)
                return ImportResult.Fail($"无法解码纹理: {context.SourceAssetPath}");

            // 读取设置
            var generateMipmaps = context.GetSettingBool("generateMipmaps", true);
            var maxSize = context.GetSettingInt("maxSize", 4096);
            var isSRGB = context.GetSettingBool("srgb", true);

            // 尺寸限制
            if (width > maxSize || height > maxSize)
            {
                // TODO: 缩放至 maxSize
            }

            // 生成 mipmap 链
            var mipLevels = generateMipmaps ? CalculateMipCount(width, height) : 1;

            // 写入主数据 blob（mip0）
            result.Blobs.Add(new ImportedBlob
            {
                BlobType = AssetTypeId.BlobType.Data,
                Data = pixelData
            });

            // 生成并写入 mipmap blobs
            if (generateMipmaps && mipLevels > 1)
            {
                var mipChain = GenerateMipChain(pixelData, width, height, mipLevels);
                for (int i = 1; i < mipLevels; i++)
                {
                    result.Blobs.Add(new ImportedBlob
                    {
                        BlobType = AssetTypeId.BlobType.MipLevel,
                        Data = mipChain[i]
                    });
                }
            }

            // 写入 TypeInfo blob（与 C++ NNTextureTypeInfo 24字节布局对齐）
            // C++ 布局：width:u32, height:u32, format:u32, mipCount:u32, arraySize:u32, flags:u32
            var typeInfo = new NNTextureTypeInfoCSharp
            {
                Width = (uint)width,
                Height = (uint)height,
                Format = 4, // NNTextureFormat::RGBA8_UNorm = 4 (Unknown=0, R8=1, RG8=2, RGB8=3, RGBA8=4)
                MipCount = (uint)mipLevels,
                ArraySize = 1,
                Flags = isSRGB ? 1u : 0u
            };
            result.Blobs.Add(new ImportedBlob
            {
                BlobType = AssetTypeId.BlobType.TypeInfo,
                Data = typeInfo.ToBytes()
            });

            /* 诊断日志 */
            Console.WriteLine($"[TextureImporter] {context.SourceAssetPath.FileName}: {width}x{height} sRGB={isSRGB} mips={mipLevels} blobs={result.Blobs.Count}");
            foreach (var b in result.Blobs)
                Console.WriteLine($"  blob type={b.BlobType} size={b.Data.Length}");

            return result;
        }
        catch (Exception ex)
        {
            return ImportResult.Fail($"纹理导入异常: {ex.Message}");
        }
    }

    public Dictionary<string, string> GetDefaultSettings() => new()
    {
        ["compression"] = "BC7",
        ["generateMipmaps"] = "true",
        ["maxSize"] = "4096",
        ["srgb"] = "true",
        ["wrapMode"] = "Repeat",
        ["filterMode"] = "Trilinear"
    };

    public bool ValidateSettings(Dictionary<string, string> settings, out string? errorMessage)
    {
        errorMessage = null;

        if (settings.TryGetValue("compression", out var comp))
        {
            var valid = new[] { "BC1", "BC3", "BC5", "BC7", "None" };
            if (!valid.Contains(comp, StringComparer.OrdinalIgnoreCase))
            {
                errorMessage = $"不支持的压缩格式: {comp}，有效值: {string.Join(", ", valid)}";
                return false;
            }
        }

        if (settings.TryGetValue("maxSize", out var maxStr) && !int.TryParse(maxStr, out _))
        {
            errorMessage = $"maxSize 必须是整数: {maxStr}";
            return false;
        }

        return true;
    }

    /// <summary>
    /// 使用 StbImageSharp 解码图像为 RGBA8。
    /// </summary>
    private static (int width, int height, byte[]? data) DecodeImage(string extension, byte[] raw)
    {
        if (raw == null || raw.Length < 8)
            return (0, 0, null);

        // DDS 文件直接读取头部
        if (extension == ".dds" && raw.Length > 128)
        {
            var magic = BitConverter.ToUInt32(raw, 0);
            if (magic == 0x20534444)
            {
                var h = BitConverter.ToInt32(raw, 12);
                var w = BitConverter.ToInt32(raw, 16);
                return (w, h, raw);
            }
        }

        // 使用 StbImageSharp 解码
        try
        {
            using var stream = new MemoryStream(raw);
            var image = ImageResult.FromStream(stream, ColorComponents.RedGreenBlueAlpha);

            if (image == null || image.Data == null)
                return (0, 0, null);

            return (image.Width, image.Height, image.Data);
        }
        catch
        {
            return (0, 0, null);
        }
    }

    /// <summary>
    /// CPU 侧生成 mipmap 链（box filter 下采样）。
    /// </summary>
    private static byte[][] GenerateMipChain(byte[] basePixels, int baseWidth, int baseHeight, int mipLevels)
    {
        var chain = new byte[mipLevels][];
        chain[0] = basePixels;

        int prevW = baseWidth, prevH = baseHeight;
        var prevPixels = basePixels;

        for (int mip = 1; mip < mipLevels && prevW > 1 && prevH > 1; mip++)
        {
            int newW = Math.Max(1, prevW / 2);
            int newH = Math.Max(1, prevH / 2);
            var mipPixels = new byte[newW * newH * 4];

            for (int y = 0; y < newH; y++)
            {
                for (int x = 0; x < newW; x++)
                {
                    int sx0 = x * 2, sy0 = y * 2;
                    int sx1 = Math.Min(sx0 + 1, prevW - 1);
                    int sy1 = Math.Min(sy0 + 1, prevH - 1);

                    for (int c = 0; c < 4; c++)
                    {
                        int a = prevPixels[(sy0 * prevW + sx0) * 4 + c];
                        int b = prevPixels[(sy0 * prevW + sx1) * 4 + c];
                        int cc = prevPixels[(sy1 * prevW + sx0) * 4 + c];
                        int d = prevPixels[(sy1 * prevW + sx1) * 4 + c];
                        mipPixels[(y * newW + x) * 4 + c] = (byte)((a + b + cc + d + 2) / 4);
                    }
                }
            }

            chain[mip] = mipPixels;
            prevPixels = mipPixels;
            prevW = newW;
            prevH = newH;
        }

        return chain;
    }

    private static int CalculateMipCount(int width, int height)
    {
        int count = 0;
        while (width > 0 || height > 0)
        {
            count++;
            width /= 2;
            height /= 2;
        }
        return Math.Max(count, 1);
    }
}

/// <summary>
/// C# 侧纹理 TypeInfo（镜像 C++ NNTextureTypeInfo，24 字节）。
/// C++ 布局：width, height, format, mipCount, arraySize, flags
/// </summary>
internal struct NNTextureTypeInfoCSharp
{
    public uint Width;
    public uint Height;
    public uint Format;
    public uint MipCount;
    public uint ArraySize;
    public uint Flags;

    public byte[] ToBytes()
    {
        var buf = new byte[24];
        BitConverter.GetBytes(Width).CopyTo(buf, 0);
        BitConverter.GetBytes(Height).CopyTo(buf, 4);
        BitConverter.GetBytes(Format).CopyTo(buf, 8);
        BitConverter.GetBytes(MipCount).CopyTo(buf, 12);
        BitConverter.GetBytes(ArraySize).CopyTo(buf, 16);
        BitConverter.GetBytes(Flags).CopyTo(buf, 20);
        return buf;
    }
}
