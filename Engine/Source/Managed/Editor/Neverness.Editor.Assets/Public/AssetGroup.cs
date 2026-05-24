using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets;

/// <summary>
/// 资产分组。
///
/// 一组资产的逻辑集合，用于 Addressable 打包。
/// </summary>
public sealed class AssetGroup
{
    /// <summary>分组名称。</summary>
    public string Name { get; set; } = string.Empty;

    /// <summary>Addressable 地址（如 "character/hero"）。</summary>
    public string Address { get; set; } = string.Empty;

    /// <summary>分组内的资产 GUID 列表。</summary>
    public List<GUID> Assets { get; } = new();

    /// <summary>打包策略。</summary>
    public BuildStrategy Strategy { get; set; } = BuildStrategy.PackTogether;

    /// <summary>标签列表（组级标签，应用于组内所有资产）。</summary>
    public List<string> Labels { get; } = new();

    /// <summary>是否在构建时包含（false = 排除此组）。</summary>
    public bool IncludeInBuild { get; set; } = true;

    /// <summary>资产数量。</summary>
    public int AssetCount => Assets.Count;

    /// <summary>添加资产到组。</summary>
    public void AddAsset(GUID guid)
    {
        if (!guid.IsZero && !Assets.Contains(guid))
            Assets.Add(guid);
    }

    /// <summary>从组中移除资产。</summary>
    public void RemoveAsset(GUID guid)
    {
        Assets.Remove(guid);
    }

    /// <summary>是否包含指定资产。</summary>
    public bool Contains(GUID guid) => Assets.Contains(guid);
}

/// <summary>打包策略。</summary>
public enum BuildStrategy
{
    /// <summary>整个组打包为一个 .nnpack。</summary>
    PackTogether,

    /// <summary>每个资产单独打包。</summary>
    PackSeparately,

    /// <summary>按子目录打包。</summary>
    PackByDirectory
}
