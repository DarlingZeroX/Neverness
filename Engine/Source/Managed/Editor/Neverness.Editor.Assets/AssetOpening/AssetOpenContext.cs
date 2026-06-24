using Neverness.Runtime.Assets;
using Neverness.Runtime.VFS;

namespace Neverness.Editor.Assets.AssetOpening;

/// <summary>
/// 资产打开上下文——传递给 <see cref="IAssetOpener.OpenAsync"/> 的执行参数。
/// </summary>
public sealed class AssetOpenContext
{
    /// <summary>资产元数据（GUID、TypeId、Importer 等）。</summary>
    public required AssetMeta Meta { get; init; }

    /// <summary>资产 VFSService 虚拟路径。</summary>
    public required NVirtualPath VirtualPath { get; init; }

    /// <summary>资产 GUID。</summary>
    public required GUID Guid { get; init; }

    /// <summary>资产类型 ID（与 AssetTypeId 常量对齐）。</summary>
    public required ulong AssetTypeId { get; init; }
}
