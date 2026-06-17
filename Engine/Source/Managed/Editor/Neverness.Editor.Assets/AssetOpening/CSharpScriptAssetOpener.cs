namespace Neverness.Editor.Assets.AssetOpening;

/// <summary>
/// C# 脚本资产打开器（桩实现）——未来调用 VSCode / Rider 集成。
/// </summary>
[AssetOpener(AssetTypeId.CSharpScript)]
public sealed class CSharpScriptAssetOpener : IAssetOpener
{
    public bool CanOpen(ulong assetTypeId) => assetTypeId == AssetTypeId.CSharpScript;

    public Task OpenAsync(AssetOpenContext context)
    {
        // TODO: 调用外部 IDE 集成
        Console.WriteLine($"[CSharpScriptAssetOpener] 打开脚本编辑器 → {context.VirtualPath}");
        return Task.CompletedTask;
    }
}
