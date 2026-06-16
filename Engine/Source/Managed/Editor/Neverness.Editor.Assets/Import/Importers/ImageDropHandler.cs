using Neverness.Editor.Assets.Private.Core;
using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets;

/// <summary>
/// 图片拖放处理器——从外部拖入图片文件到编辑器。
/// 复制原文件到 Assets/ 并生成 .meta 侧车文件，不做格式转换。
/// </summary>
[DropFileHandler(".png", ".jpg", ".jpeg", ".tga", ".bmp", ".dds", ".hdr")]
public class ImageDropHandler : IDropFileHandler
{
    private static readonly HashSet<string> SupportedExtensions = new(
        new[] { ".png", ".jpg", ".jpeg", ".tga", ".bmp", ".dds", ".hdr" },
        StringComparer.OrdinalIgnoreCase);

    public bool CanHandle(string filePath)
    {
        var ext = Path.GetExtension(filePath);
        return ext != null && SupportedExtensions.Contains(ext);
    }

    public bool Handle(string filePath, string assetsRoot)
    {
        var sourcePath = new NPath(filePath);
        var fileName = sourcePath.FileName;
        var targetPath = new NPath(Path.Combine(assetsRoot, fileName));

        // 1. 复制文件到 Assets/
        File.Copy(sourcePath.FullPath, targetPath.FullPath, overwrite: true);

        // 2. 生成 .meta 侧车（GUID + importer 名称）
        var importerName = MetaFileManager.InferImporterName(sourcePath.Extension);
        var meta = MetaFileManager.GetOrCreateMeta(targetPath, importerName);

        // 3. 预填充 ImportStateCache，防止 AssetWatcher 重复触发
        var contentHash = ImportPipeline.ComputeContentHash(targetPath);
        ImportPipeline.StateCache.MarkImported(targetPath, contentHash);

        // 4. 注册到 EditorAssetDatabase
        var virtualPath = new NVirtualPath($"/assets/{fileName}");
        EditorAssetDatabase.Register(virtualPath, meta.Guid, AssetTypeId.Texture2D);
        EditorAssetDatabase.SetSourcePath(virtualPath, targetPath);

        // 5. 标记脏，触发 ContentBrowser 刷新
        EditorAssetDatabase.MarkDirty(meta.Guid);

        ContentBrowser.Instance?.RefreshDirectory();
        ContentBrowser.Instance?.NotifyContentChanged();
        return true;
    }
}
