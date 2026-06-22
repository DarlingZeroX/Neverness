using System.Diagnostics;
using Neverness.Editor.Core.Public;
using Neverness.Editor.ProjectSystem.Public;
using Neverness.Runtime.VFS.Public;

namespace Neverness.Editor.Assets.AssetOpening;

/// <summary>
/// C# 脚本资产打开器——打开外部 IDE（Visual Studio / VS Code）加载整个解决方案。
///
/// 流程：
///   1. 获取 Game.sln 物理路径
///   2. 根据用户偏好选择 IDE
///   3. 启动 IDE 进程打开解决方案
/// </summary>
[AssetOpener(AssetTypeId.CSharpScript)]
public sealed class CSharpScriptAssetOpener : IAssetOpener
{
    private readonly IPreferencesService _preferencesService;

    public CSharpScriptAssetOpener(IPreferencesService preferencesService)
    {
        _preferencesService = preferencesService;
    }

    public bool CanOpen(ulong assetTypeId) => assetTypeId == AssetTypeId.CSharpScript;

    public Task OpenAsync(AssetOpenContext context)
    {
        var slnPath = GetSolutionPath();
        if (string.IsNullOrEmpty(slnPath))
        {
            Console.WriteLine("[CSharpScriptAssetOpener] 无法获取 Game.sln 路径");
            return Task.CompletedTask;
        }

        if (!File.Exists(slnPath))
        {
            Console.WriteLine($"[CSharpScriptAssetOpener] Game.sln 不存在: {slnPath}");
            return Task.CompletedTask;
        }

        var ide = _preferencesService.PreferredIDE;
        Console.WriteLine($"[CSharpScriptAssetOpener] 打开 IDE ({ide}): {slnPath}");

        try
        {
            switch (ide)
            {
                case IDEPreference.VisualStudio:
                    OpenVisualStudio(slnPath);
                    break;
                case IDEPreference.VSCode:
                    OpenVSCode(slnPath);
                    break;
                default:
                    Console.WriteLine($"[CSharpScriptAssetOpener] 未知 IDE 偏好: {ide}");
                    break;
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[CSharpScriptAssetOpener] 启动 IDE 失败: {ex.Message}");
        }

        return Task.CompletedTask;
    }

    /// <summary>获取 Game.sln 的物理路径。</summary>
    private static string? GetSolutionPath()
    {
        var projectRoot = VFS.GetAbsolutePath(ProjectPaths.Project.FullPath);
        if (string.IsNullOrEmpty(projectRoot))
            return null;

        return Path.Combine(projectRoot, "Game.sln");
    }

    /// <summary>启动 Visual Studio 打开解决方案。</summary>
    private static void OpenVisualStudio(string slnPath)
    {
        // 尝试使用 devenv 命令（需要 VS 在 PATH 中）
        // 或者使用文件关联（.sln 默认用 VS 打开）
        var startInfo = new ProcessStartInfo
        {
            FileName = slnPath,
            UseShellExecute = true
        };

        Process.Start(startInfo);
    }

    /// <summary>启动 VS Code 打开解决方案。</summary>
    private static void OpenVSCode(string slnPath)
    {
        // VS Code 需要 code 命令在 PATH 中
        // 使用 --folder-uri 打开项目文件夹
        var projectRoot = Path.GetDirectoryName(slnPath);
        if (string.IsNullOrEmpty(projectRoot))
        {
            Console.WriteLine("[CSharpScriptAssetOpener] 无法获取项目目录");
            return;
        }

        var startInfo = new ProcessStartInfo
        {
            FileName = "code",
            Arguments = $"\"{projectRoot}\"",
            UseShellExecute = false,
            CreateNoWindow = true
        };

        Process.Start(startInfo);
    }
}
