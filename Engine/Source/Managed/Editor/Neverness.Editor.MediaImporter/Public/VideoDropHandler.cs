using Neverness.Editor.Assets;
using Neverness.Editor.Assets.Private.Core;
using Neverness.Runtime.Assets;

using ImportPipeline = Neverness.Editor.Assets.ImportPipeline;

namespace Neverness.Editor.MediaImporter;

/// <summary>
/// 视频拖放处理器——从外部拖入视频文件到编辑器。
/// 复制原文件到 Assets/ 并生成 .meta 侧车文件，使用 VideoAssetImporter。
/// </summary>
[DropFileHandler(".mp4", ".webm", ".mov", ".mkv", ".avi")]
public class VideoDropHandler : IDropFileHandler
{
    private static readonly HashSet<string> SupportedExtensions = new(
        new[] { ".mp4", ".webm", ".mov", ".mkv", ".avi" },
        StringComparer.OrdinalIgnoreCase);

    /// <inheritdoc />
    public bool CanHandle(string filePath)
    {
        var ext = Path.GetExtension(filePath);
        return ext != null && SupportedExtensions.Contains(ext);
    }

    /// <inheritdoc />
    public bool Handle(string filePath, string assetsRoot)
    {
        var sourcePath = new NPath(filePath);
        var fileName = sourcePath.FileName;
        var targetPath = new NPath(Path.Combine(assetsRoot, fileName));

        // 1. 复制文件到 Assets/
        File.Copy(sourcePath.FullPath, targetPath.FullPath, overwrite: true);

        // 2. 生成 .meta 侧车（GUID + importer 名称）
        var meta = MetaFileManager.GetOrCreateMeta(targetPath, "VideoAssetImporter");

        // 3. 预填充 ImportStateCache（ComputeContentHash 为 internal，此处跳过）
        //    AssetWatcher 检测到新文件后会自动触发导入

        // 4. 注册到 EditorAssetDatabase
        var virtualPath = new NVirtualPath($"/assets/{fileName}");
        EditorAssetDatabase.Register(virtualPath, meta.Guid, AssetTypeId.VideoClip);
        EditorAssetDatabase.SetSourcePath(virtualPath, targetPath);

        // 5. 标记脏，触发 ContentBrowser 刷新
        EditorAssetDatabase.MarkDirty(meta.Guid);

        ContentBrowser.Instance?.RefreshDirectory();
        return true;
    }
}
