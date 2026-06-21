using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets;

/// <summary>
/// 构建资产条目。
/// </summary>
public sealed class CookAssetEntry
{
    /// <summary>资产 GUID。</summary>
    public GUID Guid { get; set; }

    /// <summary>资产类型 ID（Texture2D=1, Mesh=2 等）。</summary>
    public ulong TypeId { get; set; }

    /// <summary>.nnasset 源文件路径。</summary>
    public string SourcePath { get; set; } = string.Empty;

    /// <summary>所属构建分组索引。</summary>
    public uint GroupIndex { get; set; }

    /// <summary>是否包含在构建中。</summary>
    public bool IncludeInBuild { get; set; } = true;
}

/// <summary>
/// 构建分组。
/// </summary>
public sealed class CookGroup
{
    /// <summary>分组名称（将成为 .nnpack 文件名）。</summary>
    public string Name { get; set; } = string.Empty;

    /// <summary>可寻址地址。</summary>
    public string Address { get; set; } = string.Empty;

    /// <summary>打包策略：0=PackTogether, 1=PackSeparately。</summary>
    public uint Strategy { get; set; }

    /// <summary>显式输出路径（覆盖默认路径）。</summary>
    public string OutputPath { get; set; } = string.Empty;
}

/// <summary>
/// 构建清单（纯 C# 实现，替代 C++ NNCookManifest）。
/// </summary>
public sealed class CookManifest
{
    private readonly List<CookAssetEntry> _assets = new();
    private readonly List<CookGroup> _groups = new();

    /// <summary>输出根目录。</summary>
    public string OutputRoot { get; set; } = string.Empty;

    /// <summary>Library 根目录（Library/Imported/）。</summary>
    public string LibraryRoot { get; set; } = string.Empty;

    public void AddAsset(CookAssetEntry entry) => _assets.Add(entry);
    public void AddGroup(CookGroup group) => _groups.Add(group);

    public IReadOnlyList<CookAssetEntry> Assets => _assets;
    public IReadOnlyList<CookGroup> Groups => _groups;
    public int AssetCount => _assets.Count;
    public int GroupCount => _groups.Count;

    public void Clear()
    {
        _assets.Clear();
        _groups.Clear();
        OutputRoot = string.Empty;
        LibraryRoot = string.Empty;
    }
}
