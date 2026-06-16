using Neverness.Editor.Assets.Private.Core;
using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets;

/// <summary>
/// 兜底拖放处理器——处理所有未被其他处理器认领的文件。
/// 复制文件到 Assets/ 并生成 .meta，默认使用 DefaultImporter。
/// </summary>
[DropFileHandler]
public class DefaultDropHandler : IDropFileHandler
{
    public bool CanHandle(string filePath) => true;

    public bool Handle(string filePath, string assetsRoot)
    {
        var sourcePath = new NPath(filePath);
        var fileName = sourcePath.FileName;
        var targetPath = new NPath(Path.Combine(assetsRoot, fileName));

        File.Copy(sourcePath.FullPath, targetPath.FullPath, overwrite: true);

        var meta = MetaFileManager.GetOrCreateMeta(targetPath, "DefaultImporter");

        var contentHash = ImportPipeline.ComputeContentHash(targetPath);
        ImportPipeline.StateCache.MarkImported(targetPath, contentHash);

        var virtualPath = new NVirtualPath($"/assets/{fileName}");
        EditorAssetDatabase.Register(virtualPath, meta.Guid, 0);
        EditorAssetDatabase.SetSourcePath(virtualPath, targetPath);
        EditorAssetDatabase.MarkDirty(meta.Guid);

        ContentBrowser.Instance?.RefreshDirectory();
        return true;
    }
}
