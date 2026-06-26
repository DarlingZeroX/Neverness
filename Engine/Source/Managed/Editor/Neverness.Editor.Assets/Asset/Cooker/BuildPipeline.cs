using Neverness.Runtime.Assets;
using Neverness.Runtime.VFS;

namespace Neverness.Editor.Assets;

/// <summary>
/// 构建管线。
///
/// 编排完整的资产构建流程：
///   1. 导入所有资产（ImportPipeline）
///   2. 应用分组配置
///   3. 调用 AssetCooker 或 PackageBuilder 生成 .nnpack
///   4. 输出构建报告
///
/// 用法：
///   var profile = BuildProfile.CreateRelease(projectRoot);
///   var result = BuildPipeline.Build(profile);
/// </summary>
public static class BuildPipeline
{
    /// <summary>构建进度事件。</summary>
    public static event Action<string, float>? OnProgress;

    /// <summary>构建完成事件。</summary>
    public static event Action<BuildReport>? OnComplete;

    /* ========== 全量构建 ========== */

    /// <summary>执行完整构建。</summary>
    public static BuildReport Build(BuildProfile profile)
    {
        var report = new BuildReport { ProfileName = profile.Name };
        var startTime = DateTime.UtcNow;

        try
        {
            /* Step 1: 确保所有资产已导入 */
            if (!ImportPipeline.IsInitialized)
                ImportPipeline.Initialize(profile.LibraryRoot);

            var importSummary = ImportPipeline.ImportDirectory(profile.AssetsRoot);
            report.ImportedCount = importSummary.ImportedFiles;
            report.ImportFailedCount = importSummary.FailedFiles;

            if (importSummary.HasErrors)
                report.Warnings.AddRange(importSummary.Errors);

            /* Step 2: 构建 .nnpack */
            if (profile.GroupManager != null && profile.GroupManager.Count > 0)
            {
                /* 按分组构建 */
                foreach (var group in profile.GroupManager.Groups)
                {
                    if (!group.IncludeInBuild)
                        continue;

                    var packPath = profile.OutputRoot.Combine(group.Name + ".nnpack");
                    if (PackageBuilder.BuildFromGroup(group, profile.LibraryRoot.FullPath, packPath.FullPath))
                    {
                        report.GeneratedPacks.Add(packPath.FullPath);
                    }
                    else
                    {
                        report.Errors.Add($"分组 {group.Name} 构建失败");
                    }
                }
            }
            else
            {
                /* 单包构建：将所有已导入资产打包 */
                var importedDir = profile.LibraryRoot.Combine("Imported");
                var packPath = profile.OutputRoot.Combine("default.nnpack");

                if (PackageBuilder.BuildFromDirectory(importedDir.FullPath, packPath.FullPath))
                {
                    report.GeneratedPacks.Add(packPath.FullPath);
                }
                else
                {
                    report.Errors.Add("默认包构建失败");
                }
            }

            report.Success = report.Errors.Count == 0;
        }
        catch (Exception ex)
        {
            report.Success = false;
            report.Errors.Add($"构建异常: {ex.Message}");
        }

        report.ElapsedSeconds = (DateTime.UtcNow - startTime).TotalSeconds;
        return report;
    }

    /* ========== 增量构建 ========== */

    /// <summary>增量构建（仅重新构建变化的资产）。</summary>
    public static BuildReport BuildIncremental(BuildProfile profile, IEnumerable<string> changedFiles)
    {
        var report = new BuildReport { ProfileName = profile.Name };
        var startTime = DateTime.UtcNow;

        try
        {
            /* 仅导入变化的文件 */
            if (!ImportPipeline.IsInitialized)
                ImportPipeline.Initialize(profile.LibraryRoot);

            var importSummary = ImportPipeline.ImportChanged(profile.AssetsRoot, changedFiles.Select(f => new NPath(f)));
            report.ImportedCount = importSummary.ImportedFiles;
            report.ImportFailedCount = importSummary.FailedFiles;
            report.SkippedCount = importSummary.SkippedFiles;

            /* 如果有资产被重新导入，需要重新构建受影响的包 */
            if (importSummary.ImportedFiles > 0)
            {
                /* TODO: 根据 DependencyGraph 确定受影响的包，仅重建那些 */
                /* 当前：全量重建 */
                var importedDir = profile.LibraryRoot.Combine("Imported");
                var packPath = profile.OutputRoot.Combine("default.nnpack");
                PackageBuilder.BuildFromDirectory(importedDir.FullPath, packPath.FullPath);
            }

            report.Success = true;
        }
        catch (Exception ex)
        {
            report.Success = false;
            report.Errors.Add($"增量构建异常: {ex.Message}");
        }

        report.ElapsedSeconds = (DateTime.UtcNow - startTime).TotalSeconds;
        return report;
    }

    /// <summary>清理构建输出。</summary>
    public static void Clean(NPath outputRoot)
    {
        if (Directory.Exists(outputRoot.FullPath))
        {
            try
            {
                Directory.Delete(outputRoot.FullPath, recursive: true);
                Console.WriteLine($"[BuildPipeline] 已清理: {outputRoot}");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[BuildPipeline] 清理失败: {ex.Message}");
            }
        }
    }
}

/// <summary>构建报告。</summary>
public sealed class BuildReport
{
    public string ProfileName { get; set; } = string.Empty;
    public bool Success { get; set; }
    public int ImportedCount { get; set; }
    public int ImportFailedCount { get; set; }
    public int SkippedCount { get; set; }
    public List<string> GeneratedPacks { get; } = new();
    public List<string> Errors { get; } = new();
    public List<string> Warnings { get; } = new();
    public double ElapsedSeconds { get; set; }

    public override string ToString()
    {
        var status = Success ? "成功" : "失败";
        return $"构建{status}: {ImportedCount} 导入, {GeneratedPacks.Count} 包, {Errors.Count} 错误, {ElapsedSeconds:F1}s";
    }
}
