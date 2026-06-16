using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets;

/// <summary>
/// 缩略图管理器。
///
/// 负责为资产生成缩略图，用于编辑器 Content Browser 的预览。
///
/// 缩略图存储：Library/Thumbnails/{guid_hex}.png
///
/// 支持：
///   - 纹理缩略图（降采样到 128×128）
///   - 网格缩略图（渲染截图，需要渲染器支持）
///   - 默认图标 fallback（按资产类型返回预置图标）
///   - 自定义缩略图（Importer 生成）
///
/// 设计：
///   - ImportPipeline 在 Phase 7 调用 Generate()
///   - 缩略图数据在 ImportResult.ThumbnailData 中传递
///   - 如果 Importer 已生成缩略图，直接使用
/// </summary>
public static class ThumbnailManager
{
    private static NPath s_thumbnailRoot;

    /// <summary>默认缩略图尺寸。</summary>
    public const int DefaultSize = 128;

    /// <summary>初始化缩略图目录。</summary>
    public static void Initialize(NPath libraryRoot)
    {
        s_thumbnailRoot = libraryRoot.Combine("Thumbnails");
        if (!Directory.Exists(s_thumbnailRoot.FullPath))
            Directory.CreateDirectory(s_thumbnailRoot.FullPath);
    }

    /// <summary>缩略图文件根目录。</summary>
    public static NPath ThumbnailRoot => s_thumbnailRoot;

    /* ========== 缩略图生成 ========== */

