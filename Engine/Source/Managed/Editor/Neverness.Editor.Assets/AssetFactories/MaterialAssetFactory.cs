using Neverness.Editor.Framework.Public;

namespace Neverness.Editor.Assets.AssetFactories;

/// <summary>
/// 材质资产工厂——在指定目录创建空材质文件（暂为 JSON 占位格式）。
/// </summary>
public sealed class MaterialAssetFactory : IAssetFactory
{
    public string DisplayName => "Material";
    public string Category => "Rendering";
    public string Icon => FontAwesome5Pro.Palette;
    public string FileExtension => ".material";

    public bool CreateAsset(string directoryPath)
    {
        try
        {
            var filePath = System.IO.Path.Combine(directoryPath, "New Material.material");
            int index = 1;
            while (File.Exists(filePath))
            {
                filePath = System.IO.Path.Combine(directoryPath, $"New Material {index++}.material");
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

            File.WriteAllText(filePath, json);
            return true;
        }
        catch (Exception ex)
        {
            Console.WriteLine(ex);
            return false;
        }
    }
}
