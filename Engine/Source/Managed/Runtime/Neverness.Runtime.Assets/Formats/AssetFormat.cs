using System.Runtime.InteropServices;

namespace Neverness.Runtime.Assets.Formats;

/// <summary>
/// .nnasset 運行時二進位格式定義。
///
/// 格式佈局：
///   [AssetHeader]         — 固定 96 位元組（两行 cache line）
///   [Dependency GUIDs]    — GUID × dependencyCount
///   [Blob Descriptors]    — BlobDescriptor × blobCount
///   [Padding]             — 64 位元組對齊
///   [Binary Payload]      — blob 資料連續儲存
/// </summary>
public static class AssetFormat
{
    /* ======================== 常量 ======================== */

    /// <summary>.nnasset 檔案魔數：'NNAS'。</summary>
    public const uint Magic = 0x4E4E4153u;

    /// <summary>當前格式版本。</summary>
    public const uint Version = 2u;

    /// <summary>Header 大小（位元組，两行 cache line）。</summary>
    public const uint HeaderSize = 96u;

    /// <summary>對齊粒度。</summary>
    public const uint Alignment = 64u;

    /* ======================== 標誌位 ======================== */

    /// <summary>整體壓縮（payload 使用 Zstd/LZ4）。</summary>
    public const uint FlagCompressed = 1u << 0;

    /// <summary>支援 streaming（mip/LOD 分 blob）。</summary>
    public const uint FlagStreaming = 1u << 1;

    /// <summary>屬於 bundle/package 成員。</summary>
    public const uint FlagBundleMember = 1u << 2;

    /// <summary>包含額外型別資訊（blob 之後、payload 之前）。</summary>
    public const uint FlagHasTypeInfo = 1u << 3;

    /* ======================== 工具方法 ======================== */

    /// <summary>
    /// 對齊至 64 位元組邊界。
    /// </summary>
    public static ulong Align(ulong offset)
    {
        return (offset + Alignment - 1) & ~(ulong)(Alignment - 1);
    }

    /// <summary>
    /// 驗證 Header 魔數與版本。
    /// </summary>
    public static bool IsHeaderValid(in AssetHeader header)
    {
        return header.Magic == Magic && header.Version == Version;
    }
}

/* ======================== 格式結構 ======================== */

/// <summary>
/// .nnasset 檔案頭部（固定 96 位元組，两行 cache line）。
/// 所有偏移量相對於檔案開頭。
/// 與 C++ NNAssetHeader 記憶體佈局一致（pack=8）。
/// </summary>
[StructLayout(LayoutKind.Sequential, Pack = 8)]
public struct AssetHeader
{
    /// <summary>[0] 魔數 'NNAS'。</summary>
    public uint Magic;

    /// <summary>[4] 格式版本。</summary>
    public uint Version;

    /// <summary>[8] 資產 GUID（16 位元組）。</summary>
    public GUID AssetGuid;

    /// <summary>[24] 型別 ID（FNV-1a of type name）。</summary>
    public ulong TypeId;

    /// <summary>[32] 依賴數量。</summary>
    public uint DependencyCount;

    /// <summary>[36] blob 數量。</summary>
    public uint BlobCount;

    /// <summary>[40] 依賴表偏移。</summary>
    public ulong DependencyOffset;

    /// <summary>[48] blob 表偏移。</summary>
    public ulong BlobTableOffset;

    /// <summary>[56] 載荷偏移。</summary>
    public ulong PayloadOffset;

    /* --- 以下在第二個 cache line（偏移 64）--- */

    /// <summary>[64] 載荷大小。</summary>
    public ulong PayloadSize;

    /// <summary>[72] 標誌位。</summary>
    public uint Flags;

    /// <summary>[76] 預留。</summary>
    public uint Reserved0;

    /// <summary>[80] 預留。</summary>
    public ulong Reserved1;