    /// <summary>
    /// 生成资产缩略图。
    /// 如果 ImportResult 已包含 ThumbnailData，直接写入。
    /// 否则根据资产类型生成默认缩略图。
    /// </summary>
    public static void Generate(GUID guid, ImportResult result)
    {
        if (s_thumbnailRoot.IsEmpty)
            return;

        try
        {
            byte[]? thumbnailData = result.ThumbnailData;

            /* 如果 importer 没有生成缩略图，按类型生成 */
            if (thumbnailData == null || thumbnailData.Length == 0)
            {
                thumbnailData = result.TypeId switch
                {
                    AssetTypeId.Texture2D => GenerateTextureThumbnail(result),
                    AssetTypeId.Mesh => GenerateMeshThumbnail(result),
                    _ => null
                };
            }

            /* 降级到默认图标 */
            if (thumbnailData == null || thumbnailData.Length == 0)
                thumbnailData = GetDefaultThumbnail(result.TypeId);

            if (thumbnailData != null && thumbnailData.Length > 0)
            {
                var path = GetThumbnailPath(guid);
                File.WriteAllBytes(path.FullPath, thumbnailData);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[ThumbnailManager] 生成缩略图失败: {guid.ToUuidString()} → {ex.Message}");
        }
    }

    /// <summary>获取缩略图路径，不存在返回 null。</summary>
    public static NPath? GetThumbnail(GUID guid)
    {
        if (s_thumbnailRoot.IsEmpty)
            return null;

        var path = GetThumbnailPath(guid);
        return File.Exists(path.FullPath) ? path : null;
    }

    /// <summary>删除资产的缩略图。</summary>
    public static void Delete(GUID guid)
    {
        if (s_thumbnailRoot.IsEmpty)
            return;

        var path = GetThumbnailPath(guid);
        if (File.Exists(path.FullPath))
        {
            try { File.Delete(path.FullPath); }
            catch { /* 忽略 */ }
        }
    }

    /// <summary>清空所有缩略图缓存。</summary>
    public static void ClearAll()
    {
        if (s_thumbnailRoot.IsEmpty || !Directory.Exists(s_thumbnailRoot.FullPath))
            return;

        try
        {
            foreach (var file in Directory.EnumerateFiles(s_thumbnailRoot.FullPath, "*.png"))
                File.Delete(file);
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[ThumbnailManager] 清空缩略图失败: {ex.Message}");
        }
    }

    /* ========== 纹理缩略图 ========== */

    /// <summary>从 ImportResult 生成纹理缩略图（最近邻降采样）。</summary>
    private static byte[]? GenerateTextureThumbnail(ImportResult result)
    {
        if (result.Blobs.Count == 0)
            return null;

        var mainBlob = result.Blobs[0].Data;
        if (mainBlob == null || mainBlob.Length == 0)
            return null;

        /* 尝试从 TypeInfo 获取尺寸 */
        int srcWidth = 0, srcHeight = 0;
        if (result.TypeInfo != null && result.TypeInfo.Length >= 8)
        {
            srcWidth = BitConverter.ToInt32(result.TypeInfo, 0);
            srcHeight = BitConverter.ToInt32(result.TypeInfo, 4);
        }

        /* 如果没有 TypeInfo，尝试从 blob 大小推断正方形 */
        if (srcWidth == 0 || srcHeight == 0)
        {
            /* 假设 RGBA8 */
            var pixelCount = mainBlob.Length / 4;
            srcWidth = (int)Math.Sqrt(pixelCount);
            srcHeight = srcWidth;
            if (srcWidth * srcHeight * 4 != mainBlob.Length)
                return null; /* 无法推断 */
        }

        /* 降采样到 DefaultSize */
        return DownsampleRgba(mainBlob, srcWidth, srcHeight, DefaultSize, DefaultSize);
    }

    /// <summary>最近邻降采样 RGBA8 数据。</summary>
    private static byte[] DownsampleRgba(byte[] src, int srcW, int srcH, int dstW, int dstH)
    {
        var dst = new byte[dstW * dstH * 4];

        for (int dy = 0; dy < dstH; dy++)
        {
            var sy = (int)((long)dy * srcH / dstH);
            for (int dx = 0; dx < dstW; dx++)
            {
                var sx = (int)((long)dx * srcW / dstW);

                var si = (sy * srcW + sx) * 4;
                var di = (dy * dstW + dx) * 4;

                if (si + 3 < src.Length && di + 3 < dst.Length)
                {
                    dst[di] = src[si];
                    dst[di + 1] = src[si + 1];
                    dst[di + 2] = src[si + 2];
                    dst[di + 3] = src[si + 3];
                }
            }
        }

        /* 编码为 BMP 格式（简单实现，无压缩） */
        return EncodeBmp(dst, dstW, dstH);
    }

    /* ========== 网格缩略图 ========== */

    /// <summary>生成网格缩略图（骨架实现，需要渲染器支持）。</summary>
    private static byte[]? GenerateMeshThumbnail(ImportResult result)
    {
        /* TODO: 需要离屏渲染器渲染 3D 预览
         * 1. 创建临时场景，加载网格
         * 2. 设置相机、灯光
         * 3. 渲染到 RenderTarget
         * 4. 读取像素数据
         * 5. 编码为 PNG/BMP
         *
         * 当前返回 null，使用默认图标 */
        return null;
    }

    /* ========== 默认图标 ========== */

    /// <summary>获取按资产类型的默认图标。</summary>
    private static byte[] GetDefaultThumbnail(ulong typeId)
    {
        /* 生成简单的彩色方块作为默认图标 */
        var (r, g, b) = GetTypeColor(typeId);
        return GenerateColorIcon(DefaultSize, DefaultSize, r, g, b, typeId switch
        {
            AssetTypeId.Texture2D => "TEX",
            AssetTypeId.Mesh => "MSH",
            AssetTypeId.AudioClip => "AUD",
            AssetTypeId.Material => "MAT",
            AssetTypeId.Shader => "SHD",
            AssetTypeId.Scene => "SCN",
            AssetTypeId.Prefab => "PRF",
            AssetTypeId.Animation => "ANI",
            AssetTypeId.LuaScript => "LUA",
            _ => "???"
        });
    }

    /// <summary>根据资产类型返回主题色。</summary>
    private static (byte r, byte g, byte b) GetTypeColor(ulong typeId) => typeId switch
    {
        AssetTypeId.Texture2D => (0x3A, 0x9A, 0xFC), /* 蓝色 */
        AssetTypeId.Mesh => (0xE8, 0x6D, 0x5A),      /* 红色 */
        AssetTypeId.AudioClip => (0x9B, 0x59, 0xB6),  /* 紫色 */
        AssetTypeId.Material => (0xF3, 0x9C, 0x12),   /* 橙色 */
        AssetTypeId.Shader => (0x2E, 0xCC, 0x71),     /* 绿色 */
        AssetTypeId.Scene => (0x1A, 0xBC, 0x9C),      /* 青色 */
        AssetTypeId.Prefab => (0xE7, 0x4C, 0x3C),     /* 深红 */
        AssetTypeId.Animation => (0x8E, 0x44, 0xAD),  /* 深紫 */
        AssetTypeId.LuaScript => (0x27, 0xAE, 0x60),  /* 深绿 */
        _ => (0x7F, 0x8C, 0x8D)                        /* 灰色 */
    };

    /// <summary>
    /// 生成带有颜色和文字的 BMP 图标（无外部依赖）。
    /// </summary>
    private static byte[] GenerateColorIcon(int width, int height, byte r, byte g, byte b, string label)
    {
        /* 创建纯色位图 */
        var pixelData = new byte[width * height * 3]; /* BGR */

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                var idx = (y * width + x) * 3;

                /* 简单的边框效果 */
                bool isBorder = x == 0 || x == width - 1 || y == 0 || y == height - 1;
                bool isInnerBorder = x <= 2 || x >= width - 3 || y <= 2 || y >= height - 3;

                if (isBorder)
                {
                    /* 深色边框 */
                    pixelData[idx] = (byte)(b / 2);     /* B */
                    pixelData[idx + 1] = (byte)(g / 2); /* G */
                    pixelData[idx + 2] = (byte)(r / 2); /* R */
                }
                else if (isInnerBorder)
                {
                    /* 浅色内边框 */
                    pixelData[idx] = (byte)Math.Min(b * 1.3, 255);
                    pixelData[idx + 1] = (byte)Math.Min(g * 1.3, 255);
                    pixelData[idx + 2] = (byte)Math.Min(r * 1.3, 255);
                }
                else
                {
                    pixelData[idx] = b;
                    pixelData[idx + 1] = g;
                    pixelData[idx + 2] = r;
                }
            }
        }

