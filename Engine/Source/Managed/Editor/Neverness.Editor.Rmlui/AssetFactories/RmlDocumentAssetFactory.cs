using Neverness.Editor.Assets.AssetFactories;
using Neverness.Editor.Framework.Public;
using Neverness.Runtime.Assets;

namespace Neverness.Editor.Rmlui.AssetFactories;

/// <summary>
/// RmlUI 文档资产工厂——在指定目录创建 RmlUI HTML 文档文件。
/// 使用 EditorResourceCache 中的 RmlDocument 模板。
/// </summary>
public sealed class RmlDocumentAssetFactory : IAssetFactory
{
    public string DisplayName => "RmlUI Document";
    public string Category => "UI";
    public string Icon => FontAwesome5Pro.FileCode;
    public string FileExtension => ".html";

    public NPath? CreateAsset(NPath directoryPath)
    {
        try
        {
            // 递增命名：New Document.html, New Document 1.html, New Document 2.html ...
            var filePath = directoryPath.Combine("New Document.html");
            int index = 1;
            while (File.Exists(filePath.FullPath))
            {
                filePath = directoryPath.Combine($"New Document {index++}.html");
            }

            // 从 EditorResourceCache 获取 RmlUI 文档模板
            var content = EditorResourceCache.Instance.GetTemplate(
                EditorResourceCache.TemplateNames.RmlDocument);

            // 模板加载失败时使用硬编码兜底内容
            if (string.IsNullOrEmpty(content))
            {
                content = """
                    <rml>
                    <head>
                        <link type="text/css" href="style.css"/>
                    </head>
                    <body>
                        <div class="main_container">
                            <h1 class="title">New Document</h1>
                        </div>
                    </body>
                    </rml>
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
