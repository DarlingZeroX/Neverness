namespace Neverness.Editor.Assets.AssetOpening;

/// <summary>
/// 资产打开器标记——标记在 <see cref="IAssetOpener"/> 实现类上，
/// 声明其处理的资产类型 ID。由 <see cref="AssetOpenerRegistry.Discover"/> 反射自动发现。
/// </summary>
[AttributeUsage(AttributeTargets.Class, Inherited = false)]
public sealed class AssetOpenerAttribute : Attribute
{
    /// <summary>资产类型 ID（与 <see cref="ImportResult.AssetTypeId"/> 常量对齐）。</summary>
    public ulong TypeId { get; }

    public AssetOpenerAttribute(ulong typeId)
    {
        TypeId = typeId;
    }
}