    /// <summary>[88] 預留。</summary>
    public ulong Reserved2;
}

/// <summary>
/// Blob 描述符（32 位元組）。
/// 與 C++ NNBlobDescriptor 記憶體佈局一致（pack=8）。
/// </summary>
[StructLayout(LayoutKind.Sequential, Pack = 8)]
public struct BlobDescriptor
{
    /// <summary>相對於 PayloadOffset 的偏移。</summary>
    public ulong Offset;

    /// <summary>未壓縮大小。</summary>
    public ulong Size;

    /// <summary>壓縮後大小（0 = 未壓縮）。</summary>
    public ulong CompressedSize;

    /// <summary>Blob 類型（BlobType 常量）。</summary>
    public uint BlobType;

    /// <summary>預留標誌。</summary>
    public uint Flags;
}

/* ======================== 型別特定資訊結構 ======================== */

/// <summary>
/// 紋理型別資訊（TypeId = AssetTypeId.Texture2D）。
/// 與 C++ NNTextureTypeInfo 記憶體佈局一致。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct TextureTypeInfo
{
    public uint Width;
    public uint Height;
    public uint Format;       // DXGI_FORMAT 等枚舉值
    public uint MipCount;
    public uint ArraySize;
    public uint Flags;        // sRGB、generateMipmaps 等
}

/// <summary>
/// 網格型別資訊（TypeId = AssetTypeId.Mesh）。
/// 與 C++ NNMeshTypeInfo 記憶體佈局一致。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct MeshTypeInfo
{
    public uint VertexCount;
    public uint IndexCount;
    public uint VertexStride;
    public uint IndexFormat;  // 16 或 32
    public unsafe fixed float BoundsMin[3];
    public unsafe fixed float BoundsMax[3];
}

/// <summary>
/// 音訊型別資訊（TypeId = AssetTypeId.AudioClip）。
/// 與 C++ NNAudioTypeInfo 記憶體佈局一致。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct AudioTypeInfo
{
    public uint SampleRate;
    public uint Channels;
    public ulong SampleCount;
    public uint Format;       // AudioCompressionFormat 枚舉
    public uint Flags;        // AudioFlag 常量
}

/// <summary>
/// 影片型別資訊（TypeId = AssetTypeId.VideoClip）。
/// 與 C++ NNVideoTypeInfo 記憶體佈局一致。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct VideoTypeInfo
{
    public uint Width;
    public uint Height;
    public uint FpsNum;       // 幀率分子（如 30000）
    public uint FpsDen;       // 幀率分母（如 1001）
    public ulong FrameCount;  // 總幀數
    public double Duration;   // 總時長（秒）
    public uint CodecId;      // VideoCodec 枚舉
    public uint Flags;        // VideoFlag 常量
    public uint AudioSampleRate; // 音軌取樣率（0=無音軌）
    public uint AudioChannels;   // 音軌聲道數
}

/* ======================== 媒體枚舉 ======================== */

/// <summary>
/// 音訊壓縮格式（與 C++ NNAudioCompressionFormat 一致）。
/// </summary>
public enum AudioCompressionFormat : uint
{
    Pcm16 = 0,
    Float32 = 1,
    Opus = 2,
    Vorbis = 3
}

/// <summary>
/// 影片編碼格式（與 C++ NNVideoCodec 一致）。
/// </summary>
public enum VideoCodec : uint
{
    H264 = 0,
    H265 = 1,
    VP8 = 2,
    VP9 = 3,
    AV1 = 4,
    ProRes = 5
}

/* ======================== 音訊/影片標誌位 ======================== */

/// <summary>
/// 音訊標誌位常量。
/// </summary>
public static class AudioFlags
{
    public const uint Streaming = 1u << 0;
    public const uint Loop = 1u << 1;
    public const uint Is3D = 1u << 2;
}

