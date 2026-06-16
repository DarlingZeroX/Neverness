using Neverness.Editor.Framework.Public;
using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets.AssetFactories;

/// <summary>
/// 材质资产工厂——在指定目录创建空材质文件（暂为 JSON 占位格式）。
/// </summary>
public sealed class MaterialAssetFactory : IAssetFactory
{
    public string DisplayName => "Material";
    public string Category => "Rendering";
    public string Icon => "🎨";
    public string FileExtension => ".material";

    public NPath? CreateAsset(NPath directoryPath)
    {
        try
        {
            var filePath = directoryPath.Combine("New Material.material");
            int index = 1;
            while (File.Exists(filePath.FullPath))
            {
                filePath = directoryPath.Combine($"New Material {index++}.material");
            }

            // TODO:
            // 未来接入材质序列化系统后替换为真实格式
            var json = """
                {
                    "formatVersion": 1,
                    "name": "New Material",
                    "shader": "Standard",
                    "properties": {}
                }
                """;

            File.WriteAllText(filePath.FullPath, json);
            return filePath;
        }
        catch (Exception ex)
        {
            Console.WriteLine(ex);
            return null;
        }
    }
}
