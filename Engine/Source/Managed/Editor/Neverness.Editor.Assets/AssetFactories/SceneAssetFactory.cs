using Neverness.Editor.Framework.Public;
using Neverness.Editor.ProjectSystem.Public;
using Neverness.Runtime.Assets;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Scene;

namespace Neverness.Editor.Assets.AssetFactories;

/// <summary>
/// 场景资产工厂——在指定目录创建含默认 Camera 实体的场景文件（VGSC 二进制格式，经 VFS 序列化）。
/// 经由 SceneWorld 高层 API 操作，不直接引用 Native Bridge。
/// </summary>
public sealed class SceneAssetFactory : IAssetFactory
{
    public string DisplayName => "Scene";
    public string Category => "Scene";
    public string Icon => "🗺️";
    public string FileExtension => ".scene";

    public NPath? CreateAsset(NPath directoryPath)
    {
        try
        {
            // 递增命名：New Scene.scene, New Scene 1.scene, New Scene 2.scene ...
            var filePath = directoryPath.Combine("New Scene.scene");
            int index = 1;
            while (File.Exists(filePath.FullPath))
            {
                filePath = directoryPath.Combine($"New Scene {index++}.scene");
            }

            // 组装 VFS 虚拟路径（"/assets/相对路径"）
            var vfsDir = ProjectPaths.GetResourcePath(directoryPath);
            if (vfsDir == null)
            {
                Console.WriteLine("SceneAssetFactory: Failed to resolve VFS path");
                return null;
            }
            var vfsPath = vfsDir.Value.Combine(filePath.FileName);

            // 创建场景世界 → 添加默认 Camera 实体 → 序列化 → 释放
            using var world = SceneWorld.Create(filePath.FileNameWithoutExtension);
            if (world == null)
            {
                Console.WriteLine("SceneAssetFactory: SceneWorld.Create returned null");
                return null;
            }

            EntityFactory.CreateCamera(world);

            var result = world.Save(vfsPath.FullPath);
            if (result != NNSceneResult.Ok)
            {
                Console.WriteLine($"SceneAssetFactory: Save failed ({result})");
                return null;
            }

            return filePath;
        }
        catch (Exception ex)
        {
            Console.WriteLine(ex);
            return null;
        }
    }
}
