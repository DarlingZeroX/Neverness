namespace Neverness.Runtime.ImageCodec;

/// <summary>
/// 解碼後的 CPU 圖像資料。
///
/// 純 CPU，不允許任何 GPU API。
/// 像素資料行優先、緊密排列、無 padding。
///
/// 與 C++ NNDecodedImage 結構對應。
/// </summary>
public sealed class DecodedImage
{
    /// <summary>圖像寬度（像素）。</summary>
    public uint Width { get; set; }

    /// <summary>圖像高度（像素）。</summary>
    public uint Height { get; set; }

    /// <summary>通道數。</summary>
    public uint Channels { get; set; }

    /// <summary>像素格式。</summary>
    public ImageFormat Format { get; set; }

    /// <summary>行優先、緊密排列的像素資料。</summary>
    public byte[] Pixels { get; set; } = Array.Empty<byte>();

    /// <summary>圖像是否有效。</summary>
    public bool IsValid => Pixels.Length > 0 && Width > 0 && Height > 0;

    /// <summary>像素資料位元組數。</summary>
    public int ByteSize => Pixels.Length;

    /// <summary>重置所有欄位。</summary>
    public void Reset()
    {
        Width = 0;
        Height = 0;
        Channels = 0;
        Format = ImageFormat.Unknown;
        Pixels = Array.Empty<byte>();
    }
}
