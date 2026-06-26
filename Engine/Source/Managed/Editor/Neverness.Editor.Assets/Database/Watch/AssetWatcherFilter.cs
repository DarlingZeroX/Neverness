namespace Neverness.Editor.Assets;

/// <summary>
/// 资产监视器文件过滤规则。
///
/// 判断哪些文件/目录应该被 AssetWatcher 忽略：
///   - .meta 侧车文件
///   - 临时文件（dotfiles、~ 前缀）
///   - Editor 内部管理的资产（.scene、.prefab）
///   - 系统目录（Library、Temp、dotfile 目录）
///
/// @threadsafe 所有方法为纯函数，无状态。
/// </summary>
internal static class AssetWatcherFilter
{
    /// <summary>
    /// Editor 内部管理的资产扩展名——不需要 FileSystemWatcher 监视。
    /// 这些资产的修改由 Editor 自身驱动，走 Editor 内部保存流程，
    /// 不需要通过 Watcher → ImportPipeline 重新导入。
    ///
    /// 注意：.material 不在此列，因为美术可能用外部文本编辑器批量修改材质参数。
    /// </summary>
    private static readonly HashSet<string> s_editorManagedExtensions = new(StringComparer.OrdinalIgnoreCase)
    {
        ".scene",
        ".prefab",
    };

    /// <summary>判断文件路径是否应被忽略。</summary>
    internal static bool ShouldIgnoreFile(string path)
    {
        /* 忽略 .meta 文件 */
        if (path.EndsWith(".meta", StringComparison.OrdinalIgnoreCase))
            return true;

        /* 忽略临时文件 */
        var fileName = Path.GetFileName(path);
        if (fileName.StartsWith('.') || fileName.StartsWith('~'))
            return true;

        /* 忽略 Editor 内部管理的资产 */
        var ext = Path.GetExtension(path);
        if (!string.IsNullOrEmpty(ext) && s_editorManagedExtensions.Contains(ext))
            return true;

        return false;
    }

    /// <summary>判断目录路径是否应被忽略。</summary>
    internal static bool ShouldIgnoreDirectory(string dirPath)
    {
        var dirName = Path.GetFileName(dirPath);
        return dirName.Equals("Library", StringComparison.OrdinalIgnoreCase)
            || dirName.Equals("Temp", StringComparison.OrdinalIgnoreCase)
            || dirName.StartsWith('.');
    }
}
