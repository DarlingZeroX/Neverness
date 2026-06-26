namespace Neverness.Editor.Assets.AssetOpening;

/// <summary>
/// 材质资产打开器（桩实现）——未来打开 MaterialEditorPanel。
/// </summary>
[AssetOpener(AssetTypeId.Material)]
public sealed class MaterialAssetOpener : IAssetOpener
{
    public bool CanOpen(ulong assetTypeId) => assetTypeId == AssetTypeId.Material;

    public Task OpenAsync(AssetOpenContext context)
    {
        // TODO: 打开 MaterialEditorPanel
        Console.WriteLine($"[MaterialAssetOpener] TODO: 打开材质编辑器 → {context.VirtualPath}");
        return Task.CompletedTask;
    }
}
