namespace Neverness.Editor.Assets.AssetOpening;

/// <summary>
/// 纹理资产打开器（桩实现）——未来打开 TextureInspectorPanel。
/// </summary>
[AssetOpener(AssetTypeId.Texture2D)]
public sealed class TextureAssetOpener : IAssetOpener
{
    public bool CanOpen(ulong assetTypeId) => assetTypeId == AssetTypeId.Texture2D;

    public Task OpenAsync(AssetOpenContext context)
    {
        // TODO: 打开 TextureInspectorPanel
        Console.WriteLine($"[TextureAssetOpener] TODO: 打开纹理查看器 → {context.VirtualPath}");
        return Task.CompletedTask;
    }
}
