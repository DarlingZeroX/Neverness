using System.Buffers.Binary;
using System.Runtime.InteropServices;
using System.Text;
using StbImageSharp;

namespace Neverness.Runtime.ImageCodec;

/// <summary>
/// 純函數式圖像編解碼接口。
///
/// 無狀態，線程安全。
/// LDR 解碼底層使用 StbImageSharp（stb_image 的純 C# 移植）。
/// HDR 解碼為手寫的 .hdr Radiance 格式解碼器。
///
/// 支持格式：PNG、JPEG、BMP、TGA、PSD、GIF、HDR、PIC、PNM。
///
/// 與 C++ ImageCodecFunc 命名空間對應。
/// </summary>
public static class ImageCodec
{
    /* ======================== 解碼 ======================== */

    /// <summary>
    /// 從記憶體緩衝區解碼圖像。
    /// 自動檢測 HDR/LDR 格式。
    /// </summary>
    /// <param name="data">壓縮圖像資料（PNG/JPG/TGA/BMP/HDR）。</param>
    /// <param name="desiredChannels">期望通道數（0=自動，3=RGB，4=RGBA）。</param>
    /// <returns>解碼結果；失敗時 IsValid == false。</returns>
    public static DecodedImage DecodeFromMemory(ReadOnlySpan<byte> data, uint desiredChannels = 0)
    {
        if (data.IsEmpty)
            return new DecodedImage();

        // 檢測是否為 HDR（.hdr Radiance 格式）
        if (IsHdrData(data))
        {
            return DecodeHdrFromMemory(data, desiredChannels);
        }
        else
        {
            return DecodeLdrFromMemory(data, desiredChannels);
        }
    }

    /// <summary>
    /// 從檔案路徑解碼圖像。
    /// </summary>
    /// <param name="filePath">圖像檔案路徑。</param>
    /// <param name="desiredChannels">期望通道數（0=自動，3=RGB，4=RGBA）。</param>
    /// <returns>解碼結果；失敗時 IsValid == false。</returns>
    public static DecodedImage DecodeFromFile(string filePath, uint desiredChannels = 0)
    {
        if (string.IsNullOrEmpty(filePath) || !File.Exists(filePath))
            return new DecodedImage();

        try
        {
            var data = File.ReadAllBytes(filePath);
            return DecodeFromMemory(data, desiredChannels);
        }
        catch
        {
            return new DecodedImage();
        }
    }

    /* ======================== Mipmap 生成 ======================== */

    /// <summary>
    /// 生成 mipmap（CPU 側，簡單 box filter）。
    /// 僅支持 RGBA8 格式。
    /// </summary>
    /// <param name="source">源圖像（必須是 RGBA8）。</param>
    /// <param name="mipLevels">目標 mip 層數（含 base level）。</param>
    /// <returns>從 mip0 到 mipN 的像素資料，每個 mip 緊密排列。</returns>
    public static byte[] GenerateMipsCPU(DecodedImage source, uint mipLevels)
    {
        if (source == null || !source.IsValid || source.Format != ImageFormat.RGBA8 || mipLevels == 0)
            return Array.Empty<byte>();

        const int channels = 4; // RGBA8
        var baseW = (int)source.Width;
        var baseH = (int)source.Height;
        var srcPixels = source.Pixels;

        // 預計算總大小
        long totalSize = 0;
        int w = baseW, h = baseH;
        for (int i = 0; i < mipLevels && w > 0 && h > 0; i++)
        {
            totalSize += (long)w * h * channels;
            w = Math.Max(1, w / 2);
            h = Math.Max(1, h / 2);
        }

        var result = new byte[totalSize];
        int dstOffset = 0;

        // mip0：複製源資料
        int mip0Size = baseW * baseH * channels;
        srcPixels.AsSpan(0, mip0Size).CopyTo(result.AsSpan(dstOffset, mip0Size));
        dstOffset += mip0Size;

        // 後續 mip：2x2 box filter
        var prevMip = srcPixels;
        int prevOffset = 0;
        int prevW = baseW, prevH = baseH;

        for (int mip = 1; mip < mipLevels && prevW > 1 && prevH > 1; mip++)
        {
            int newW = Math.Max(1, prevW / 2);
            int newH = Math.Max(1, prevH / 2);

            for (int y = 0; y < newH; y++)
            {
                for (int x = 0; x < newW; x++)
                {
                    int srcX0 = x * 2;
                    int srcY0 = y * 2;
                    int srcX1 = Math.Min(srcX0 + 1, prevW - 1);
                    int srcY1 = Math.Min(srcY0 + 1, prevH - 1);

                    for (int c = 0; c < channels; c++)
                    {
                        int a = prevMip[prevOffset + (srcY0 * prevW + srcX0) * channels + c];
                        int b = prevMip[prevOffset + (srcY0 * prevW + srcX1) * channels + c];
                        int cc = prevMip[prevOffset + (srcY1 * prevW + srcX0) * channels + c];
                        int d = prevMip[prevOffset + (srcY1 * prevW + srcX1) * channels + c];

                        result[dstOffset + (y * newW + x) * channels + c] =
                            (byte)((a + b + cc + d + 2) / 4);
                    }
                }
            }

            prevMip = result;
            prevOffset = dstOffset;
            dstOffset += newW * newH * channels;
            prevW = newW;
            prevH = newH;
        }

        return result;
    }

