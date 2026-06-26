using Neverness.Editor.Settings;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Scene;

namespace Neverness.Editor.Assets.AssetOpening;

/// <summary>
/// 场景资产打开器——通过 Runtime SceneManager 加载场景并触发事件驱动链路。
/// 打开成功后自动将 VFS 路径写入 EditorSettings.Session.LastOpenedScene。
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
        var vfsPath = context.VirtualPath.FullPath;

        Console.WriteLine($"[SceneAssetOpener] 打开场景: name={sceneName}, vfsPath={vfsPath}");

        var result = _sceneManager.LoadSceneFromAsset(sceneName, vfsPath);
        if (!result)
        {
            Console.Error.WriteLine($"[SceneAssetOpener] 加载场景失败: {vfsPath}");
        }
        else
        {
            // 记录上次打开的场景，供下次启动自动加载
            EditorSettings.Session.LastOpenedScene = vfsPath;
            Console.WriteLine($"[SceneAssetOpener] 场景加载成功: {sceneName}, 激活场景数={_sceneManager.LoadedCount}");
        }

        return Task.CompletedTask;
    }
}
