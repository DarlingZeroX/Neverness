using Neverness.Editor.Framework.Public;
using Neverness.Editor.ProjectSystem.Public;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Scene;
using Neverness.Runtime.Serialization;

namespace Neverness.Editor.Assets.AssetFactories;

/// <summary>
/// 场景资产工厂——在指定目录创建空场景文件（VGSC 二进制格式，经 VFS 序列化）。
/// 从 ContentBrowser.CreateNewSceneFile 迁移而来。
/// </summary>
public sealed class SceneAssetFactory : IAssetFactory
{
    public string DisplayName => "Scene";
    public string Category => "Scene";
    public string Icon => FontAwesome5Pro.File;
    public string FileExtension => ".scene";

    public bool CreateAsset(string directoryPath)
    {
        try
        {
            // 递增命名：New Scene.scene, New Scene 1.scene, New Scene 2.scene ...
            var filePath = System.IO.Path.Combine(directoryPath, "New Scene.scene");
            int index = 1;
            while (File.Exists(filePath))
            {
                filePath = System.IO.Path.Combine(directoryPath, $"New Scene {index++}.scene");
            }

            // 组装 VFS 虚拟路径（"/assets/相对路径"）
            var vfsDir = ProjectPaths.GetResourcePath(directoryPath);
            var vfsPath = vfsDir + System.IO.Path.GetFileName(filePath);

            // 创建空场景 → 序列化到 VFS 路径 → 销毁临时场景
            var result = SceneNativeBridge.CreateScene(out var sceneHandle);
            if (result != NNSceneResult.Ok)
            {
                Console.WriteLine($"SceneAssetFactory: CreateScene failed ({result})");
                return false;
            }

            result = NNSceneSerializeBridge.SerializeScene(sceneHandle, vfsPath);
            SceneNativeBridge.DestroyScene(sceneHandle);

            if (result != NNSceneResult.Ok)
            {
                Console.WriteLine($"SceneAssetFactory: SerializeScene failed ({result})");
                return false;
            }

            return true;
        }
        catch (Exception ex)
        {
            Console.WriteLine(ex);
            return false;
        }
    }
}
