using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets;

/// <summary>
/// 资产导入结果。
///
/// IAssetImporter.Import() 返回此对象，包含编译 .nnasset 所需的所有数据。
/// </summary>
public sealed class ImportResult
{
    /// <summary>资产 GUID。</summary>
    public GUID AssetGuid { get; set; }

    /// <summary>资产类型 ID（FNV-1a hash of type name）。</summary>
    public ulong TypeId { get; set; }

    /// <summary>导入的 blob 数据列表。</summary>
    public List<ImportedBlob> Blobs { get; } = new();

    /// <summary>依赖的资产 GUID 列表。</summary>
    public List<GUID> Dependencies { get; } = new();

    /// <summary>缩略图数据（PNG/BMP 格式，可选）。</summary>
    public byte[]? ThumbnailData { get; set; }

    /// <summary>类型特定的元数据（可选，用于运行时快速查询）。</summary>
    public byte[]? TypeInfo { get; set; }

    /// <summary>导入是否成功。</summary>
    public bool Success { get; set; } = true;

    /// <summary>错误信息（Success=false 时）。</summary>
    public string? ErrorMessage { get; set; }

    /// <summary>创建成功结果的快捷方法。</summary>
    public static ImportResult Ok(GUID guid, ulong typeId)
    {
        return new ImportResult { AssetGuid = guid, TypeId = typeId, Success = true };
    }

    /// <summary>创建失败结果的快捷方法。</summary>
    public static ImportResult Fail(string error)
    {
        return new ImportResult { Success = false, ErrorMessage = error };
    }
}

/// <summary>
/// 导入的单个 blob 数据。
///
/// 对应 .nnasset 中的一个 blob 描述符。
/// </summary>
public sealed class ImportedBlob
{
    private byte[] _data = Array.Empty<byte>();

    /// <summary>Blob 类型（参见 NNAssetFormat 的 blob type 常量）。</summary>
    public uint BlobType { get; set; }

    /// <summary>原始数据。</summary>
    public byte[] Data
    {
        get => _data;
        set => _data = value ?? Array.Empty<byte>();
    }

    /// <summary>压缩后的数据（可选，null 表示不压缩）。</summary>
    public byte[]? CompressedData { get; set; }

    /// <summary>Blob 标志位。</summary>
    public uint Flags { get; set; }

    /// <summary>实际写入 .nnasset 的数据（优先使用压缩数据）。</summary>
    public byte[] Payload => CompressedData ?? _data;
}

/// <summary>
/// 预定义资产类型 ID 常量（与 C++ 侧 NNAssetTypes.h 一致）。
/// </summary>
public static class AssetTypeId
{
    public const ulong Texture2D = 1;
    public const ulong Mesh = 2;
    public const ulong AudioClip = 3;
    public const ulong Material = 4;
    public const ulong Shader = 5;
    public const ulong Scene = 6;
    public const ulong Prefab = 7;
    public const ulong Animation = 8;
    public const ulong LuaScript = 9;
    public const ulong VideoClip = 10;

    /// <summary>
    /// 预定义 blob 类型常量。
    /// </summary>
    public static class BlobType
    {
        public const uint Data = 0;
        public const uint MipLevel = 1;
        public const uint VertexBuffer = 2;
        public const uint IndexBuffer = 3;
        public const uint Thumbnail = 4;
        public const uint AudioPcm = 5;
        public const uint AudioSeek = 6;
        public const uint EntityHierarchy = 7;
        public const uint ComponentData = 8;
        public const uint TypeInfo = 9;
        public const uint VideoFrame = 10;
        public const uint VideoSeek = 11;
        public const uint Subtitle = 12;
    }
}
