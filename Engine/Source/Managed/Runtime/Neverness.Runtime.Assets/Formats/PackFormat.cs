using System.Runtime.InteropServices;

namespace Neverness.Runtime.Assets.Formats;

/// <summary>
/// .nnpack 资产包二进制格式定义。
///
/// .nnpack 是 Neverness Engine 的资产分发格式，包含一组编译后的 .nnasset。
/// 用于运行时分发（本地包/热更新/Addressable）。
///
/// 布局：
///   [PackHeader: 64 bytes]
///   [AssetTable: assetCount × PackAssetEntry]
///   [Manifest: 变长，包含包名、标签索引、地址索引]
///   [Asset Data: 连续 .nnasset 数据]
/// </summary>
public static class PackFormat
{
    /* ======================== 常量 ======================== */

    /// <summary>.nnpack 檔案魔數：'NNPK'。</summary>
    public const uint Magic = 0x4E4E504Bu;

    /// <summary>當前格式版本。</summary>
    public const uint Version = 1u;

    /// <summary>Header 大小（位元組）。</summary>
    public const uint HeaderSize = 64;

    /// <summary>對齊粒度。</summary>
    public const uint Alignment = 64;

    /* ======================== 包標誌位 ======================== */

    /// <summary>包數據已壓縮。</summary>
    public const uint FlagCompressed = 0x0001;

    /// <summary>包數據已加密。</summary>
    public const uint FlagEncrypted = 0x0002;

    /// <summary>支持流式加載。</summary>
    public const uint FlagStreaming = 0x0004;

    /* ======================== 工具方法 ======================== */

    /// <summary>
    /// 對齊至 64 位元組邊界。
    /// </summary>
    public static ulong Align(ulong offset)
    {
        return (offset + Alignment - 1) & ~(ulong)(Alignment - 1);
    }

    /// <summary>
    /// 驗證包頭是否有效。
    /// </summary>
    public static bool IsHeaderValid(in PackHeader header)
    {
        return header.Magic == Magic
            && header.Version == Version
            && header.AssetCount > 0;
    }
}

/* ======================== 包頭 ======================== */

/// <summary>
/// .nnpack 文件頭（固定 64 字節）。
/// 與 C++ NNPackHeader 記憶體佈局一致（pack=8）。
/// </summary>
[StructLayout(LayoutKind.Sequential, Pack = 8)]
public struct PackHeader
{
    /// <summary>[0] 魔數 'NNPK'。</summary>
    public uint Magic;

    /// <summary>[4] 格式版本。</summary>
    public uint Version;

    /// <summary>[8] 資產數量。</summary>
    public uint AssetCount;

    /// <summary>[12] 標誌位。</summary>
    public uint Flags;

    /// <summary>[16] AssetTable 偏移。</summary>
    public ulong TableOffset;

    /// <summary>[24] AssetTable 大小。</summary>
    public ulong TableSize;

    /// <summary>[32] Manifest 偏移。</summary>
    public ulong ManifestOffset;

    /// <summary>[40] Manifest 大小。</summary>
    public ulong ManifestSize;

    /// <summary>[48] 資產數據起始偏移。</summary>
    public ulong DataOffset;

    /// <summary>[56] 資產數據總大小。</summary>
    public ulong TotalDataSize;
}

/* ======================== 資產表條目 ======================== */

/// <summary>
/// 包內單個資產的索引條目（48 位元組）。
/// 與 C++ NNPackAssetEntry 記憶體佈局一致（pack=8）。
/// </summary>
[StructLayout(LayoutKind.Sequential, Pack = 8)]
public struct PackAssetEntry
{
    /// <summary>資產 GUID (high)。</summary>
    public ulong GuidHigh;

    /// <summary>資產 GUID (low)。</summary>
    public ulong GuidLow;

    /// <summary>資產類型 ID。</summary>
    public ulong TypeId;

    /// <summary>在包數據區中的偏移。</summary>
    public ulong Offset;

    /// <summary>原始大小。</summary>
    public ulong Size;

    /// <summary>壓縮大小（0 = 未壓縮）。</summary>
    public ulong CompressedSize;

    /// <summary>條目標誌。</summary>
    public uint Flags;

    /// <summary>填充對齊。</summary>
    public uint Pad;

    /// <summary>
    /// 取得此條目對應的 GUID。
    /// </summary>
    public readonly GUID GetGuid() => new(GuidHigh, GuidLow);

    /// <summary>
    /// 取得有效數據大小（優先使用壓縮大小）。
    /// </summary>
    public readonly ulong EffectiveSize => CompressedSize > 0 ? CompressedSize : Size;
}
