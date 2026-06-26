namespace Neverness.Editor.Assets.AssetOpening;

/// <summary>
/// 脚本资产打开器（桩实现）——未来调用 VSCode / Rider 集成。
/// 支持 LuaScript 和 CSharpScript。
/// </summary>
[AssetOpener(AssetTypeId.LuaScript)]
public sealed class ScriptAssetOpener : IAssetOpener
{
    public bool CanOpen(ulong assetTypeId) =>
        assetTypeId == AssetTypeId.LuaScript || assetTypeId == AssetTypeId.CSharpScript;

    public Task OpenAsync(AssetOpenContext context)
    {
        // TODO: 调用外部 IDE 集成
        Console.WriteLine($"[ScriptAssetOpener] 打开脚本编辑器 → {context.VirtualPath}");
        return Task.CompletedTask;
    }
}
