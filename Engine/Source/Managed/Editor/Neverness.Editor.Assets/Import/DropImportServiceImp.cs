using Neverness.Editor.Core.Public;

namespace Neverness.Editor.Assets.Private;

/// <summary>
/// 拖放导入服务实现——处理从外部拖入的文件导入。
/// 实现 IDropImportService 接口，通过 Core 服务定位器暴露。
/// </summary>
public class DropImportServiceImp : IDropImportService
{
    /// <summary>导入拖入的外部文件到指定目录。</summary>
    public (int SuccessCount, int FailCount) ImportFiles(string[] filePaths, string targetDirectory)
    {
        var successCount = 0;
        var failCount = 0;

        foreach (var filePath in filePaths)
        {
            try
            {
                var fileName = Path.GetFileName(filePath);

                // 调用 ImageDropHandler
                var handler = new ImageDropHandler();
                if (handler.CanHandle(filePath))
                {
                    if (handler.Handle(filePath, targetDirectory))
                    {
                        successCount++;
                        Console.WriteLine($"[DropImportService] ✓ 导入成功: {fileName}");
                    }
                    else
                    {
                        failCount++;
                        Console.WriteLine($"[DropImportService] ✗ 导入失败: {fileName}");
                    }
                }
                else
                {
                    Console.WriteLine($"[DropImportService] 不支持的文件类型: {fileName}");
                }
            }
            catch (Exception ex)
            {
                failCount++;
                Console.WriteLine($"[DropImportService] 导入异常: {Path.GetFileName(filePath)} → {ex.Message}");
            }
        }

        return (successCount, failCount);
    }
}