/// <summary>
/// 影片標誌位常量。
/// </summary>
public static class VideoFlags
{
    public const uint HasAudio = 1u << 0;
    public const uint Streaming = 1u << 1;
    public const uint HasSubtitles = 1u << 2;
    public const uint AlphaChannel = 1u << 3;
    public const uint Loop = 1u << 4;
}

/* ======================== 格式讀寫器 ======================== */

/// <summary>
/// .nnasset 格式讀寫工具。
/// </summary>
public static class AssetFormatReader
{
    /// <summary>
    /// 從位元組緩衝區讀取 Header。
    /// </summary>
    /// <param name="data">至少 96 位元組的緩衝區。</param>
    /// <returns>解析後的 Header；若魔數/版本不正確則 Header.Magic 為 0。</returns>
    public static AssetHeader ReadHeader(ReadOnlySpan<byte> data)
    {
        if (data.Length < (int)AssetFormat.HeaderSize)
            return default;

        var header = MemoryMarshal.Read<AssetHeader>(data);
        if (!AssetFormat.IsHeaderValid(in header))
            return default;

        return header;
    }

    /// <summary>
    /// 從位元組緩衝區讀取完整的 .nnasset 檔案。
    /// </summary>
    /// <param name="data">完整的 .nnasset 檔案資料。</param>
    /// <param name="header">輸出：Header。</param>
    /// <param name="dependencies">輸出：依賴 GUID 列表。</param>
    /// <param name="blobs">輸出：Blob 描述符列表。</param>
    /// <param name="payload">輸出：載荷資料（Span 切片，不複製）。</param>
    /// <returns>是否成功。</returns>
    public static bool ReadFile(ReadOnlySpan<byte> data,
        out AssetHeader header,
        out ReadOnlySpan<GUID> dependencies,
        out ReadOnlySpan<BlobDescriptor> blobs,
        out ReadOnlySpan<byte> payload)
    {
        header = default;
        dependencies = ReadOnlySpan<GUID>.Empty;
        blobs = ReadOnlySpan<BlobDescriptor>.Empty;
        payload = ReadOnlySpan<byte>.Empty;

        if (data.Length < (int)AssetFormat.HeaderSize)
            return false;

        header = MemoryMarshal.Read<AssetHeader>(data);
        if (!AssetFormat.IsHeaderValid(in header))
            return false;

        // 讀取依賴 GUID
        if (header.DependencyCount > 0)
        {
            var depOffset = (int)header.DependencyOffset;
            var depSize = (int)header.DependencyCount * 16; // sizeof(GUID) = 16
            if (depOffset + depSize > data.Length)
                return false;
            dependencies = MemoryMarshal.Cast<byte, GUID>(data.Slice(depOffset, depSize));
        }

        // 讀取 Blob 描述符
        if (header.BlobCount > 0)
        {
            var blobOffset = (int)header.BlobTableOffset;
            var blobSize = (int)header.BlobCount * 32; // sizeof(BlobDescriptor) = 32
            if (blobOffset + blobSize > data.Length)
                return false;
            blobs = MemoryMarshal.Cast<byte, BlobDescriptor>(data.Slice(blobOffset, blobSize));
        }

        // 讀取 Payload（零拷貝切片）
        if (header.PayloadSize > 0)
        {
            var payOffset = (int)header.PayloadOffset;
            var paySize = (int)header.PayloadSize;
            if (payOffset + paySize > data.Length)
                return false;
            payload = data.Slice(payOffset, paySize);
        }

        return true;
    }

    /// <summary>
    /// 從 Payload 中讀取指定 Blob 的資料（零拷貝）。
    /// </summary>
    /// <param name="payload">Payload 資料。</param>
    /// <param name="blob">Blob 描述符。</param>
    /// <returns>Blob 資料的 Span 切片。</returns>
    public static ReadOnlySpan<byte> GetBlobData(ReadOnlySpan<byte> payload, in BlobDescriptor blob)
    {
        var offset = (int)blob.Offset;
        var size = blob.CompressedSize > 0 ? (int)blob.CompressedSize : (int)blob.Size;
        if (offset + size > payload.Length)
            return ReadOnlySpan<byte>.Empty;
        return payload.Slice(offset, size);
    }

