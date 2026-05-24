namespace Neverness.Editor.Assets.AssetOpening;

/// <summary>
/// 资产打开策略接口——每种资产类型实现一个 Opener。
/// 通过 <see cref="AssetOpenerAttribute"/> 标记，由 <see cref="AssetOpenerRegistry"/> 反射自动注册。
/// </summary>
public interface IAssetOpener
{
    /// <summary>是否可以打开指定 TypeId 的资产。</summary>
    bool CanOpen(ulong assetTypeId);

    /// <summary>异步打开资产。</summary>
    Task OpenAsync(AssetOpenContext context);
}
