using Neverness.Editor.Assets.AssetFactories;
using Neverness.Editor.Framework.Public;
using Neverness.Runtime.VFS;

namespace Neverness.Editor.Rmlui.AssetFactories;

/// <summary>
/// CSS 样式资产工厂——在指定目录创建 RmlUI CSS 样式文件。
/// 使用 EditorResourceCache 中的 RmlStyle 模板。
/// </summary>
public sealed class CssAssetFactory : IAssetFactory
{
    public string DisplayName => "CSS Style";
    public string Category => "UI";
    public string Icon => "🎨";
    public string FileExtension => ".css";

    public NPath? CreateAsset(NPath directoryPath)
    {
        try
        {
            // 递增命名：New Style.css, New Style 1.css, New Style 2.css ...
            var filePath = directoryPath.Combine("New Style.css");
            int index = 1;
            while (File.Exists(filePath.FullPath))
            {
                filePath = directoryPath.Combine($"New Style {index++}.css");
            }

            // 从 EditorResourceCache 获取 CSS 样式模板
            var content = EditorResourceCache.Instance.GetTemplate(
                EditorResourceCache.TemplateNames.RmlStyle);

            // 模板加载失败时使用硬编码兜底内容
            if (string.IsNullOrEmpty(content))
            {
                content = """
                    body {
                        font-family: "Microsoft YaHei";
                    }

                    .container {
                        display: flex;
                        flex-direction: column;
                        align-items: center;
                        justify-content: center;
                        height: 100vh;
                        width: 100vw;
                        padding: 20px;
                        box-sizing: border-box;
                    }
                    """;
            }

            File.WriteAllText(filePath.FullPath, content);
            return filePath;
        }
        catch (Exception ex)
        {
            Console.WriteLine(ex);
            return null;
        }
    }
}
