namespace Neverness.Rendering.RenderAssets;

/// <summary>
/// Mip Level 数据。
/// </summary>
public sealed class MipLevel
{
    /// <summary>此 mip 层级的宽度。</summary>
    public uint Width { get; set; }

    /// <summary>此 mip 层级的高度。</summary>
    public uint Height { get; set; }

    /// <summary>像素数据（行优先，紧密排列）。</summary>
    public byte[] Pixels { get; set; } = [];
}

/// <summary>
/// 纹理源资产（CPU 侧）。
/// 代表 import pipeline 产物，纯 CPU，不允许 GPU API。
/// </summary>
public sealed class TextureSourceAsset
{
    private uint _width;
    private uint _height;
    private TextureFormat _format = TextureFormat.RGBA8_UNorm;
    private bool _isSRGB;
    private bool _hasAlpha;
    private readonly List<MipLevel> _mips = [];

    /// <summary>纹理宽度。</summary>
    public uint Width => _width;

    /// <summary>纹理高度。</summary>
    public uint Height => _height;

    /// <summary>Mip 层级数量。</summary>
    public uint MipCount => (uint)_mips.Count;

    /// <summary>纹理格式。</summary>
    public TextureFormat Format => _format;

    /// <summary>是否为 sRGB 纹理。</summary>
    public bool IsSRGB => _isSRGB;

    /// <summary>是否包含 Alpha 通道。</summary>
    public bool HasAlpha => _hasAlpha;

    /// <summary>获取指定 mip 层级。</summary>
    public MipLevel GetMip(uint level) => _mips[(int)level];

    /// <summary>获取所有 mip 层级。</summary>
    public IReadOnlyList<MipLevel> Mips => _mips;

    /// <summary>
    /// 从解码图像初始化（base mip only）。
    /// </summary>
    public void SetFromDecodedImage(
        uint width, uint height,
        TextureFormat format,
        byte[] pixels,
        bool isSRGB, bool hasAlpha)
    {
        _width = width;
        _height = height;
        _format = format;
        _isSRGB = isSRGB;
        _hasAlpha = hasAlpha;

        var mip = new MipLevel
        {
            Width = width,
            Height = height,
            Pixels = pixels
        };

        _mips.Clear();
        _mips.Add(mip);
    }

    /// <summary>
    /// 设置完整 mip 链。
    /// </summary>
    public void SetMips(List<MipLevel> mips)
    {
        _mips.Clear();
        _mips.AddRange(mips);
        if (_mips.Count > 0)
        {
            _width = _mips[0].Width;
            _height = _mips[0].Height;
        }
    }

    /// <summary>
    /// 序列化到二进制 blob（用于 .nnasset 存储）。
    /// 格式: [Width:u32][Height:u32][Format:u32][Flags:u32][MipCount:u32][Mip0Size:u32][Mip0Pixels...][Mip1Size:u32][Mip1Pixels...]...
    /// </summary>
    public byte[] Serialize()
    {
        if (!IsValid)
            return [];

        // 计算头大小
        var headerSize = 20 + _mips.Count * 4;
        var totalSize = headerSize + GetTotalByteSize();

        var result = new byte[totalSize];
        using var stream = new MemoryStream(result);
        using var writer = new BinaryWriter(stream);

        // 写入头部
        uint flags = 0;
        if (_isSRGB) flags |= 1;
        if (_hasAlpha) flags |= 2;

        writer.Write(_width);
        writer.Write(_height);
        writer.Write((uint)_format);
        writer.Write(flags);
        writer.Write((uint)_mips.Count);

        // 写入各 mip 大小
        foreach (var mip in _mips)
        {
            writer.Write((uint)mip.Pixels.Length);
        }

        // 写入各 mip 像素
        foreach (var mip in _mips)
        {
            writer.Write(mip.Pixels);
        }

        return result;
    }

    /// <summary>
    /// 从二进制 blob 反序列化。
    /// </summary>
    public bool Deserialize(byte[] data)
    {
        if (data == null || data.Length < 20)
            return false;

        Reset();

        using var stream = new MemoryStream(data);
        using var reader = new BinaryReader(stream);

        try
        {
            _width = reader.ReadUInt32();
            _height = reader.ReadUInt32();
            _format = (TextureFormat)reader.ReadUInt32();

            var flags = reader.ReadUInt32();
            _isSRGB = (flags & 1) != 0;
            _hasAlpha = (flags & 2) != 0;

            var mipCount = reader.ReadUInt32();
            var headerSize = 20 + mipCount * 4;
            if (data.Length < headerSize)
            {
                Reset();
                return false;
            }

            // 读取各 mip 大小
            var mipSizes = new uint[mipCount];
            for (var i = 0; i < mipCount; i++)
            {
                mipSizes[i] = reader.ReadUInt32();
            }

            // 读取各 mip 像素
            _mips.Clear();
            for (var i = 0; i < mipCount; i++)
            {
                if (stream.Position + mipSizes[i] > stream.Length)
                {
                    Reset();
                    return false;
                }

                var pixels = reader.ReadBytes((int)mipSizes[i]);
                _mips.Add(new MipLevel
                {
                    Width = Math.Max(1, _width >> i),
                    Height = Math.Max(1, _height >> i),
                    Pixels = pixels
                });
            }

            return true;
        }
        catch
        {
            Reset();
            return false;
        }
    }

    /// <summary>
    /// 获取所有 mip 层级的总字节数。
    /// </summary>
    public long GetTotalByteSize()
    {
        long total = 0;
        foreach (var mip in _mips)
        {
            total += mip.Pixels.Length;
        }
        return total;
    }

    /// <summary>
    /// 纹理是否有效。
    /// </summary>
    public bool IsValid => _width > 0 && _height > 0 && _mips.Count > 0;

    /// <summary>
    /// 重置为默认状态。
    /// </summary>
    public void Reset()
    {
        _width = 0;
        _height = 0;
        _format = TextureFormat.RGBA8_UNorm;
        _isSRGB = false;
        _hasAlpha = false;
        _mips.Clear();
    }
}
