namespace Neverness.Editor.Assets.AssetOpening;

/// <summary>
/// 预制体资产打开器（桩实现）——未来打开 PrefabEditor（非实例化到 Runtime 场景）。
/// </summary>
[AssetOpener(AssetTypeId.Prefab)]
public sealed class PrefabAssetOpener : IAssetOpener
{
    public bool CanOpen(ulong assetTypeId) => assetTypeId == AssetTypeId.Prefab;

    public Task OpenAsync(AssetOpenContext context)
    {
        // TODO: 打开 PrefabEditor
        Console.WriteLine($"[PrefabAssetOpener] TODO: 打开预制体编辑器 → {context.VirtualPath}");
        return Task.CompletedTask;
    }
}