    /// <summary>
    /// 從 Payload 中讀取指定類型的第一個 Blob（零拷貝）。
    /// </summary>
    /// <param name="payload">Payload 資料。</param>
    /// <param name="blobs">Blob 描述符列表。</param>
    /// <param name="blobType">要查找的 Blob 類型。</param>
    /// <returns>匹配的 Blob 資料；未找到返回空。</returns>
    public static ReadOnlySpan<byte> GetBlobByType(ReadOnlySpan<byte> payload,
        ReadOnlySpan<BlobDescriptor> blobs, uint blobType)
    {
        for (int i = 0; i < blobs.Length; i++)
        {
            if (blobs[i].BlobType == blobType)
                return GetBlobData(payload, in blobs[i]);
        }
        return ReadOnlySpan<byte>.Empty;
    }
}

/// <summary>
/// .nnasset 格式寫入工具。
/// </summary>
public static class AssetFormatWriter
{
    /// <summary>
    /// 構建完整的 .nnasset 檔案。
    /// </summary>
    /// <param name="guid">資產 GUID。</param>
    /// <param name="typeId">型別 ID。</param>
    /// <param name="dependencies">依賴 GUID 列表。</param>
    /// <param name="blobDescriptors">Blob 描述符列表。</param>
    /// <param name="payload">載荷資料。</param>
    /// <param name="flags">標誌位。</param>
    /// <returns>完整的 .nnasset 檔案位元組陣列。</returns>
    public static byte[] WriteFile(GUID guid, ulong typeId,
        ReadOnlySpan<GUID> dependencies,
        ReadOnlySpan<BlobDescriptor> blobDescriptors,
        ReadOnlySpan<byte> payload,
        uint flags = 0)
    {
        // 計算偏移
        var depOffset = (ulong)AssetFormat.HeaderSize;
        var depSize = (ulong)dependencies.Length * 16; // sizeof(GUID) = 16
        var blobTableOffset = depOffset + depSize;
        var blobTableSize = (ulong)blobDescriptors.Length * 32; // sizeof(BlobDescriptor) = 32
        var payloadOffset = AssetFormat.Align(blobTableOffset + blobTableSize);

        // 構建 Header
        var header = new AssetHeader
        {
            Magic = AssetFormat.Magic,
            Version = AssetFormat.Version,
            AssetGuid = guid,
            TypeId = typeId,
            DependencyCount = (uint)dependencies.Length,
            BlobCount = (uint)blobDescriptors.Length,
            DependencyOffset = depOffset,
            BlobTableOffset = blobTableOffset,
            PayloadOffset = payloadOffset,
            PayloadSize = (ulong)payload.Length,
            Flags = flags,
        };

        // 計算總大小
        var totalSize = (int)(payloadOffset + (ulong)payload.Length);
        var buffer = new byte[totalSize];

        // 寫入 Header
        MemoryMarshal.Write(buffer, in header);

        // 寫入依賴
        if (dependencies.Length > 0)
        {
            var depBytes = MemoryMarshal.AsBytes(dependencies);
            depBytes.CopyTo(buffer.AsSpan((int)depOffset, (int)depSize));
        }

        // 寫入 Blob 表
        if (blobDescriptors.Length > 0)
        {
            var blobBytes = MemoryMarshal.AsBytes(blobDescriptors);
            blobBytes.CopyTo(buffer.AsSpan((int)blobTableOffset, (int)blobTableSize));
        }

        // Payload（自動填充 0 對齊）
        if (payload.Length > 0)
        {
            payload.CopyTo(buffer.AsSpan((int)payloadOffset, payload.Length));
        }

        return buffer;
    }
}
