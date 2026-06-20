namespace Neverness.Editor.Core.Public;

/// <summary>
/// 拖放导入服务接口——处理从外部拖入的文件导入。
/// 通过 Core 服务定位器暴露，AvaloniaFrontend 层调用。
/// </summary>
public interface IDropImportService
{
    /// <summary>导入拖入的外部文件到指定目录。</summary>
    /// <param name="filePaths">文件路径列表。</param>
    /// <param name="targetDirectory">目标目录路径。</param>
    /// <returns>导入结果（成功数，失败数）。</summary>
    (int SuccessCount, int FailCount) ImportFiles(string[] filePaths, string targetDirectory);
}