        return EncodeBmp(pixelData, width, height);
    }

    /* ========== BMP 编码 ========== */

    /// <summary>
    /// 编码为 BMP 格式（无压缩 24-bit RGB）。
    /// 不依赖任何外部库。
    /// </summary>
    private static byte[] EncodeBmp(byte[] bgrData, int width, int height)
    {
        /* BMP 每行必须 4 字节对齐 */
        var rowSize = (width * 3 + 3) & ~3;
        var imageSize = rowSize * height;
        var fileSize = 54 + imageSize;

        var bmp = new byte[fileSize];
        using var ms = new MemoryStream(bmp);
        using var w = new BinaryWriter(ms);

        /* BMP 文件头 (14 bytes) */
        w.Write((byte)'B');
        w.Write((byte)'M');
        w.Write(fileSize);
        w.Write(0);   /* reserved */
        w.Write(54);  /* pixel data offset */

        /* DIB 头 (40 bytes - BITMAPINFOHEADER) */
        w.Write(40);         /* header size */
        w.Write(width);
        w.Write(height);
        w.Write((short)1);   /* planes */
        w.Write((short)24);  /* bits per pixel */
        w.Write(0);          /* compression (none) */
        w.Write(imageSize);
        w.Write(2835);       /* ppm X (~72 DPI) */
        w.Write(2835);       /* ppm Y */
        w.Write(0);          /* colors used */
        w.Write(0);          /* important colors */

        /* 像素数据（BMP 从下到上） */
        var pad = new byte[rowSize - width * 3];
        for (int y = height - 1; y >= 0; y--)
        {
            var srcOffset = y * width * 3;
            w.Write(bgrData, srcOffset, width * 3);
            if (pad.Length > 0)
                w.Write(pad);
        }

        return bmp;
    }

    /* ========== 路径 ========== */

    private static NPath GetThumbnailPath(GUID guid)
    {
        return s_thumbnailRoot.Combine(guid.ToHexString() + ".png");
    }
}
