using System.IO.MemoryMappedFiles;
using System.Runtime.InteropServices;
using Neverness.Runtime.Assets.Formats;

namespace Neverness.Runtime.Assets.Pack;

/// <summary>
/// 單個掛載的 .nnpack 包。
///
/// 使用 MemoryMappedFile 零拷貝讀取包內資產資料。
/// 掛載時只讀取 Header + AssetTable（很小），數據區域按需通過 Span 訪問。
/// </summary>
public sealed class PackMount : IDisposable
{
    /// <summary>包檔案路徑。</summary>
    public string Path { get; }

    /// <summary>包頭。</summary>
    public PackHeader Header { get; }

    /// <summary>資產表。</summary>
    public PackAssetEntry[] AssetTable { get; }

    /// <summary>Manifest 資料（可選）。</summary>
    public byte[]? ManifestData { get; }

    /// <summary>GUID.Low → 資產表索引。</summary>
    public Dictionary<ulong, int> GuidToIndex { get; }

    private readonly MemoryMappedFile _mmf;
    private readonly MemoryMappedViewAccessor _accessor;
    private readonly long _fileSize;
    private bool _disposed;

    /// <summary>
    /// 從檔案路徑建立 PackMount。
    /// </summary>
    /// <param name="packPath">.nnpack 檔案路徑。</param>
    /// <exception cref="InvalidDataException">包格式無效。</exception>
    public PackMount(string packPath)
    {
        Path = packPath;

        var fileInfo = new FileInfo(packPath);
        _fileSize = fileInfo.Length;
        if (_fileSize < (long)PackFormat.HeaderSize)
            throw new InvalidDataException($"包檔案太小: {packPath}");

        // 建立 MemoryMappedFile（唯讀）
        _mmf = MemoryMappedFile.CreateFromFile(packPath, FileMode.Open, null, 0,
            MemoryMappedFileAccess.Read);
        _accessor = _mmf.CreateViewAccessor(0, 0, MemoryMappedFileAccess.Read);

        // 讀取 Header
        _accessor.Read(0, out PackHeader hdr);
        Header = hdr;
        if (!PackFormat.IsHeaderValid(in hdr))
            throw new InvalidDataException($"包頭無效: {packPath}");

        // 讀取 AssetTable
        var tableOffset = (long)Header.TableOffset;
        var tableSize = (long)Header.TableSize;
        if (tableOffset + tableSize > _fileSize)
            throw new InvalidDataException($"AssetTable 超出檔案範圍: {packPath}");

        var entryCount = (int)Header.AssetCount;
        AssetTable = new PackAssetEntry[entryCount];
        for (int i = 0; i < entryCount; i++)
        {
            _accessor.Read(tableOffset + i * 48, out AssetTable[i]); // sizeof(PackAssetEntry) = 48
        }

        // 建立 GUID → 索引
        GuidToIndex = new Dictionary<ulong, int>(entryCount);
        for (int i = 0; i < entryCount; i++)
        {
            GuidToIndex[AssetTable[i].GuidLow] = i;
        }

        // 讀取 Manifest（可選）
        if (Header.ManifestOffset > 0 &&
            Header.ManifestOffset + Header.ManifestSize <= (ulong)_fileSize)
        {
            ManifestData = new byte[Header.ManifestSize];
            _accessor.ReadArray((long)Header.ManifestOffset, ManifestData, 0,
                (int)Header.ManifestSize);
        }
    }

    /// <summary>
    /// 從包內讀取資產資料（零拷貝，返回 Span 切片）。
    ///
    /// 注意：返回的 Span 指向 MemoryMappedViewAccessor 的記憶體，
    /// 必須在 PackMount 被 Dispose 前使用。若需跨異步邊界傳遞，應複製為 byte[]。
    /// </summary>
    /// <param name="entryIndex">資產表索引。</param>
    /// <returns>資產資料的唯讀 Span；若範圍無效則返回空。</returns>
    public unsafe ReadOnlySpan<byte> ReadAssetData(int entryIndex)
    {
        if (entryIndex < 0 || entryIndex >= AssetTable.Length)
            return ReadOnlySpan<byte>.Empty;

        ref var entry = ref AssetTable[entryIndex];
        var dataStart = (long)(Header.DataOffset + entry.Offset);
        var readSize = (long)(entry.CompressedSize > 0 ? entry.CompressedSize : entry.Size);

        if (dataStart + readSize > _fileSize || readSize <= 0)
            return ReadOnlySpan<byte>.Empty;

        // 使用 unsafe 指標直接訪問 mmf 記憶體
        byte* ptr = null;
        _accessor.SafeMemoryMappedViewHandle.AcquirePointer(ref ptr);
        try
        {
            return new ReadOnlySpan<byte>(ptr + dataStart, (int)readSize);
        }
        finally
        {
            _accessor.SafeMemoryMappedViewHandle.ReleasePointer();
        }
    }

    /// <summary>
    /// 從包內讀取資產資料（安全版本，複製為 byte[]）。
    /// 適用於需要跨異步邊界傳遞資料的場景。
    /// </summary>
    public byte[]? ReadAssetDataCopy(int entryIndex)
    {
        var span = ReadAssetData(entryIndex);
        if (span.IsEmpty) return null;
        return span.ToArray();
    }

    /// <summary>
    /// 取得資產在包內的有效資料大小。
    /// </summary>
    public long GetAssetDataSize(int entryIndex)
    {
        if (entryIndex < 0 || entryIndex >= AssetTable.Length)
            return 0;
        ref var entry = ref AssetTable[entryIndex];
        return (long)(entry.CompressedSize > 0 ? entry.CompressedSize : entry.Size);
    }

    /// <summary>
    /// 是否包含指定 GUID 的資產。
    /// </summary>
    public bool ContainsAsset(GUID guid) => GuidToIndex.ContainsKey(guid.Low);

    /// <summary>
    /// 依 GUID 取得資產表索引（未找到返回 -1）。
    /// </summary>
    public int GetAssetIndex(GUID guid) =>
        GuidToIndex.TryGetValue(guid.Low, out var idx) ? idx : -1;

    public void Dispose()
    {
        if (_disposed) return;
        _disposed = true;
        _accessor.Dispose();
        _mmf.Dispose();
    }
}