    /* ======================== 工具函數 ======================== */

    /// <summary>
    /// 判斷格式是否為 HDR（浮點資料）。
    /// </summary>
    public static bool IsHDRFormat(ImageFormat format)
    {
        return format switch
        {
            ImageFormat.R16F or ImageFormat.RG16F or ImageFormat.RGB16F or ImageFormat.RGBA16F
            or ImageFormat.R32F or ImageFormat.RG32F or ImageFormat.RGB32F or ImageFormat.RGBA32F
                => true,
            _ => false,
        };
    }

    /// <summary>
    /// 計算給定寬高的最大 mip 層數。
    /// </summary>
    public static uint CalculateMipCount(uint width, uint height)
    {
        uint count = 1;
        while (width > 1 || height > 1)
        {
            width = Math.Max(1u, width / 2);
            height = Math.Max(1u, height / 2);
            count++;
        }
        return count;
    }

    /// <summary>
    /// 取得格式對應的每像素位元組數。
    /// </summary>
    public static uint GetBytesPerPixel(ImageFormat format)
    {
        return format switch
        {
            ImageFormat.R8 => 1,
            ImageFormat.RG8 => 2,
            ImageFormat.RGB8 => 3,
            ImageFormat.RGBA8 => 4,
            ImageFormat.R16F => 2,
            ImageFormat.RG16F => 4,
            ImageFormat.RGB16F => 6,
            ImageFormat.RGBA16F => 8,
            ImageFormat.R32F => 4,
            ImageFormat.RG32F => 8,
            ImageFormat.RGB32F => 12,
            ImageFormat.RGBA32F => 16,
            _ => 0,
        };
    }

    /* ======================== LDR 解碼（StbImageSharp） ======================== */

    /// <summary>
    /// 從記憶體解碼 LDR 圖像（8-bit 整型）。
    /// </summary>
    private static DecodedImage DecodeLdrFromMemory(ReadOnlySpan<byte> data, uint desiredChannels)
    {
        try
        {
            var dataArray = data.ToArray();
            var comp = desiredChannels > 0 ? (ColorComponents)desiredChannels : ColorComponents.Default;
            var result = ImageResult.FromMemory(dataArray, comp);

            if (result?.Data == null || result.Width <= 0 || result.Height <= 0)
                return new DecodedImage();

            uint channels = desiredChannels > 0 ? desiredChannels : (uint)result.Comp switch
            {
                1 => 1, 2 => 2, 3 => 3, 4 => 4, _ => 4,
            };

            return new DecodedImage
            {
                Width = (uint)result.Width,
                Height = (uint)result.Height,
                Channels = channels,
                Format = ChannelsToFormat(channels, false),
                Pixels = result.Data,
            };
        }
        catch
        {
            return new DecodedImage();
        }
    }

    /* ======================== HDR 解碼（手寫 .hdr Radiance） ======================== */

    /// <summary>
    /// 從記憶體解碼 HDR 圖像（.hdr Radiance RGBE 格式 → float RGB）。
    /// </summary>
    private static DecodedImage DecodeHdrFromMemory(ReadOnlySpan<byte> data, uint desiredChannels)
    {
        try
        {
            return DecodeHdrRadiance(data, desiredChannels);
        }
        catch
        {
            return new DecodedImage();
        }
    }

