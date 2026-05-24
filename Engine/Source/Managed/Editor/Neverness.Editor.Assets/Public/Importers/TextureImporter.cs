using Neverness.Runtime.Assets;

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

            /* 读取源文件 */
            var sourceData = context.ReadAllBytes();

            /* TODO: 实际应使用图像解码库（如 StbImageSharp）
             * 解析为 RGBA 像素数据，此处为骨架实现 */
            var (width, height, pixelData) = DecodeImage(context.Extension, sourceData);

            if (pixelData == null || width == 0 || height == 0)
                return ImportResult.Fail($"无法解码纹理: {context.SourceAssetPath}");

            /* 读取设置 */
            var generateMipmaps = context.GetSettingBool("generateMipmaps", true);
            var maxSize = context.GetSettingInt("maxSize", 4096);

            /* 尺寸限制 */
            if (width > maxSize || height > maxSize)
            {
                /* TODO: 缩放至 maxSize */
            }

            /* 生成 mipmap 链 */
            var mipLevels = generateMipmaps ? CalculateMipCount(width, height) : 1;

            /* 写入主数据 blob */
            result.Blobs.Add(new ImportedBlob
            {
                BlobType = AssetTypeId.BlobType.Data,
                Data = pixelData
            });

            /* 写入 mipmap blobs（骨架：仅写入 level 0） */
            if (generateMipmaps)
            {
                /* TODO: 实际生成各层 mip level */
                for (int i = 1; i < mipLevels; i++)
                {
                    result.Blobs.Add(new ImportedBlob
                    {
                        BlobType = AssetTypeId.BlobType.MipLevel,
                        Data = Array.Empty<byte>() /* TODO: 下采样数据 */
                    });
                }
            }

            /* 写入 TypeInfo */
            var typeInfo = new NNTextureTypeInfoCSharp
            {
                Width = (uint)width,
                Height = (uint)height,
                MipLevels = (uint)mipLevels,
                Format = 0 /* TODO: 映射到实际格式枚举 */
            };
            result.TypeInfo = typeInfo.ToBytes();

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

    /// <summary>解码图像（骨架实现）。</summary>
    private static (int width, int height, byte[]? data) DecodeImage(string extension, byte[] raw)
    {
        /* TODO: 集成 StbImageSharp 或其他解码库 */
        /* 当前返回占位数据，标记源文件长度以便调试 */
        if (raw.Length < 8)
            return (0, 0, null);

        /* DDS 文件可直接读取头部 */
        if (extension == ".dds" && raw.Length > 128)
        {
            /* DDS 头部 magic = "DDS " (0x20534444) */
            var magic = BitConverter.ToUInt32(raw, 0);
            if (magic == 0x20534444)
            {
                var h = BitConverter.ToInt32(raw, 12);
                var w = BitConverter.ToInt32(raw, 16);
                return (w, h, raw); /* DDS 原始数据直接传递 */
            }
        }

        /* 其他格式：返回占位 4x4 RGBA */
        /* TODO: 实际解码 */
        var placeholder = new byte[4 * 4 * 4]; /* 4x4 RGBA8 */
        return (4, 4, placeholder);
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
/// C# 侧纹理 TypeInfo（镜像 C++ NNTextureTypeInfo）。
/// </summary>
internal struct NNTextureTypeInfoCSharp
{
    public uint Width;
    public uint Height;
    public uint MipLevels;
    public uint Format;

    public byte[] ToBytes()
    {
        var buf = new byte[16];
        BitConverter.GetBytes(Width).CopyTo(buf, 0);
        BitConverter.GetBytes(Height).CopyTo(buf, 4);
        BitConverter.GetBytes(MipLevels).CopyTo(buf, 8);
        BitConverter.GetBytes(Format).CopyTo(buf, 12);
        return buf;
    }
}
