using Neverness.Runtime.Engine;
using Neverness.Runtime.Scene;

namespace Neverness.Editor.Assets.AssetOpening;

/// <summary>
/// 场景资产打开器——通过 Runtime SceneManager 加载场景并触发事件驱动链路。
/// </summary>
[AssetOpener(AssetTypeId.Scene)]
public sealed class SceneAssetOpener : IAssetOpener
{
    private readonly SceneManager _sceneManager;

    public SceneAssetOpener(SceneManager sceneManager)
    {
        _sceneManager = sceneManager;
    }

    public bool CanOpen(ulong assetTypeId) => assetTypeId == AssetTypeId.Scene;

    public Task OpenAsync(AssetOpenContext context)
    {
        var sceneName = context.VirtualPath.FileNameWithoutExtension;

        var result = _sceneManager.LoadSceneFromAsset(sceneName, context.VirtualPath.FullPath);
        if (result != NNSceneResult.Ok)
        {
            Console.WriteLine($"[SceneAssetOpener] 加载场景失败: {context.VirtualPath} ({result})");
        }

        return Task.CompletedTask;
    }
}