    /// <summary>
    /// .hdr Radiance 格式解碼器。
    /// 解碼 RGBE 像素為 float RGB/RGBA。
    /// </summary>
    private static DecodedImage DecodeHdrRadiance(ReadOnlySpan<byte> data, uint desiredChannels)
    {
        int pos = 0;

        // 跳過 header 行（以空行結束）
        while (pos < data.Length - 1)
        {
            if (data[pos] == '\n' && (pos + 1 < data.Length && (data[pos + 1] == '\n' || data[pos + 1] == '\r')))
            {
                pos += 2;
                break;
            }
            // 跳過當前行
            while (pos < data.Length && data[pos] != '\n')
                pos++;
            pos++;
        }

        // 讀取分辨率行（如 "-Y 1024 +X 2048"）
        var resLine = ReadLine(data, ref pos);
        if (resLine == null)
            return new DecodedImage();

        if (!ParseResolution(resLine, out int width, out int height))
            return new DecodedImage();

        if (width <= 0 || height <= 0 || width > 65536 || height > 65536)
            return new DecodedImage();

        uint channels = desiredChannels > 0 ? desiredChannels : 3;

        // 解碼 RGBE 像素
        var rgbePixels = new byte[width * height * 4]; // RGBE = 4 bytes per pixel
        int pixelOffset = 0;

        for (int y = 0; y < height; y++)
        {
            // 檢查是否為新的 RLE 格式（magic: 0x02 0x02）
            if (pos + 4 <= data.Length
                && data[pos] == 2 && data[pos + 1] == 2
                && data[pos + 2] == ((width >> 8) & 0xFF)
                && data[pos + 3] == (width & 0xFF))
            {
                // 新格式 RLE：每個通道獨立 RLE
                pos += 4;
                for (int ch = 0; ch < 4; ch++)
                {
                    int x = 0;
                    while (x < width)
                    {
                        if (pos >= data.Length) goto done;
                        byte code = data[pos++];
                        if (code > 128)
                        {
                            // RLE run
                            int count = code - 128;
                            if (pos >= data.Length) goto done;
                            byte val = data[pos++];
                            for (int i = 0; i < count && x < width; i++)
                            {
                                rgbePixels[pixelOffset + x * 4 + ch] = val;
                                x++;
                            }
                        }
                        else
                        {
                            // 原始資料
                            for (int i = 0; i < code && x < width; i++)
                            {
                                if (pos >= data.Length) goto done;
                                rgbePixels[pixelOffset + x * 4 + ch] = data[pos++];
                                x++;
                            }
                        }
                    }
                }
            }
            else
            {
                // 舊格式：每像素 RGBE，可能有 RLE
                int x = 0;
                while (x < width)
                {
                    if (pos + 4 > data.Length) goto done;

                    byte r = data[pos++], g = data[pos++], b = data[pos++], e = data[pos++];

                    if (r == 2 && g == 2 && b < 128)
                    {
                        // 舊格式中嵌入的新格式 RLE
                        // 重新處理
                        pos -= 4;
                        goto newRle;
                    }

                    rgbePixels[pixelOffset + x * 4 + 0] = r;
                    rgbePixels[pixelOffset + x * 4 + 1] = g;
                    rgbePixels[pixelOffset + x * 4 + 2] = b;
                    rgbePixels[pixelOffset + x * 4 + 3] = e;
                    x++;
                    continue;

                    newRle:
                    // 跳過 magic bytes
                    pos += 2;
                    for (int ch = 0; ch < 4; ch++)
                    {
                        int cx = x;
                        while (cx < width)
                        {
                            if (pos >= data.Length) goto done;
                            byte code = data[pos++];
                            if (code > 128)
                            {
                                int count = code - 128;
                                if (pos >= data.Length) goto done;
                                byte val = data[pos++];
                                for (int i = 0; i < count && cx < width; i++)
                                {
                                    rgbePixels[pixelOffset + cx * 4 + ch] = val;
                                    cx++;
                                }
                            }
                            else
                            {
                                for (int i = 0; i < code && cx < width; i++)
                                {
                                    if (pos >= data.Length) goto done;
                                    rgbePixels[pixelOffset + cx * 4 + ch] = data[pos++];
                                    cx++;
                                }
                            }
                        }
                    }
                    x = width; // 該行已處理完
                }
            }

            pixelOffset += width * 4;
        }

        done:

        // RGBE → float RGB/RGBA
        int floatChannels = (int)channels;
        var floatPixels = new float[width * height * floatChannels];

        for (int i = 0; i < width * height; i++)
        {
            byte r = rgbePixels[i * 4 + 0];
            byte g = rgbePixels[i * 4 + 1];
            byte b = rgbePixels[i * 4 + 2];
            byte e = rgbePixels[i * 4 + 3];

            float scale = MathF.Pow(2.0f, e - 128) / 256.0f;
            float fr = r * scale;
            float fg = g * scale;
            float fb = b * scale;

            int dstIdx = i * floatChannels;
            floatPixels[dstIdx + 0] = fr;
            floatPixels[dstIdx + 1] = fg;
            if (floatChannels >= 3)
                floatPixels[dstIdx + 2] = fb;
            if (floatChannels >= 4)
                floatPixels[dstIdx + 3] = 1.0f; // alpha = 1
        }

        // float[] → byte[]
        var pixelBytes = new byte[floatPixels.Length * sizeof(float)];
        Buffer.BlockCopy(floatPixels, 0, pixelBytes, 0, pixelBytes.Length);

        return new DecodedImage
        {
            Width = (uint)width,
            Height = (uint)height,
            Channels = channels,
            Format = ChannelsToFormat(channels, true),
            Pixels = pixelBytes,
        };
    }

    /* ======================== HDR 工具 ======================== */

    private static string? ReadLine(ReadOnlySpan<byte> data, ref int pos)
    {
        int start = pos;
        while (pos < data.Length && data[pos] != '\n')
            pos++;
        if (pos > start)
        {
            var line = Encoding.ASCII.GetString(data[start..pos]);
            pos++; // 跳過 '\n'
            return line;
        }
        pos++;
        return null;
    }

    private static bool ParseResolution(string resLine, out int width, out int height)
    {
        width = 0;
        height = 0;

        var parts = resLine.Split(' ', StringSplitOptions.RemoveEmptyEntries);
        if (parts.Length < 4) return false;

        // 格式：-Y height +X width（或 +Y height -X width 等）
        for (int i = 0; i < parts.Length - 1; i++)
        {
            if (parts[i] == "-Y" || parts[i] == "+Y")
            {
                if (!int.TryParse(parts[i + 1], out height)) return false;
            }
            if (parts[i] == "-X" || parts[i] == "+X")
            {
                if (!int.TryParse(parts[i + 1], out width)) return false;
            }
        }

        return width > 0 && height > 0;
    }

    /* ======================== 格式工具 ======================== */

    /// <summary>
    /// 檢測資料是否為 HDR 格式（.hdr Radiance）。
    /// </summary>
    private static bool IsHdrData(ReadOnlySpan<byte> data)
    {
        // .hdr 檔案以 "#?RADIANCE" 或 "#?RGBE" 開頭
        if (data.Length >= 10)
        {
            ReadOnlySpan<byte> radiance = "#?RADIANCE"u8;
            if (data[..10].SequenceEqual(radiance))
                return true;

            if (data.Length >= 6)
            {
                ReadOnlySpan<byte> rgbe = "#?RGBE"u8;
                if (data[..6].SequenceEqual(rgbe))
                    return true;
            }
        }

        return false;
    }

    /// <summary>
    /// 從通道數和 HDR 標誌映射到 ImageFormat。
    /// </summary>
    private static ImageFormat ChannelsToFormat(uint channels, bool isHDR)
    {
        if (isHDR)
        {
            return channels switch
            {
                1 => ImageFormat.R32F,
                2 => ImageFormat.RG32F,
                3 => ImageFormat.RGB32F,
                4 => ImageFormat.RGBA32F,
                _ => ImageFormat.Unknown,
            };
        }
        else
        {
            return channels switch
            {
                1 => ImageFormat.R8,
                2 => ImageFormat.RG8,
                3 => ImageFormat.RGB8,
                4 => ImageFormat.RGBA8,
                _ => ImageFormat.Unknown,
            };
        }
    }
}
